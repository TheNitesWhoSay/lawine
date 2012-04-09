/************************************************************************/
/* File Name   : isomap.c                                               */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : April 11st, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : Isometric map API implementation                       */
/************************************************************************/

#include "isomap.h"

/*
	名词解释：

	1. ISOM值(isom)			地图文件ISOM段的数值。
	2. ISOM菱形				8个相等的ISOM值围成的菱形。
							它可以分成4个正交的角落菱形。
	3. 角落菱形(corner)		每个ISOM菱形在左上右下四个角落上的小菱形。边缘形状处理时很重要。
	4. TILE菱形				从地图的最左侧开始，每两个TILE组成一个长方形。
							每个这样的一个长方形都对应到一个菱形数据结构LISOMTILE。
							这样的一个菱形就叫做TILE菱形。有时也把这种长方形简称为TILE菱形。
	5. 画刷(brush)			对应L_BRUSH_XXX枚举，代表着地形画刷的类型。
							每个画刷都与一种地形相对应。
							画刷的ID与其对应中央地形的索引是相等的。
	6. 画刷菱形				画刷占据的ISOM菱形。
							直观的说它就是staredit中显示画刷位置的那个菱形。
	7. 地形(terrian)		画刷直接对应的地形，如Dirt、Water等等。
	8. 中央地形(center)		其ISOM菱形的各角落都为单一地形。
							显然在同一个ERA里，它的数目和地形数目相同。
							由于画刷、地形、中央地形是一一对应的，有时候不区分它们。
	9. 边缘地形(edge)		两种不同地形的交界处。
							其ISOM菱形的角落地形存在两个值。
	10.边缘形状(shape)		边缘地形的4个角落菱形的组合方式。共计14种。
	11.水平邻接(h-abut)		每个TILE在XY平面上左上右下四方向上的邻接关系。
							其ID格式同CV5的left_abut、top_abut、right_abut和bottom_abut。
	12.垂直邻接(v-abut)		每个TILE在Z轴上的上下方邻接关系。
							其ID格式同CV5的up_abut、down_abut，直接对应V_ABUT_XXX。
	13.连接(link)			如果两种地形可以组合成边缘，则称它们是可连接的。
							连接带有方向性，连接的两端必然有一个在上层，另一个在下层。
	14.覆盖型地形(overlap)	与下层地形交接时不会产生悬崖的地形类型。比如Grass、Mud等。
	15.分层型地形(layer)	即非覆盖地形。如Water、Dirt、High Dirt等。
	16.带铺垫地形(matted)	带铺垫地形一定是分层型地形。
							该种地形与下方地形连接形成边缘时，要求其下方地形至少再向远处
							延伸开一个画刷菱形，从而形成一个底座。比如Structure、Temple等。
	17.扩散型地形(diffuse)	扩散型地形必然是覆盖型地形。
							当该种地形覆盖在下层地形上时，在LHHH和HHLH形状上，
							它在横向上比非扩散型地形多占用一部分面积（体现在水平邻接关系上），
							因此该种地形看起来比非扩散型地形更胖些。
	18.方砖型地形(tile)		方砖型地形必然是覆盖型地形。
							该类地形覆盖下层地形时通常像方砖一样铺在上面。
							如Asphalt、Plating等。
	19.台阶型地形(step)		Installation特有的一种地形，只有Substructure、Floor和Roof这三种地形。
							它们都是分层型地形。
							该种地形的悬崖都是垂直的平台，但并不是所有的垂直平台都是台阶型地形，
							比如Temple就不是。
	20.屋顶型地形(roof)		仅Roof为该类型地形。
							它在生成垂直邻接关系的算法上有点特殊，与所有其他地形都不同。
	21.悬崖(cliff)			两种分层型地形（即非覆盖型地形）的边缘处会形成悬崖。
							而分层型地形与覆盖型地形、覆盖型地形与覆盖型地形的边缘则不会出现。
	22.TILE类型(type)		一个TILE的地形类型。它可能是某种中央地形或者边缘类型。
							每一个TILE类型都有一个对应的唯一ID。
	23.TILE位置(position)	表明是TILE菱形的上下左右的哪个角的常数。
							其中奇菱形和偶菱形是有区别的。

	注：该源文件未考虑多线程同步问题。
*/

/************************************************************************/

/* 垂直邻接ID常数 */
#define V_ABUT_NONE			0			/* 无 */
#define V_ABUT_CENTER		1			/* 中央 */
#define V_ABUT_LEFT			2			/* 左边界 */
#define V_ABUT_RIGHT		3			/* 右边界 */

#define SHAPE_NUM			14			/* 边缘形状数 */
#define CLIFF_NUM			4			/* 每种悬崖地形对应的水平邻接ID数 */

#define MAX_CENTER			13			/* 最大中央地形种类数（Jungle/Desert/Ice/Twilight） */
#define MAX_EDGE			12			/* 最大边缘地形种类数（Jungle/Desert/Ice/Twilight） */
#define MAX_LINK			6			/* 单方向最大连接数（Platform:Platform） */

#define MAX_SHAPE_ID		16			/* 边缘形状索引最大值 + 1 */
#define MAX_CENTER_ISOM		0x0f		/* 中央地形的最大ISOM值（Platform:Dark Platform） + 1 */

#define CENTER_FLAG			0x00		/* 0x0#表示直接使用低层地形的TILE边界ID */
#define DIFFUSE_FLAG		0x10		/* 0x1#表示如果是扩散地形，使用高层地形的TILE边界ID，否则使用SIDE_MASK相应的值 */
#define CLIFF_FLAG			0x20		/* 0x2#表示如果是悬崖地形，使用对应悬崖的TILE边界ID，否则使用低层地形的TILE边界ID */
#define SIDE_FLAG			0x30		/* 0x3#表示直接使用该常数（它实际代表了边缘的8个方向） */
#define MAP_FLAG_MASK		0xf0

/************************************************************************/

#define for_each_from(from)				for ((from) = 0; (from) < FROM_NUM; (from)++)

#define REVERSE(order)					(!(order))
#define OPPOSITE(from)					(((from) + FROM_NUM / 2) % FROM_NUM)
#define SIDE_OF_FROM(from)				((((from) + 1) % FROM_NUM) / (FROM_NUM / 2))
#define DIR1_OF_FROM(from)				(from)
#define DIR2_OF_FROM(from)				(((from) + 1) % FROM_NUM)
#define X_DIR_OF_FROM(from)				((((from) + 1) % FROM_NUM) / (FROM_NUM / 2) * (FROM_NUM / 2))
#define Y_DIR_OF_FROM(from)				((from) / (FROM_NUM / 2) * (FROM_NUM / 2) + 1)
#define IS_FROM_DIR(from, dir)			((from) == (dir) || ((from) + 1) % FROM_NUM == (dir))
#define MAKE_SHAPE_ID(l, t, r, b)		(((l) << LEFT) | ((t) << TOP) | ((r) << RIGHT) | ((b) << BOTTOM))
#define LOC_MAP_POS(data, pos, size)	((data) + (pos)->x + (pos)->y * CALC_ISOM_ROW((size)->cx))
#define CALC_DIRTY_SIZE(size)			((CALC_ISOM_ROW((size)->cx) * CALC_ISOM_LINE((size)->cy)) >> 3)
#define SET_DIRTY(dirty, pos, size)		(*((BUFPTR)(dirty) + (((pos)->x + (pos)->y * CALC_ISOM_ROW((size)->cx)) >> 3)) |= (1 << ((pos)->x & 0x07)))
#define GET_DIRTY(dirty, pos, size)		(*((BUFPTR)(dirty) + (((pos)->x + (pos)->y * CALC_ISOM_ROW((size)->cx)) >> 3)) & (1 << ((pos)->x & 0x07)))

// TODO:
#define M_CENTER						CENTER_FLAG
#define M_DIFFUSE(n)					(DIFFUSE_FLAG | (n))
#define M_CLIFF(n)						(CLIFF_FLAG | (n))
#define M_SIDE(n)						(SIDE_FLAG | (n))

/************************************************************************/

/* lL tL rH bH */
#define LLHH_PARAM			{ \
		{ LOW, LOW, HIGH, HIGH },										/* order */ \
		{ AS_LOW, AS_EDGE, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_SIDE(3), M_SIDE(3) },					/* x_abut */ \
		{ M_CENTER, M_SIDE(3) },										/* y_abut */ \
	}

/* lH tL rL bH */
#define HLLH_PARAM			{ \
		{ HIGH, LOW, LOW, HIGH },										/* order */ \
		{ AS_EDGE, AS_LOW, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_SIDE(1), M_SIDE(1) },					/* x_abut */ \
		{ M_SIDE(1), M_CENTER },										/* y_abut */ \
	}

/* lH tH rL bL */
#define HHLL_PARAM			{ \
		{ HIGH, HIGH, LOW, LOW },										/* order */ \
		{ AS_EDGE, AS_EDGE, M_CLIFF(AS_LOW), AS_EDGE },					/* type */ \
		{ M_SIDE(4), M_SIDE(4), M_CLIFF(0), M_CLIFF(0) },				/* x_abut */ \
		{ M_SIDE(4), M_CLIFF(0) },										/* y_abut */ \
	}

/* lL tH rH bL */
#define LHHL_PARAM			{ \
		{ LOW, HIGH, HIGH, LOW },										/* order */ \
		{ AS_EDGE, AS_EDGE, AS_EDGE, M_CLIFF(AS_LOW) },					/* type */ \
		{ M_SIDE(2), M_SIDE(2), M_CLIFF(1), M_CLIFF(1) },				/* x_abut */ \
		{ M_CLIFF(1), M_SIDE(2) },										/* y_abut */ \
	}

/* lL tL rL bH */
#define LLLH_PARAM			{ \
		{ LOW, LOW, LOW, HIGH },										/* order */ \
		{ AS_LOW, AS_LOW, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_SIDE(1), M_SIDE(3) }, 					/* x_abut */ \
		{ M_CENTER, M_CENTER },											/* y_abut */ \
	}

/* lH tL rL bL */
#define HLLL_PARAM			{ \
		{ HIGH, LOW, LOW, LOW },										/* order */ \
		{ AS_EDGE, AS_LOW, AS_LOW, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_CENTER, M_CLIFF(3) },					/* x_abut */ \
		{ M_SIDE(6), M_CENTER },										/* y_abut */ \
	}

/* lL tH rL bL */
#define LHLL_PARAM			{ \
		{ LOW, HIGH, LOW, LOW },										/* order */ \
		{ AS_EDGE, AS_EDGE, M_CLIFF(AS_LOW), M_CLIFF(AS_LOW) },			/* type */ \
		{ M_SIDE(2), M_SIDE(4), M_CLIFF(0), M_CLIFF(1) },				/* x_abut */ \
		{ M_CLIFF(1), M_CLIFF(0) },										/* y_abut */ \
	}

/* lL tL rH bL */
#define LLHL_PARAM			{ \
		{ LOW, LOW, HIGH, LOW },										/* order */ \
		{ AS_LOW, AS_EDGE, AS_EDGE, AS_LOW },							/* type */ \
		{ M_CENTER, M_CENTER, M_CLIFF(2), M_CENTER },					/* x_abut */ \
		{ M_CENTER, M_SIDE(5) },										/* y_abut */ \
	}

/* lL tH rH bH */
#define LHHH_PARAM			{ \
		{ LOW, HIGH, HIGH, HIGH },										/* order */ \
		{ AS_EDGE, M_DIFFUSE(AS_HIGH), M_DIFFUSE(AS_HIGH), AS_EDGE },	/* type */ \
		{ M_SIDE(2), M_DIFFUSE(8), M_DIFFUSE(8), M_SIDE(3) },			/* x_abut */ \
		{ M_CLIFF(1), M_DIFFUSE(8) },									/* y_abut */ \
	}

/* lH tH rL bH */
#define HHLH_PARAM			{ \
		{ HIGH, HIGH, LOW, HIGH },										/* order */ \
		{ M_DIFFUSE(AS_HIGH), AS_EDGE, AS_EDGE, M_DIFFUSE(AS_HIGH) },	/* type */ \
		{ M_DIFFUSE(7), M_SIDE(4), M_SIDE(1), M_DIFFUSE(7) },			/* x_abut */ \
		{ M_DIFFUSE(7), M_CLIFF(0) },									/* y_abut */ \
	}

/* lH tL rH bH */
#define HLHH_PARAM			{ \
		{ HIGH, LOW, HIGH, HIGH },										/* order */ \
		{ AS_EDGE, AS_EDGE, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_SIDE(3), M_SIDE(1) },					/* x_abut */ \
		{ M_SIDE(1), M_SIDE(3) },										/* y_abut */ \
	}

/* lH tH rH bL */
#define HHHL_PARAM			{ \
		{ HIGH, HIGH, HIGH, LOW },										/* order */ \
		{ AS_EDGE, AS_EDGE, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_SIDE(4), M_SIDE(2), M_CLIFF(1), M_CLIFF(0) },				/* x_abut */ \
		{ M_SIDE(4), M_SIDE(2) },										/* y_abut */ \
	}

/* lL tH rL bH */
#define LHLH_PARAM			{ \
		{ LOW, HIGH, LOW, HIGH },										/* order */ \
		{ AS_EDGE, AS_EDGE, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_SIDE(2), M_SIDE(4), M_SIDE(1), M_SIDE(3) },					/* x_abut */ \
		{ M_CLIFF(1), M_CLIFF(0) },										/* y_abut */ \
	}

/* lH tL rH bL */
#define HLHL_PARAM			{ \
		{ HIGH, LOW, HIGH, LOW },										/* order */ \
		{ AS_EDGE, AS_EDGE, AS_EDGE, AS_EDGE },							/* type */ \
		{ M_CENTER, M_CENTER, M_CLIFF(2), M_CLIFF(3) },					/* x_abut */ \
		{ M_SIDE(6), M_SIDE(5) },										/* y_abut */ \
	}

/* L_ERA_BADLANDS */
#define BADLANDS_PARAM		{ \
		0x0d,	/* edge_start */ \
		{	/* center */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 0:dirt					L_BRUSH_BADLANDS_DIRT */ \
			{ 0x09, 0x04, 0x0f, S_NONE },		/* 1:mud					L_BRUSH_BADLANDS_MUD */ \
			{ 0x02, 0x03, 0x04, S_NONE },		/* 2:high-dirt				L_BRUSH_BADLANDS_HIGH_DIRT */ \
			{ 0x03, 0x05, 0x05, S_NONE },		/* 3:water					L_BRUSH_BADLANDS_WATER */ \
			{ 0x04, 0x06, 0x02, S_NONE },		/* 4:grass					L_BRUSH_BADLANDS_GRASS */ \
			{ 0x07, 0x07, 0x03, S_NONE },		/* 5:high-grass				L_BRUSH_BADLANDS_HIGH_GRASS */ \
			{ 0x08, 0x12, 0x0e, S_MATTED },		/* 6:structure				L_BRUSH_BADLANDS_STRUCTURE */ \
			{ 0x05, 0x0e, 0x0a, S_TILE },		/* 7:asphalt				L_BRUSH_BADLANDS_ASPHALT */ \
			{ 0x06, 0x0f, 0x0b, S_DIFFUSE },	/* 8:rocky-ground			L_BRUSH_BADLANDS_ROCKY_GROUND */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x22, { 0x1e, 0x1f, 0x20, 0x21 } },	/* 0:0D-1A dirt/high-dirt */ \
			{ 0x03, 0x01, FALSE, 0x23, { 0x22, 0x23, 0x24, 0x25 } },	/* 1:1B-28 water/dirt */ \
			{ 0x01, 0x04, FALSE, 0x14, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:29-36 dirt/grass */ \
			{ 0x01, 0x06, FALSE, 0x1c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:37-44 dirt/rocky-ground */ \
			{ 0x02, 0x07, FALSE, 0x15, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:45-52 high-dirt/high-grass */ \
			{ 0x01, 0x05, FALSE, 0x1b, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:53-60 dirt/asphalt */ \
			{ 0x05, 0x08, FALSE, 0x1f, { 0x1a, 0x1b, 0x1c, 0x1d } },	/* 6:61-6E asphalt/structure */ \
			{ 0x01, 0x09, FALSE, 0x16, { 0x00, 0x00, 0x00, 0x00 } },	/* 7:6F-7C dirt/mud */ \
		}, \
	}

/* L_ERA_PLATFORM */
#define PLATFORM_PARAM		{ \
		0x18,	/* edge_start */ \
		{	/* center */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 0:space					L_BRUSH_PLATFORM_SPACE */ \
			{ 0x09, 0x08, 0x0d, S_MATTED },		/* 1:low-platform			L_BRUSH_PLATFORM_LOW_PLATFORM */ \
			{ 0x0a, 0x09, 0x12, S_MATTED },		/* 2:rusty-pit				L_BRUSH_PLATFORM_RUSTY_PIT */ \
			{ 0x02, 0x03, 0x02, S_NONE },		/* 3:platform				L_BRUSH_PLATFORM_PLATFORM */ \
			{ 0x0e, 0x0b, 0x07, S_TILE },		/* 4:dark-platform			L_BRUSH_PLATFORM_DARK_PLATFORM */ \
			{ 0x0b, 0x04, 0x03, S_TILE },		/* 5:plating				L_BRUSH_PLATFORM_PLATING */ \
			{ 0x08, 0x07, 0x06, S_TILE },		/* 6:solar-array			L_BRUSH_PLATFORM_SOLAR_ARRAY */ \
			{ 0x04, 0x05, 0x04, S_NONE },		/* 7:high-platform			L_BRUSH_PLATFORM_HIGH_PLATFORM */ \
			{ 0x0c, 0x06, 0x05, S_TILE },		/* 8:high-plating			L_BRUSH_PLATFORM_HIGH_PLATING */ \
			{ 0x0d, 0x0a, 0x08, S_MATTED },		/* 9:elevated-catwalk		L_BRUSH_PLATFORM_ELEVATED_CATWALK */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x14, { 0x17, 0x18, 0x19, 0x1a } },	/* 0:18-25 space/platform */ \
			{ 0x02, 0x04, FALSE, 0x15, { 0x1b, 0x1c, 0x1d, 0x1e } },	/* 1:26-33 platform/high-platform */ \
			{ 0x02, 0x08, FALSE, 0x10, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:34-41 platform/solar-array */ \
			{ 0x02, 0x09,  TRUE, 0x11, { 0x0e, 0x0f, 0x10, 0x11 } },	/* 3:42-4F low-platform/platform */ \
			{ 0x02, 0x0a,  TRUE, 0x12, { 0x13, 0x14, 0x15, 0x16 } },	/* 4:50-5D rusty-pit/platform */ \
			{ 0x02, 0x0b, FALSE, 0x0e, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:5E-6B platform/plating */ \
			{ 0x04, 0x0c, FALSE, 0x0f, { 0x00, 0x00, 0x00, 0x00 } },	/* 6:6C-79 high-platform/high-plating */ \
			{ 0x02, 0x0d, FALSE, 0x13, { 0x09, 0x0a, 0x0b, 0x0c } },	/* 7:7A-87 platform/elevated-catwalk */ \
			{ 0x02, 0x0e, FALSE, 0x0d, { 0x00, 0x00, 0x00, 0x00 } },	/* 8:88-95 platform/dark-platform */ \
		}, \
	}

/* L_ERA_INSTALL */
#define INSTALL_PARAM		{ \
		0x16,	/* edge_start */ \
		{	/* center */ \
			{ 0x01, 0x02, 0x01, S_STEP },		/* 0:substructure			L_BRUSH_INSTALL_SUBSTRUCTURE */ \
			{ 0x02, 0x03, 0x02, S_STEP },		/* 1:floor					L_BRUSH_INSTALL_FLOOR */ \
			{ 0x03, 0x06, 0x03, S_ROOF },		/* 2:roof					L_BRUSH_INSTALL_ROOF */ \
			{ 0x04, 0x04, 0x04, S_TILE },		/* 3:substructure-plating	L_BRUSH_INSTALL_SUBSTRUCTURE_PLATING */ \
			{ 0x05, 0x05, 0x05, S_TILE },		/* 4:plating				L_BRUSH_INSTALL_PLATING */ \
			{ 0x06, 0x08, 0x07, S_TILE },		/* 5:substructure-panels	L_BRUSH_INSTALL_SUBSTRUCTURE_PANELS */ \
			{ 0x07, 0x07, 0x06, S_NONE },		/* 6:bottomless-pit			L_BRUSH_INSTALL_BOTTOMLESS_PIT */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x0c, { 0x08, 0x09, 0x0a, 0x0b } },	/* 0:16-23 substructure/floor */ \
			{ 0x02, 0x03, FALSE, 0x0d, { 0x0c, 0x0d, 0x0e, 0x0f } },	/* 1:24-31 floor/roof */ \
			{ 0x01, 0x04, FALSE, 0x0a, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:32-3F substructure/substructure-plating */ \
			{ 0x02, 0x05, FALSE, 0x0b, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:40-4D floor/plating */ \
			{ 0x01, 0x06, FALSE, 0x0e, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:4E-5B substructure/substructure-panels */ \
			{ 0x07, 0x01, FALSE, 0x0f, { 0x10, 0x11, 0x12, 0x13 } },	/* 5:5C-69 bottomless-pit/substructure */ \
		}, \
	}

/* L_ERA_ASHWORLD */
#define ASHWORLD_PARAM		{ \
		0x1b,	/* edge_start */ \
		{	/* center */ \
			{ 0x01, 0x08, 0x07, S_NONE },		/* 0:magma					L_BRUSH_ASHWORLD_MAGMA */ \
			{ 0x02, 0x02, 0x01, S_NONE },		/* 1:dirt					L_BRUSH_ASHWORLD_DIRT */ \
			{ 0x03, 0x03, 0x02, S_TILE },		/* 2:lava					L_BRUSH_ASHWORLD_LAVA */ \
			{ 0x04, 0x06, 0x05, S_DIFFUSE },	/* 3:shale					L_BRUSH_ASHWORLD_SHALE */ \
			{ 0x08, 0x09, 0x08, S_TILE },		/* 4:broken-rock			L_BRUSH_ASHWORLD_BROKEN_ROCK */ \
			{ 0x05, 0x04, 0x03, S_NONE },		/* 5:high-dirt				L_BRUSH_ASHWORLD_HIGH_DIRT */ \
			{ 0x06, 0x05, 0x04, S_TILE },		/* 6:high-lava				L_BRUSH_ASHWORLD_HIGH_LAVA */ \
			{ 0x07, 0x07, 0x06, S_DIFFUSE },	/* 7:high-shale				L_BRUSH_ASHWORLD_HIGH_SHALE */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x11, { 0x09, 0x0a, 0x0b, 0x0c } },	/* 0:1B-28 magma/dirt */ \
			{ 0x02, 0x05, FALSE, 0x10, { 0x0d, 0x0e, 0x0f, 0x10 } },	/* 1:29-36 dirt/high-dirt */ \
			{ 0x02, 0x03, FALSE, 0x0b, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:37-44 dirt/lava */ \
			{ 0x05, 0x06, FALSE, 0x0c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:45-52 high-dirt/high-lava */ \
			{ 0x02, 0x04, FALSE, 0x0d, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:53-60 dirt/shale */ \
			{ 0x05, 0x07, FALSE, 0x0e, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:61-6E high-dirt/high-shale */ \
			{ 0x02, 0x08, FALSE, 0x0f, { 0x00, 0x00, 0x00, 0x00 } },	/* 6:6F-7C dirt/broken-rock */ \
		}, \
	}

/* L_ERA_JUNGLE */
#define JUNGLE_PARAM		{ \
		0x11,	/* edge_start */ \
		{	/* center */ \
			{ 0x03, 0x05, 0x05, S_NONE },		/* 0:water					L_BRUSH_JUNGLE_WATER */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 1:dirt					L_BRUSH_JUNGLE_DIRT */ \
			{ 0x0d, 0x04, 0x0f, S_NONE },		/* 2:mud					L_BRUSH_JUNGLE_MUD */ \
			{ 0x04, 0x08, 0x08, S_NONE },		/* 3:jungle					L_BRUSH_JUNGLE_JUNGLE */ \
			{ 0x06, 0x0f, 0x0b, S_DIFFUSE },	/* 4:rocky-ground			L_BRUSH_JUNGLE_ROCKY_GROUND */ \
			{ 0x07, 0x0b, 0x06, S_DIFFUSE },	/* 5:ruins					L_BRUSH_JUNGLE_RUINS */ \
			{ 0x05, 0x09, 0x0c, S_DIFFUSE },	/* 6:raised-jungle			L_BRUSH_JUNGLE_RAISED_JUNGLE */ \
			{ 0x08, 0x10, 0x10, S_MATTED },		/* 7:temple					L_BRUSH_JUNGLE_TEMPLE */ \
			{ 0x02, 0x03, 0x04, S_NONE },		/* 8:high-dirt				L_BRUSH_JUNGLE_HIGH_DIRT */ \
			{ 0x09, 0x0a, 0x09, S_NONE },		/* 9:high-jungle			L_BRUSH_JUNGLE_HIGH_JUNGLE */ \
			{ 0x0a, 0x0c, 0x07, S_DIFFUSE },	/* a:high-ruins				L_BRUSH_JUNGLE_HIGH_RUINS */ \
			{ 0x0b, 0x0d, 0x0d, S_DIFFUSE },	/* b:high-raised-jungle		L_BRUSH_JUNGLE_HIGH_RAISED_JUNGLE */ \
			{ 0x0c, 0x11, 0x11, S_MATTED },		/* c:high-temple			L_BRUSH_JUNGLE_HIGH_TEMPLE */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x22, { 0x1e, 0x1f, 0x20, 0x21 } },	/* 0:11-1E dirt/high-dirt */ \
			{ 0x03, 0x01, FALSE, 0x23, { 0x22, 0x23, 0x24, 0x25 } },	/* 1:1F-2C water/dirt */ \
			{ 0x01, 0x04, FALSE, 0x17, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:2D-3A dirt/jungle */ \
			{ 0x01, 0x06, FALSE, 0x1c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:3B-48 dirt/rocky-ground */ \
			{ 0x04, 0x05, FALSE, 0x1d, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:49-56 jungle/raised jungle */ \
			{ 0x04, 0x07, FALSE, 0x19, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:57-64 jungle/ruins */ \
			{ 0x04, 0x08, FALSE, 0x20, { 0x12, 0x13, 0x14, 0x15 } },	/* 6:65-72 jungle/temple */ \
			{ 0x02, 0x09, FALSE, 0x18, { 0x00, 0x00, 0x00, 0x00 } },	/* 7:73-80 high-dirt/high-jungle */ \
			{ 0x09, 0x0a, FALSE, 0x1a, { 0x00, 0x00, 0x00, 0x00 } },	/* 8:81-8E high-jungle/high-ruins */ \
			{ 0x09, 0x0b, FALSE, 0x1e, { 0x00, 0x00, 0x00, 0x00 } },	/* 9:8F-9C high-jungle/high-raised-jungle */ \
			{ 0x09, 0x0c, FALSE, 0x21, { 0x16, 0x17, 0x18, 0x19 } },	/* a:9D-AA high-jungle/high-temple */ \
			{ 0x01, 0x0d, FALSE, 0x16, { 0x00, 0x00, 0x00, 0x00 } },	/* b:AB-B8 dirt/mud */ \
		}, \
	}

/* L_ERA_DESERT */
#define DESERT_PARAM		{ \
		0x11,	/* edge_start */ \
		{	/* center */ \
			{ 0x03, 0x05, 0x05, S_NONE },		/* 0:tar					L_BRUSH_DESERT_TAR */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 1:dirt					L_BRUSH_DESERT_DIRT */ \
			{ 0x0d, 0x04, 0x0f, S_NONE },		/* 2:dried-mud				L_BRUSH_DESERT_DRIED_MUD */ \
			{ 0x04, 0x08, 0x08, S_NONE },		/* 3:sand-dunes				L_BRUSH_DESERT_SAND_DUNES */ \
			{ 0x06, 0x0f, 0x0b, S_DIFFUSE },	/* 4:rocky-ground			L_BRUSH_DESERT_ROCKY_GROUND */ \
			{ 0x07, 0x0b, 0x06, S_DIFFUSE },	/* 5:crags					L_BRUSH_DESERT_CRAGS */ \
			{ 0x05, 0x09, 0x0c, S_DIFFUSE },	/* 6:sandy-sunken-pit		L_BRUSH_DESERT_SANDY_SUNKEN_PIT */ \
			{ 0x08, 0x10, 0x10, S_MATTED },		/* 7:compound				L_BRUSH_DESERT_COMPOUND */ \
			{ 0x02, 0x03, 0x04, S_NONE },		/* 8:high-dirt				L_BRUSH_DESERT_HIGH_DIRT */ \
			{ 0x09, 0x0a, 0x09, S_NONE },		/* 9:high-sand-dunes		L_BRUSH_DESERT_HIGH_SAND_DUNES */ \
			{ 0x0a, 0x0c, 0x07, S_DIFFUSE },	/* a:high-crags				L_BRUSH_DESERT_HIGH_CRAGS */ \
			{ 0x0b, 0x0d, 0x0d, S_DIFFUSE },	/* b:high-sandy-sunken-pit	L_BRUSH_DESERT_HIGH_SANDY_SUNKEN_PIT */ \
			{ 0x0c, 0x11, 0x11, S_MATTED },		/* c:high-compound			L_BRUSH_DESERT_HIGH_COMPOUND */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x22, { 0x1e, 0x1f, 0x20, 0x21 } },	/* 0:11-1E dirt/high-dirt */ \
			{ 0x03, 0x01, FALSE, 0x23, { 0x22, 0x23, 0x24, 0x25 } },	/* 1:1F-2C tar/dirt */ \
			{ 0x01, 0x04, FALSE, 0x17, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:2D-3A dirt/sand-dunes */ \
			{ 0x01, 0x06, FALSE, 0x1c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:3B-48 dirt/rocky-ground */ \
			{ 0x04, 0x05, FALSE, 0x1d, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:49-56 sand-dunes/sandy-sunken-pit */ \
			{ 0x04, 0x07, FALSE, 0x19, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:57-64 sand-dunes/crags */ \
			{ 0x04, 0x08, FALSE, 0x20, { 0x12, 0x13, 0x14, 0x15 } },	/* 6:65-72 sand-dunes/compound */ \
			{ 0x02, 0x09, FALSE, 0x18, { 0x00, 0x00, 0x00, 0x00 } },	/* 7:73-80 high-dirt/high-sand-dunes */ \
			{ 0x09, 0x0a, FALSE, 0x1a, { 0x00, 0x00, 0x00, 0x00 } },	/* 8:81-8E high-sand-dunes/high-crags */ \
			{ 0x09, 0x0b, FALSE, 0x1e, { 0x00, 0x00, 0x00, 0x00 } },	/* 9:8F-9C high-sand-dunes/high-sandy-sunken-pit */ \
			{ 0x09, 0x0c, FALSE, 0x21, { 0x16, 0x17, 0x18, 0x19 } },	/* a:9D-AA high-sand-dunes/high-compound */ \
			{ 0x01, 0x0d, FALSE, 0x16, { 0x00, 0x00, 0x00, 0x00 } },	/* b:AB-B8 dirt/dried-mud */ \
		}, \
	}

/* L_ERA_ICE */
#define ICE_PARAM			{ \
		0x11,	/* edge_start */ \
		{	/* center */ \
			{ 0x03, 0x05, 0x05, S_NONE },		/* 0:ice					L_BRUSH_ICE_ICE */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 1:snow					L_BRUSH_ICE_SNOW */ \
			{ 0x0d, 0x04, 0x0f, S_NONE },		/* 2:moguls					L_BRUSH_ICE_MOGULS */ \
			{ 0x04, 0x08, 0x08, S_NONE },		/* 3:dirt					L_BRUSH_ICE_DIRT */ \
			{ 0x06, 0x0f, 0x0b, S_DIFFUSE },	/* 4:rocky-snow				L_BRUSH_ICE_ROCKY_SNOW */ \
			{ 0x07, 0x0b, 0x06, S_DIFFUSE },	/* 5:grass					L_BRUSH_ICE_GRASS */ \
			{ 0x05, 0x09, 0x0c, S_DIFFUSE },	/* 6:water					L_BRUSH_ICE_WATER */ \
			{ 0x08, 0x10, 0x10, S_MATTED },		/* 7:outpost				L_BRUSH_ICE_OUTPOST */ \
			{ 0x02, 0x03, 0x04, S_NONE },		/* 8:high-snow				L_BRUSH_ICE_HIGH_SNOW */ \
			{ 0x09, 0x0a, 0x09, S_NONE },		/* 9:high-dirt				L_BRUSH_ICE_HIGH_DIRT */ \
			{ 0x0a, 0x0c, 0x07, S_DIFFUSE },	/* a:high-grass				L_BRUSH_ICE_HIGH_GRASS */ \
			{ 0x0b, 0x0d, 0x0d, S_DIFFUSE },	/* b:high-water				L_BRUSH_ICE_HIGH_WATER */ \
			{ 0x0c, 0x11, 0x11, S_MATTED },		/* c:high-outpost			L_BRUSH_ICE_HIGH_OUTPOST */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x22, { 0x1e, 0x1f, 0x20, 0x21 } },	/* 0:11-1E snow/high-snow */ \
			{ 0x03, 0x01, FALSE, 0x23, { 0x22, 0x23, 0x24, 0x25 } },	/* 1:1F-2C ice/snow */ \
			{ 0x01, 0x04, FALSE, 0x17, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:2D-3A snow/dirt */ \
			{ 0x01, 0x06, FALSE, 0x1c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:3B-48 snow/rocky-snow */ \
			{ 0x04, 0x05, FALSE, 0x1d, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:49-56 dirt/water */ \
			{ 0x04, 0x07, FALSE, 0x19, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:57-64 dirt/grass */ \
			{ 0x04, 0x08, FALSE, 0x20, { 0x12, 0x13, 0x14, 0x15 } },	/* 6:65-72 dirt/outpost */ \
			{ 0x02, 0x09, FALSE, 0x18, { 0x00, 0x00, 0x00, 0x00 } },	/* 7:73-80 high-snow/high-dirt */ \
			{ 0x09, 0x0a, FALSE, 0x1a, { 0x00, 0x00, 0x00, 0x00 } },	/* 8:81-8E high-dirt/high-grass */ \
			{ 0x09, 0x0b, FALSE, 0x1e, { 0x00, 0x00, 0x00, 0x00 } },	/* 9:8F-9C high-dirt/high-water */ \
			{ 0x09, 0x0c, FALSE, 0x21, { 0x16, 0x17, 0x18, 0x19 } },	/* a:9D-AA high-dirt/high-outpost */ \
			{ 0x01, 0x0d, FALSE, 0x16, { 0x00, 0x00, 0x00, 0x00 } },	/* b:AB-B8 snow/moguls */ \
		}, \
	}

/* L_ERA_TWILIGHT */
#define TWILIGHT_PARAM		{ \
		0x11,	/* edge_start */ \
		{	/* center */ \
			{ 0x03, 0x05, 0x05, S_NONE },		/* 0:water					L_BRUSH_TWLIGHT_WATER */ \
			{ 0x01, 0x02, 0x01, S_NONE },		/* 1:dirt					L_BRUSH_TWLIGHT_DIRT */ \
			{ 0x0d, 0x04, 0x0f, S_NONE },		/* 2:mud					L_BRUSH_TWLIGHT_MUD */ \
			{ 0x04, 0x08, 0x08, S_NONE },		/* 3:crushed-rock			L_BRUSH_TWLIGHT_CRUSHED_ROCK */ \
			{ 0x06, 0x0f, 0x0b, S_DIFFUSE },	/* 4:crevices				L_BRUSH_TWLIGHT_CREVICES */ \
			{ 0x07, 0x0b, 0x06, S_DIFFUSE },	/* 5:flagstones				L_BRUSH_TWLIGHT_FLAGSTONES */ \
			{ 0x05, 0x09, 0x0c, S_DIFFUSE },	/* 6:sunken-ground			L_BRUSH_TWLIGHT_SUNKEN_GROUND */ \
			{ 0x08, 0x10, 0x10, S_MATTED },		/* 7:basilica				L_BRUSH_TWLIGHT_BASILICA */ \
			{ 0x02, 0x03, 0x04, S_NONE },		/* 8:high-dirt				L_BRUSH_TWLIGHT_HIGH_DIRT */ \
			{ 0x09, 0x0a, 0x09, S_NONE },		/* 9:high-crushed-rock		L_BRUSH_TWLIGHT_HIGH_CRUSHED_ROCK */ \
			{ 0x0a, 0x0c, 0x07, S_DIFFUSE },	/* a:high-flagstones		L_BRUSH_TWLIGHT_HIGH_FLAGSTONES */ \
			{ 0x0b, 0x0d, 0x0d, S_DIFFUSE },	/* b:high-sunken-ground		L_BRUSH_TWLIGHT_HIGH_SUNKEN_GROUND */ \
			{ 0x0c, 0x11, 0x11, S_MATTED },		/* c:high-basilica			L_BRUSH_TWLIGHT_HIGH_BASILICA */ \
		}, \
		{	/* edge */ \
			{ 0x01, 0x02, FALSE, 0x22, { 0x1e, 0x1f, 0x20, 0x21 } },	/* 0:11-1E dirt/high-dirt */ \
			{ 0x03, 0x01, FALSE, 0x23, { 0x22, 0x23, 0x24, 0x25 } },	/* 1:1F-2C water/dirt */ \
			{ 0x01, 0x04, FALSE, 0x17, { 0x00, 0x00, 0x00, 0x00 } },	/* 2:2D-3A dirt/crushed-rock */ \
			{ 0x01, 0x06, FALSE, 0x1c, { 0x00, 0x00, 0x00, 0x00 } },	/* 3:3B-48 dirt/crevices */ \
			{ 0x04, 0x05, FALSE, 0x1d, { 0x00, 0x00, 0x00, 0x00 } },	/* 4:49-56 crushed-rock/raised crushed-rock */ \
			{ 0x04, 0x07, FALSE, 0x19, { 0x00, 0x00, 0x00, 0x00 } },	/* 5:57-64 crushed-rock/flagstones */ \
			{ 0x04, 0x08, FALSE, 0x20, { 0x12, 0x13, 0x14, 0x15 } },	/* 6:65-72 crushed-rock/basilica */ \
			{ 0x02, 0x09, FALSE, 0x18, { 0x00, 0x00, 0x00, 0x00 } },	/* 7:73-80 high-dirt/high-crushed-rock */ \
			{ 0x09, 0x0a, FALSE, 0x1a, { 0x00, 0x00, 0x00, 0x00 } },	/* 8:81-8E high-crushed-rock/high-flagstones */ \
			{ 0x09, 0x0b, FALSE, 0x1e, { 0x00, 0x00, 0x00, 0x00 } },	/* 9:8F-9C high-crushed-rock/high-sunken-ground */ \
			{ 0x09, 0x0c, FALSE, 0x21, { 0x16, 0x17, 0x18, 0x19 } },	/* a:9D-AA high-crushed-rock/high-basilica */ \
			{ 0x01, 0x0d, FALSE, 0x16, { 0x00, 0x00, 0x00, 0x00 } },	/* b:AB-B8 dirt/mud */ \
		}, \
	}

/************************************************************************/

enum ORDER { LOW, HIGH };
enum SIDE { LEFT_SIDE, RIGHT_SIDE, SIDE_NUM };
enum DIR { LEFT, TOP, RIGHT, BOTTOM, DIR_NUM };
enum FROM { LEFT_TOP, TOP_RIGHT, RIGHT_BOTTOM, BOTTOM_LEFT, FROM_NUM };

/* 中央地形的类型枚举 */
enum CENTER_STYLE {
	S_NONE,									/* 非特殊地形 */
	S_MATTED,								/* 带铺垫地形 */
	S_DIFFUSE,								/* 扩散型地形 */
	S_TILE,									/* 方砖型地形 */
	S_STEP,									/* 台阶型地形 */
	S_ROOF,									/* 屋顶型地形 */
};

/* TILE类型映射方式枚举 */
enum TYPE_MAP {
	AS_EDGE,								/* 与边缘本身的TILE类型相同 */
	AS_LOW,									/* 与低层地形的TILE类型相同 */
	AS_HIGH,								/* 与高层地形的TILE类型相同 */
};

/************************************************************************/

/* 边缘形状相关参数结构体 */
struct SHAPE_PARAM {
	INT		order[DIR_NUM];					/* 4个角落菱形各自地势的高低 */
	INT		type[FROM_NUM];					/* 4个角落菱形各自对应的TILE类型信息 */
	INT		x_abut[FROM_NUM];				/* X轴上的水平邻接关系信息 */
											/* 其索引对应关系为： */
											/*	LEFT_TOP		左上菱形的RIGHT */
											/*	TOP_RIGHT		右上菱形的LEFT */
											/*	RIGHT_BOTTOM	右下菱形的LEFT */
											/*	BOTTOM_LEFT		左下菱形的RIGHT */
	INT		y_abut[SIDE_NUM];				/* Y轴上的水平邻接关系信息 */
											/* 其索引对应关系为： */
											/*	LEFT_SIDE		左上菱形的BOTTOM与左下菱形的TOP */
											/*	RIGHT_SIDE		右上菱形的BOTTOM与右下菱形的TOP */
};

/* ERA相关参数结构体 */
struct ERA_PARAM {
	WORD		edge_start;					/* 边缘的起始ISOM值 */
											/* 从其开始每连续SHAPE_NUM个为一组，对应一个edge数组成员 */
	struct {
		WORD	isom;						/* 对应的ISOM值 */
		WORD	type;						/* 对应的TILE类型 */
		WORD	abut;						/* 水平邻接关系ID */
		WORD	style;						/* 中央地形类型 */
	} center[MAX_CENTER];					/* 中央地形参数 */
	struct {
		WORD	low;						/* 低层地形的ISOM值 */
		WORD	high;						/* 高层地形的ISOM值 */
		BOOL	upend;						/* 在生成ISOM值时是否反转上下关系 */
											/* 仅有Platform中的Low Platform和Rusty Pit具有这种特性 */
		WORD	type;						/* 边缘过渡TILE类型 */
		WORD	cliff[CLIFF_NUM];			/* 悬崖水平邻接关系ID（仅悬崖边缘有效） */
	} edge[MAX_EDGE];						/* 边缘地形参数 */
};

/* ERA相关信息，初始化时由ERA_PARAM生成 */
struct ERA_INFO {
	BOOL		init_flag;					/* 初始化标志 */
	INT			center_num;					/* 中央地形数（center数组有效成员数） */
	INT			edge_num;					/* 边缘地形数（edge数组有效成员数） */
	INT			max_center_type;			/* 中央地形类型的最大值 */
	struct {
		INT		below;						/* 下方能连接的地形索引 */
		INT		below_edge;					/* 下方连接所形成边缘的索引 */
		INT		above_num;					/* 上方能连接的地形个数（above、above_edge有效成员数） */
		INT		above[MAX_LINK];			/* 上方能连接的地形索引的数组 */
		INT		above_edge[MAX_LINK];		/* 上方连接所形成边缘的索引的数组 */
	} center[MAX_CENTER];					/* 中央地形信息 */
	struct {
		INT		low;						/* 低层地形的地形索引 */
		INT		high;						/* 高层地形的地形索引 */
		BOOL	cliff;						/* 是否悬崖边缘 */
		INT		shape[SHAPE_NUM][DIR_NUM];	/* 各边缘形状的各角落菱形的地形索引 */
	} edge[MAX_EDGE];						/* 边缘地形信息 */
};

/* ISOM菱形映射TILE的信息的结构体 */
struct TILE_MAP {
	WORD	type[FROM_NUM];					/* ISOM菱形的4个角落菱形的TILE类型 */
	WORD	x_abut[FROM_NUM];				/* ISOM菱形的4个角落菱形的X轴上的（水平）邻接ID */
	WORD	y_abut[FROM_NUM];				/* ISOM菱形的4个角落菱形的Y轴上的（水平）邻接ID */
	WORD	z_abut[FROM_NUM];				/* ISOM菱形的4个角落菱形的Z轴上的（垂直）邻接ID */
	WORD	proj[SIDE_NUM];					/* 悬崖地形投影到下方TILE菱形上的垂直邻接ID */
};

/* 坐标信息队列节点结构体 */
struct POS_QUENE {
	POINT				pos;				/* 坐标信息 */
	struct POS_QUENE	*next;				/* 队列中的下一节点 */
};

/************************************************************************/

/* 偶菱形的位置值（left, top, right, bottom顺序） */
static CONST WORD TILE_POS_EVEN[DIR_NUM] = { 0x8, 0xa, 0x0, 0x2 };

/* 奇菱形的位置值（left, top, right, bottom顺序） */
static CONST WORD TILE_POS_ODD[DIR_NUM]  = { 0x4, 0xc, 0xe, 0x6 };

/* 14种边缘组合形状 */
static CONST struct SHAPE_PARAM SHAPE_TABLE[SHAPE_NUM] = {
	LLHH_PARAM, HLLH_PARAM, HHLL_PARAM, LHHL_PARAM,
	LLLH_PARAM, HLLL_PARAM, LHLL_PARAM, LLHL_PARAM,
	LHHH_PARAM, HHLH_PARAM, HLHH_PARAM, HHHL_PARAM,
	LHLH_PARAM, HLHL_PARAM,
};

/* 各ERA的ISOM参数表 */
static CONST struct ERA_PARAM PARAM_TABLE[L_ERA_NUM] = {
	BADLANDS_PARAM, PLATFORM_PARAM, INSTALL_PARAM, ASHWORLD_PARAM,
	JUNGLE_PARAM, DESERT_PARAM, ICE_PARAM, TWILIGHT_PARAM,
};

/* 全局变量 */
static BOOL g_InitFlag;
static UINT g_TileDictNum[L_ERA_NUM];
static ISOM_DICT *g_TileDict[L_ERA_NUM];
static struct ERA_INFO g_EraInfo[L_ERA_NUM];
static INT g_Isom2Center[L_ERA_NUM][MAX_CENTER_ISOM];
static INT g_Shape2Index[MAX_SHAPE_ID];
static struct POS_QUENE g_PosQueneHead;
static struct POS_QUENE *g_PosQueneTail;

/************************************************************************/

/* 公用函数 */
static BOOL validate_iso_map(CONST ISOM_MAP *map, BOOL create);
static BOOL check_pos(CONST ISOM_MAP *map, CONST POINT *pos);
static VOID calc_corner_pos(INT from, CONST POINT *base, POINT *corner);
static VOID calc_link_pos(INT from, CONST POINT *base, POINT *link);
static WORD get_center_isom(INT era, INT center);
static WORD get_edge_isom(INT era, INT edge, INT shape);
static INT isom_to_center(INT era, WORD isom);
static BOOL isom_to_edge_shape(INT era, WORD isom, INT *edge, INT *shape);
static INT shape_to_index(INT left, INT top, INT right, INT bottom);

/* ISOM值生成相关函数 */
static BOOL isometric_brush(ISOM_MAP *map, INT brush, CONST POINT *pos);
static WORD isometric_link(ISOM_MAP *map, WORD isom, INT from, CONST POINT *pos);
static BOOL update_isom(ISOM_MAP *map, WORD isom, INT from, CONST POINT *pos);
static BOOL set_tile_pos(ISOM_MAP *map, CONST POINT *pos);
static BOOL get_isom_shape(INT era, WORD isom, INT *low, INT *high, INT *shape_info);
static WORD match_shape(INT era, CONST INT *shape_info);
static INT search_brush_link(INT era, INT brush_from, INT brush_to);

/* TILE映射相关函数 */
static BOOL make_tile_map(CONST ISOM_MAP *map, CONST POINT *pos, ISOM_TILE *tile);
static VOID adjust_dirty_map(CONST ISOM_MAP *map, CONST POINT *pos, CONST ISOM_TILE *isom);
static VOID update_tile(CONST ISOM_MAP *map, CONST POINT *pos, CONST ISOM_TILE *isom);
static VOID map_isom_tile(CONST ISOM_MAP *map, CONST struct TILE_MAP *tile_map, CONST POINT *pos, ISOM_TILE *tile);
static CONST ISOM_DICT *lookup_tile(INT era, CONST ISOM_TILE *tile);
static INT gen_mega_tile_index(INT era, INT map_cx, INT y, CONST ISOM_DICT *dict, CONST ISOM_TILE *isom, LTILECPTR tile);
static WORD map_edge_tile_type(INT era, INT low, INT high, INT edge, INT temp);
static WORD map_edge_hor_abuttal(INT era, INT low, INT high, INT edge, INT temp, WORD *proj);
static WORD map_edge_ver_abuttal(INT era, INT low, INT high, INT edge, INT temp);
static VOID project_abuttal(CONST ISOM_MAP *map, ISOM_TILE *tile, INT from, WORD proj, CONST POINT *pos);

/* 坐标信息队列操作相关函数 */
static BOOL init_pos_quene(CONST POINT *pos);
static VOID exit_pos_quene(VOID);
static BOOL push_pos_quene(CONST POINT *pos);
static BOOL pop_pos_quene(VOID);
static CONST POINT *peek_pos_quene(VOID);
static BOOL is_pos_quene_empty(VOID);

/************************************************************************/

BOOL init_iso_map(VOID)
{
	INT i, shape;
	CONST INT *order;

	/* 如已初始化，什么都不做 */
	if (g_InitFlag)
		return TRUE;

	/* 先将形状索引映射表全部填为无效值 */
	DMemSet(g_Shape2Index, -1, sizeof(g_Shape2Index));

	/* 初始化形状索引映射表，以便从具体形状找到对应的边缘形状索引 */
	for (i = 0; i < SHAPE_NUM; i++) {
		order = SHAPE_TABLE[i].order;
		shape = MAKE_SHAPE_ID(order[LEFT], order[TOP], order[RIGHT], order[BOTTOM]);
		DAssert(DBetween(shape, 0, MAX_SHAPE_ID));
		g_Shape2Index[shape] = i;
	}

	/* 设置初始化标志 */
	g_InitFlag = TRUE;
	return TRUE;
}

VOID exit_iso_map(VOID)
{
	/* 如还未初始化则什么都不做 */
	if (!g_InitFlag)
		return;

	/* 清除初始化标志 */
	g_InitFlag = FALSE;

	/* 清除全局变量 */
	DVarClr(g_TileDictNum);
	DVarClr(g_TileDict);
	DVarClr(g_EraInfo);
	DVarClr(g_Isom2Center);
	DVarClr(g_Shape2Index);
}

BOOL init_iso_era(INT era, CONST ISOM_DICT *tile_dict, UINT tile_num)
{
	INT center, edge, shape, low, high;
	UINT size;
	WORD isom;
	ISOM_DICT *dict;
	struct ERA_INFO *info;
	CONST struct ERA_PARAM *param;

	/* 如全局仍未初始化则失败 */
	if (!g_InitFlag)
		return FALSE;

	/* 参数有效性检查 */
	if (!DBetween(era, 0, L_ERA_NUM) || !tile_dict || !tile_num)
		return FALSE;

	/* 如果指定ERA已初始化，则什么都不用做 */
	if (g_EraInfo[era].init_flag)
		return TRUE;

	DAssert(!g_TileDictNum[era] && !g_TileDict[era]);

	/* 创建TILE字典 */
	/* 内存分配 */
	size = tile_num * sizeof(ISOM_DICT);
	dict = DAlloc(size);
	if (!dict)
		return FALSE;

	/* 字典内容设定 */
	DMemCpy(dict, tile_dict, size);
	g_TileDict[era] = dict;
	g_TileDictNum[era] = tile_num;

	/* 用ERA参数初始化ERA信息 */
	param = &PARAM_TABLE[era];
	info = &g_EraInfo[era];

	/* 计数器清零 */
	info->center_num = 0;
	info->edge_num = 0;

	/* 将ISOM值到中央地形类型的查找表的内容全部初始化为无效值 */
	DMemSet(&g_Isom2Center[era], -1, sizeof(g_Isom2Center[era]));

	/* 初始化中央地形信息 */
	for (center = 0; center < MAX_CENTER; center++) {
		info->center[center].above_num = 0;
		info->center[center].below = -1;
		info->center[center].below_edge = -1;
		DMemSet(info->center[center].above, -1, sizeof(info->center[center].above));
		DMemSet(info->center[center].above_edge, -1, sizeof(info->center[center].above_edge));
	}

	/* 循环建立各种到中央地形的查找表，同时统计中央地形类型数目 */
	for (center = 0; center < MAX_CENTER; center++) {

		/* 遇0表示结束 */
		isom = param->center[center].isom;
		if (!isom)
			break;

		/* 中央地形类型最大值的统计处理 */
		if (param->center[center].type > info->max_center_type)
			info->max_center_type = param->center[center].type;

		DAssert(isom < MAX_CENTER_ISOM);

		/* 填充ISOM值到中央地形类型的查找表 */
		g_Isom2Center[era][isom] = center;

		/* 累加中央地形类型计数 */
		info->center_num++;
	}

	/* 循环生成边缘形状信息，同时获得边缘数目 */
	for (edge = 0; edge < MAX_EDGE; edge++) {

		/* 遇0表示结束 */
		if (!param->edge[edge].low || !param->edge[edge].high)
			break;

		DAssert(param->edge[edge].low < MAX_CENTER_ISOM);
		DAssert(param->edge[edge].high < MAX_CENTER_ISOM);

		/* 确定低层地形和高层地形的地形索引 */
		low = g_Isom2Center[era][param->edge[edge].low];
		high = g_Isom2Center[era][param->edge[edge].high];

		DAssert(low >= 0 && high >= 0);

		/* 如果需要反转上下关系，在此反转 */
		if (param->edge[edge].upend)
			DSwap(low, high);

		/* 填充高低层地形索引值 */
		info->edge[edge].low = low;
		info->edge[edge].high = high;

		/* 判断是否悬崖边缘 */
		info->edge[edge].cliff = param->edge[edge].cliff[0] ? TRUE : FALSE;

		/* 查表生成边缘形状的各角落菱形地形索引信息 */
		for (shape = 0; shape < SHAPE_NUM; shape++) {
			info->edge[edge].shape[shape][LEFT] = SHAPE_TABLE[shape].order[LEFT] ? high : low;
			info->edge[edge].shape[shape][TOP] = SHAPE_TABLE[shape].order[TOP] ? high : low;
			info->edge[edge].shape[shape][RIGHT] = SHAPE_TABLE[shape].order[RIGHT] ? high : low;
			info->edge[edge].shape[shape][BOTTOM] = SHAPE_TABLE[shape].order[BOTTOM] ? high : low;
		}

		/* 累加边缘地形类型计数 */
		info->edge_num++;
	}

	DAssert(info->center_num > 0 && info->edge_num > 0);

	/* 循环建立整个地形的画刷信息 */
	for (center = 0; center < info->center_num; center++) {

		/* 遇0表示结束 */
		isom = param->center[center].isom;
		if (!isom)
			break;

		/* 逐项检查每组边缘组合 */
		for (edge = 0; edge < info->edge_num; edge++) {

			/* 如果是上方的连接 */
			if (param->edge[edge].low == isom) {

				DAssert(info->center[center].above_num <= MAX_LINK);
				DAssert(param->edge[edge].high < MAX_CENTER_ISOM);
				high = g_Isom2Center[era][param->edge[edge].high];
				DAssert(high >= 0);
				info->center[center].above[info->center[center].above_num] = high;
				info->center[center].above_edge[info->center[center].above_num] = edge;
				info->center[center].above_num++;

				continue;
			}

			/* 如果是下方的连接（最多一个） */
			if (param->edge[edge].high == isom) {

				DAssert(param->edge[edge].low < MAX_CENTER_ISOM);
				low = g_Isom2Center[era][param->edge[edge].low];
				DAssert(low >= 0);
				info->center[center].below = low;
				info->center[center].below_edge = edge;

				continue;
			}
		}
	}

	/* 设置初始化标志 */
	info->init_flag = TRUE;
	return TRUE;
}

VOID exit_iso_era(INT era)
{
	/* 参数有效性检查 */
	if (!DBetween(era, 0, L_ERA_NUM))
		return;

	/* 如仍未初始化则什么都不做 */
	if (!g_EraInfo[era].init_flag)
		return;

	/* 清除对应的字典信息 */
	g_TileDictNum[era] = 0;
	DFree(g_TileDict[era]);
	g_TileDict[era] = NULL;

	/* 清除ERA相关信息 */
	DVarClr(g_EraInfo[era]);
	DVarClr(g_Isom2Center[era]);
}

BOOL create_iso_map(ISOM_MAP *map, BOOL new_map)
{
	INT i, j, row, line, center;
	UINT size;
	WORD group, mega;
	VPTR dirty;
	LISOMPTR data;
	LTILEPTR tile;
	LISOMCOORD isom;
	ISOM_TILE index;
	CONST ISOM_DICT *dict;
	CONST struct ERA_PARAM *param;

	/* 如未初始化则失败 */
	if (!g_InitFlag)
		return FALSE;

	/* 参数有效性检查 */
	if (!validate_iso_map(map, TRUE))
		return FALSE;

	row = CALC_ISOM_ROW(map->size.cx);
	line = CALC_ISOM_LINE(map->size.cy);

	size = row * line * sizeof(LISOMTILE);
	data = DAlloc(size);
	if (!data)
		return FALSE;

	/* ISOM初始化 */
	DMemClr(data, size);

	size = map->size.cx * map->size.cy * sizeof(LTILEIDX);
	tile = DAlloc(size);
	if (!tile) {
		DFree(data);
		return FALSE;
	}

	/* TILE初始化 */
	DMemClr(tile, size);

	size = CALC_DIRTY_SIZE(&map->size);
	dirty = DAlloc(size);
	if (!dirty) {
		DFree(data);
		DFree(tile);
		return FALSE;
	}

	/* 脏标志位图初始化 */
	DMemClr(dirty, size);

	map->isom = data;
	map->tile = tile;
	map->dirty = dirty;

	/* 仅仅是初始化结构体而不是新建地图的话到此为止 */
	if (!new_map)
		return TRUE;

	param = &PARAM_TABLE[map->era];

	/* 填充结构设置 */
	isom.pos = 0;
	isom.isom = get_center_isom(map->era, map->def);
	isom.unused = 0;

	/* 以初始值填充每个ISOM菱形 */
	for (i = 0; i < line; i++) {
		for (j = 0; j < row; j++, data++) {
			data->left = isom;
			data->top = isom;
			data->right = isom;
			data->bottom = isom;
		}
	}

	center = isom_to_center(map->era, isom.isom);

	index.type = param->center[center].type;
	index.left_abut = param->center[center].abut;
	index.top_abut = param->center[center].abut;
	index.right_abut = param->center[center].abut;
	index.bottom_abut = param->center[center].abut;
	index.up_abut = V_ABUT_NONE;
	index.down_abut = V_ABUT_NONE;

	dict = lookup_tile(map->era, &index);
	DAssert(dict && dict->group_no);

	/* 以初始值填充每个TILE */
	for (i = 0; i < map->size.cy; i++) {
		for (j = 0; j < map->size.cx; j += 2) {
			group = dict->group_no;
			mega = gen_mega_tile_index(map->era, map->size.cx, i, dict, &index, tile);
			tile->mega_index = mega;
			tile->group_no = group++;
			tile++;
			tile->mega_index = mega;
			tile->group_no = group;
			tile++;
		}
	}

	/* 成功返回 */
	return TRUE;
}

VOID destroy_iso_map(ISOM_MAP *map)
{
	if (!map)
		return;

	DFree(map->isom);
	map->isom = NULL;

	DFree(map->tile);
	map->tile = NULL;

	DFree(map->dirty);
	map->dirty = NULL;
}

BOOL brush_iso_map(ISOM_MAP *map, INT brush, CONST POINT *tile_pos)
{
	POINT pos;

	/* 如未初始化则失败 */
	if (!g_InitFlag)
		return FALSE;

	/* ISOM地图参数有效性检查 */
	if (!validate_iso_map(map, FALSE))
		return FALSE;

	/* 画刷索引值即是中央地形，必须在允许值范围内 */
	if (!DBetween(brush, 0, g_EraInfo[map->era].center_num))
		return FALSE;

	/* TILE坐标参数有效性检查 */
	if (!tile_pos || tile_pos->x < 0 || tile_pos->y < 0)
		return FALSE;

	/*
		调整TILE坐标系到ISOM菱形坐标系。
		须注意对于一个画刷，它的坐标为画刷菱形中心所在位置的ISOM菱形坐标。
		也就是画刷所涉及的周围4个ISOM菱形中，右下侧的那个菱形的的坐标。
	*/
	pos.x = tile_pos->x / 2;
	pos.y = tile_pos->y;

	/* 是否出界 */
	if (pos.x >= CALC_ISOM_ROW(map->size.cx) || pos.y >= CALC_ISOM_LINE(map->size.cy))
		return FALSE;

	/* 画刷中心所在菱形必须为偶菱形 */
	if (pos.x % 2 ^ pos.y % 2)
		return FALSE;

	/* 等角画刷处理 */
	return isometric_brush(map, brush, &pos);
}

BOOL update_iso_map(CONST ISOM_MAP *map)
{
#ifndef NDEBUG
	FILE *fp;
	LTILEPTR tile;
#endif
	INT row, line, size;
	POINT pos;
	ISOM_TILE *isom;

	/* 如未初始化则失败 */
	if (!g_InitFlag)
		return FALSE;

	/* 参数有效性检查 */
	if (!validate_iso_map(map, FALSE))
		return FALSE;

	/* ISOM行列数计算 */
	row = CALC_ISOM_ROW(map->size.cx);
	line = CALC_ISOM_LINE(map->size.cy);

	/* TILE映射表内存分配 */
	size = line * row * sizeof(ISOM_TILE);
	isom = DAlloc(size);
	DMemClr(isom, size);

	/* 首先生成一张粗略的TILE映射表 */
	for (pos.y = 0; pos.y < line; pos.y++) {
		for (pos.x = 0; pos.x < row; pos.x++)
			make_tile_map(map, &pos, isom);
	}

	/* 根据TILE映射表重新调整脏标志位图 */
	for (pos.y = 0; pos.y < line; pos.y++) {
		for (pos.x = 0; pos.x < row; pos.x++)
			adjust_dirty_map(map, &pos, isom);
	}

	/* 由于下面要用到随机数，因此在此初始化 */
	DInitRand();

	/* 修正上面生成的映射表的TILE类型，再查表找到实际对应的TILE */
	for (pos.y = 0; pos.y < line; pos.y++) {
		for (pos.x = 0; pos.x < row; pos.x++)
			update_tile(map, &pos, isom);
	}

#ifndef NDEBUG
	tile = map->tile;
	fp = fopen("isom\\temp.cv5.txt", "w");
	if (fp) {
		for (pos.y = 0; pos.y < line; pos.y++) {
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%02X\t\t", LOC_MAP_POS(isom, &pos, &map->size)->top_abut);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "%02X\t%02X\t%02X\t",
				LOC_MAP_POS(isom, &pos, &map->size)->left_abut,
				LOC_MAP_POS(isom, &pos, &map->size)->type,
				LOC_MAP_POS(isom, &pos, &map->size)->right_abut
				);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%02X\t\t", LOC_MAP_POS(isom, &pos, &map->size)->bottom_abut);
			fputc('\n', fp);
		}
		fclose(fp);
	}
	fp = fopen("isom\\temp.isom.txt", "w");
	if (fp) {
		for (pos.y = 0; pos.y < line; pos.y++) {
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%02X\t\t", LOC_MAP_POS(map->isom, &pos, &map->size)->top.isom);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "%02X\t\t%02X\t",
					LOC_MAP_POS(map->isom, &pos, &map->size)->left.isom,
					LOC_MAP_POS(map->isom, &pos, &map->size)->right.isom
				);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%02X\t\t", LOC_MAP_POS(map->isom, &pos, &map->size)->bottom.isom);
			fputc('\n', fp);
		}
		fclose(fp);
	}
	fp = fopen("isom\\temp.tile.txt", "w");
	if (fp) {
		for (pos.y = 0; pos.y < line; pos.y++) {
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t\t\t");
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%04X\t\t", tile[pos.y*map->size.cx+pos.x*2].group_no);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t\t\t");
			fputc('\n', fp);
		}
		fclose(fp);
	}
	fp = fopen("isom\\temp.cliff.txt", "w");
	if (fp) {
		for (pos.y = 0; pos.y < line; pos.y++) {
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%d\t\t", LOC_MAP_POS(isom, &pos, &map->size)->up_abut);
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t\t\t");
			fputc('\n', fp);
			for (pos.x = 0; pos.x < row; pos.x++)
				fprintf(fp, "\t%d\t\t", LOC_MAP_POS(isom, &pos, &map->size)->down_abut);
			fputc('\n', fp);
		}
		fclose(fp);
	}
#endif

	DFree(isom);

	size = CALC_DIRTY_SIZE(&map->size);
	DMemClr(map->dirty, size);

	return TRUE;
}

/************************************************************************/

static BOOL validate_iso_map(CONST ISOM_MAP *map, BOOL create)
{
	INT row, line;

	if (!map || !DBetween(map->era, 0, L_ERA_NUM))
		return FALSE;

	if (map->size.cx < 0 || map->size.cx > MAX_ISOM_MAP_ROW)
		return FALSE;

	if (map->size.cy < 0 || map->size.cy > MAX_ISOM_MAP_LINE)
		return FALSE;

	row = CALC_ISOM_ROW(map->size.cx);
	line = CALC_ISOM_LINE(map->size.cy);

	if (create)
		return TRUE;

	if (!map->isom || !map->tile || !g_EraInfo[map->era].init_flag)
		return FALSE;

	if (!DBetween(map->def, 0, g_EraInfo[map->era].center_num))
		return FALSE;

	return TRUE;
}

static BOOL check_pos(CONST ISOM_MAP *map, CONST POINT *pos)
{
	DAssert(map && pos);

	if (!DBetween(pos->x, 0, CALC_ISOM_ROW(map->size.cx)))
		return FALSE;

	if (!DBetween(pos->y, 0, CALC_ISOM_LINE(map->size.cy)))
		return FALSE;

	return TRUE;
}

static VOID calc_corner_pos(INT from, CONST POINT *base, POINT *corner)
{
	DAssert(DBetween(from, 0, FROM_NUM) && base && corner);

	/*
		计算画刷各方位菱形的坐标，完成以下映射：
			from			corner.x	corner.y
			LEFT_TOP		base.x - 1	base.y - 1
			TOP_RIGHT		base.x		base.y - 1
			RIGHT_BOTTOM	base.x		base.y
			BOTTOM_LEFT		base.x - 1	base.y
	*/
	corner->x = IS_FROM_DIR(from, LEFT) ? base->x - 1 : base->x;
	corner->y = IS_FROM_DIR(from, TOP) ? base->y - 1 : base->y;
}

static VOID calc_link_pos(INT from, CONST POINT *base, POINT *link)
{
	DAssert(DBetween(from, 0, FROM_NUM) && base && link);

	/*
		计算连接画刷的中心坐标，完成以下映射：
			from			link.x		link.y
			LEFT_TOP		base.x - 1	base.y - 1
			TOP_RIGHT		base.x + 1	base.y - 1
			RIGHT_BOTTOM	base.x + 1	base.y + 1
			BOTTOM_LEFT		base.x - 1	base.y + 1
	*/
	link->x = IS_FROM_DIR(from, LEFT) ? base->x - 1 : base->x + 1;
	link->y = IS_FROM_DIR(from, TOP) ? base->y - 1 : base->y + 1;
}

static WORD get_center_isom(INT era, INT center)
{
	DAssert(DBetween(era, 0, L_ERA_NUM) && g_EraInfo[era].init_flag);
	DAssert(DBetween(center, 0, g_EraInfo[era].center_num));

	return PARAM_TABLE[era].center[center].isom;
}

static WORD get_edge_isom(INT era, INT edge, INT shape)
{
	DAssert(DBetween(era, 0, L_ERA_NUM) && g_EraInfo[era].init_flag);
	DAssert(DBetween(edge, 0, g_EraInfo[era].edge_num));
	DAssert(DBetween(shape, 0, SHAPE_NUM));

	return PARAM_TABLE[era].edge_start + edge * SHAPE_NUM + shape;
}

static INT isom_to_center(INT era, WORD isom)
{
	INT center;

	DAssert(DBetween(era, 0, L_ERA_NUM) && g_InitFlag);
	DAssert(g_EraInfo[era].init_flag);

	if (isom >= PARAM_TABLE[era].edge_start)
		return -1;

	if (isom >= MAX_CENTER_ISOM)
		return -1;

	center = g_Isom2Center[era][isom];
	if (center < 0)
		return -1;

	return center;
}

static BOOL isom_to_edge_shape(INT era, WORD isom, INT *edge, INT *shape)
{
	DAssert(edge && shape);
	DAssert(DBetween(era, 0, L_ERA_NUM) && g_InitFlag);
	DAssert(g_EraInfo[era].init_flag);

	*edge = -1;
	*shape = -1;

	if (isom < PARAM_TABLE[era].edge_start)
		return FALSE;

	*edge = (isom - PARAM_TABLE[era].edge_start) / SHAPE_NUM;
	if (*edge >= g_EraInfo[era].edge_num) {
		*edge = -1;
		return FALSE;
	}

	*shape = (isom - PARAM_TABLE[era].edge_start) % SHAPE_NUM;
	return TRUE;
}

static INT shape_to_index(INT left, INT top, INT right, INT bottom)
{
	INT shape;

	DAssert(g_InitFlag);

	shape = MAKE_SHAPE_ID(left, top, right, bottom);
	if (!DBetween(shape, 0, MAX_SHAPE_ID))
		return -1;

	return g_Shape2Index[shape];
}

static BOOL isometric_brush(ISOM_MAP *map, INT brush, CONST POINT *pos)
{
	INT from;
	WORD isom;
	POINT link_pos;
	WORD update;

	DAssert(map && pos);

	/* 位置队列初始化 */
	if (!init_pos_quene(pos))
		return FALSE;

	/* 先获取画刷对应的中央地形ISOM值 */
	isom = get_center_isom(map->era, brush);

	/* 更新画刷放下位置处的ISOM值 */
	for_each_from (from)
		update_isom(map, isom, from, pos);

	/* 必须使用广度优先 */
	while (!is_pos_quene_empty()) {

		/* 获取当前队首坐标 */
		pos = peek_pos_quene();
		DAssert(pos);

		/* 获取当前ISOM值 */
		isom = LOC_MAP_POS(map->isom, pos, &map->size)->left.isom;
		DAssert(isom == LOC_MAP_POS(map->isom, pos, &map->size)->top.isom);

		// TODO:
		if (!isom)
			continue;

		/* 分别匹配周边四个方向上的连接地形 */
		for_each_from (from) {

			/* 连接地形匹配 */
			update = isometric_link(map, isom, from, pos);
			if (!update)
				continue;

			/* 计算变更了的ISOM菱形的坐标，放入待处理队列 */
			calc_link_pos(from, pos, &link_pos);
			if (!push_pos_quene(&link_pos)) {
				exit_pos_quene();
				return FALSE;
			}
		}

		/* 处理完当前队首坐标，出队 */
		DVerify(pop_pos_quene());
	}

	return TRUE;
}

static WORD isometric_link(ISOM_MAP *map, WORD isom, INT from, CONST POINT *pos)
{
	INT opp, low, high, link_low, link_high, brush1, brush2, brush3, brush4;
	WORD link, new_link;
	POINT corner, link_pos;
	LISOMPTR data;
	INT shape[DIR_NUM], link_shape[DIR_NUM];
	LISOMCOORD *tile;
	CONST struct ERA_PARAM *param;

	DAssert(map && pos && DBetween(from, 0, FROM_NUM));

	/* 地图出界检查 */
	if (!check_pos(map, pos))
		return 0;

	calc_corner_pos(from, pos, &corner);

	/* 地图出界检查 */
	if (!check_pos(map, &corner))
		return 0;

	data = LOC_MAP_POS(map->isom, &corner, &map->size);
	tile = (LISOMCOORD *)&data->left;

	/*
		获取连接对象的ISOM值，对应关系如下：
			from			dir1		dir2
			LEFT_TOP		LEFT		TOP
			TOP_RIGHT		TOP			RIGHT
			RIGHT_BOTTOM	RIGHT		BOTTOM
			BOTTOM_LEFT		BOTTOM		LEFT
	*/
	link = tile[DIR1_OF_FROM(from)].isom;
	DAssert(tile[DIR2_OF_FROM(from)].isom);

	calc_link_pos(from, pos, &link_pos);

	/* 地图出界检查 */
	if (!check_pos(map, &link_pos))
		return 0;

	if (!get_isom_shape(map->era, isom, &low, &high, shape))
		return 0;

	// TODO: 连接目标的ISOM值可能为0，需要处理!!
	if (!get_isom_shape(map->era, link, &link_low, &link_high, link_shape)) {
		DAssert(FALSE);
		return 0;
	}

	param = &PARAM_TABLE[map->era];
	opp = OPPOSITE(from);

	/*
		此处的对应关系为：
			from			brush1	brush2	link_brush1	link_brush2	link_brush3	link_brush4
			LEFT_TOP		LEFT	TOP		BOTTOM		RIGHT		LEFT		TOP
			TOP_RIGHT		TOP		RIGHT	LEFT		BOTTOM		TOP			RIGHT
			RIGHT_BOTTOM	RIGHT	BOTTOM	TOP			LEFT		RIGHT		BOTTOM
			BOTTOM_LEFT		BOTTOM	LEFT	RIGHT		TOP			BOTTOM		LEFT
	*/
	brush1 = shape[DIR1_OF_FROM(from)];
	brush2 = shape[DIR2_OF_FROM(from)];

	/*
		对于非Matted地形，如果当前边界的地形和连接目标边界的地形吻合，则认为可连通，不做任何转换终止即可；
		但对于Matted地形，如果边界上有低层地形，那么连接目标内侧的地形也必须要与之吻合才行。
	*/

	/* 首先强制让边界能够吻合 */
	link_shape[DIR2_OF_FROM(opp)] = brush1;
	link_shape[DIR1_OF_FROM(opp)] = brush2;

	/* 如果修正后的连接形状不合法，则核心思想是根据现有形状信息，找到两个可连接的画刷地形，用这两个画刷去填充那些不相容的菱形。*/

	/* 注意这里的brush1-4，它们是按逆时针顺序排列的，所以对应关系为1-4、2-3。 */
	brush3 = link_shape[DIR2_OF_FROM(from)];
	brush4 = link_shape[DIR1_OF_FROM(from)];

	/*
		若边界两个地形相同，那么brush1（brush2）首先被选中。
		由于brush3和brush4一定是可连接的，所以只要从brush1建立向其中任意一个的连接关系，
		选取其连接链上离brush1最近的一个地形作为另一个地形候补，填充原有的brush3和brush4即可。
	*/
	brush4 = search_brush_link(map->era, brush1, brush4);
	brush3 = search_brush_link(map->era, brush2, brush3);

	/* TODO: comment */
	if (brush1 != brush2) {
		if (brush4 != brush2)
			brush4 = brush1;
		if (brush3 != brush1)
			brush3 = brush2;
	}

	link_shape[DIR2_OF_FROM(from)] = brush3;
	link_shape[DIR1_OF_FROM(from)] = brush4;

	/*
		Matted地形处理。
		首先当前画刷菱形的低层和高层地形相同意味着不是边缘，就一定不需要做Matted地形处理。
		如果当前画刷菱形的高层是Matted地形，同时当前画刷贴近连接方向一侧的地形为低层地形，
		则必须进行如下处理
	*/
	if (low != high) {
		if (param->center[high].style == S_MATTED) {
			if (brush1 == low && link_shape[DIR1_OF_FROM(from)] != high)
				link_shape[DIR1_OF_FROM(from)] = low;
			if (brush2 == low && link_shape[DIR2_OF_FROM(from)] != high)
				link_shape[DIR2_OF_FROM(from)] = low;
		} else {
			brush4 = link_shape[DIR1_OF_FROM(from)];
			brush3 = link_shape[DIR2_OF_FROM(from)];
			if (param->center[brush1].style != S_MATTED && param->center[brush4].style == S_MATTED && shape[DIR2_OF_FROM(opp)] != brush4)
				link_shape[DIR1_OF_FROM(from)] = brush1;
			if (param->center[brush2].style != S_MATTED && param->center[brush3].style == S_MATTED && shape[DIR1_OF_FROM(opp)] != brush3)
				link_shape[DIR2_OF_FROM(from)] = brush2;
		}
	}

	/*
		边界特殊处理。超出边境的菱形形状强制与扩展方向上与其最近的那个菱形的形状相同。
			from			x		x-equ		y		y-equ
			LEFT_TOP		LEFT	BOTTOM		TOP		RIGHT
			TOP_RIGHT		RIGHT	BOTTOM		TOP		LEFT
			RIGHT_BOTTOM	RIGHT	TOP			BOTTOM	LEFT
			BOTTOM_LEFT		LEFT	TOP			BOTTOM	RIGHT
	*/
	if (link_shape[DIR1_OF_FROM(opp)] != link_shape[DIR2_OF_FROM(opp)]) {
		calc_corner_pos(from, &link_pos, &corner);
		if (!DBetween(corner.x, 0, CALC_ISOM_ROW(map->size.cx)))
			link_shape[X_DIR_OF_FROM(from)] = link_shape[Y_DIR_OF_FROM(opp)];
		if (!DBetween(corner.y, 0, CALC_ISOM_LINE(map->size.cy)))
			link_shape[Y_DIR_OF_FROM(from)] = link_shape[X_DIR_OF_FROM(opp)];
	}

	/* 如果修正后的连接形状对应的ISOM值与之前没发生改变，什么都不做 */
	new_link = match_shape(map->era, link_shape);
	if (new_link == link)
		return 0;

	DAssert(new_link);

	/* 产生了新的ISOM值则以其更新画刷菱形 */
	for_each_from (from)
		update_isom(map, new_link, from, &link_pos);

	return new_link;
}

static BOOL update_isom(ISOM_MAP *map, WORD isom, INT from, CONST POINT *pos)
{
	INT opp;
	POINT corner;
	LISOMPTR data;
	LISOMCOORD *coord[DIR_NUM];

	DAssert(map && pos && DBetween(from, 0, FROM_NUM));

	/* 计算指定方向上的角落菱形的坐标 */
	calc_corner_pos(from, pos, &corner);

	/* 设置TILE菱形位置 */
	if (!set_tile_pos(map, &corner))
		return FALSE;

	data = LOC_MAP_POS(map->isom, &corner, &map->size);
	coord[LEFT] = &data->left;
	coord[TOP] = &data->top;
	coord[RIGHT] = &data->right;
	coord[BOTTOM] = &data->bottom;

	/* 标志TILE菱形为脏 */
	SET_DIRTY(map->dirty, &corner, &map->size);

	/* 对画刷来说也许是左上角，但对ISOM菱形来说确是左上角菱形的右下角，因此在此先求出from相对的方向	*/
	opp = OPPOSITE(from);

	/*
		填充对应的ISOM值，对应关系如下：
			from			opp				dir1	dir2
			LEFT_TOP		RIGHT_BOTTOM	RIGHT	BOTTOM
			TOP_RIGHT		BOTTOM_LEFT		BOTTOM	LEFT
			RIGHT_BOTTOM	LEFT_TOP		LEFT	TOP
			BOTTOM_LEFT		TOP_RIGHT		TOP		RIGHT
	*/
	coord[DIR1_OF_FROM(opp)]->isom = isom;
	coord[DIR2_OF_FROM(opp)]->isom = isom;

	return TRUE;
}

static BOOL set_tile_pos(ISOM_MAP *map, CONST POINT *pos)
{
	LISOMPTR data;
	CONST WORD *tile_pos;

	DAssert(map && pos);

	/* 出界检查 */
	if (!check_pos(map, pos))
		return FALSE;

	/* 根据奇偶性设定位置值 */
	tile_pos = ((pos->x + pos->y) & 0x01) ? TILE_POS_EVEN : TILE_POS_ODD;

	data = LOC_MAP_POS(map->isom, pos, &map->size);
	data->left.pos = tile_pos[LEFT];
	data->top.pos = tile_pos[TOP];
	data->right.pos = tile_pos[RIGHT];
	data->bottom.pos = tile_pos[BOTTOM];

	return TRUE;
}

static BOOL get_isom_shape(INT era, WORD isom, INT *low, INT *high, INT *shape_info)
{
	INT center, edge, shape;
	struct ERA_INFO *info;
	CONST struct ERA_PARAM *param;

	DAssert(DBetween(era, 0, L_ERA_NUM) && low && high && shape_info);
	DAssert(g_InitFlag && g_EraInfo[era].init_flag);

	*low = -1;
	*high = -1;
	DMemSet(shape_info, -1, DIR_NUM * sizeof(INT));

	center = isom_to_center(era, isom);
	if (center >= 0) {
		*low = center;
		*high = center;
		shape_info[LEFT] = center;
		shape_info[TOP] = center;
		shape_info[RIGHT] = center;
		shape_info[BOTTOM] = center;
		return TRUE;
	}

	if (!isom_to_edge_shape(era, isom, &edge, &shape))
		return FALSE;

	param = &PARAM_TABLE[era];
	info = &g_EraInfo[era];

	/* 注意，连接处理时需要的是未经过上下关系反转处理的高低层地形，因此从PARAM_TABLE直接取 */
	*low = isom_to_center(era, param->edge[edge].low);
	*high = isom_to_center(era, param->edge[edge].high);

	DMemCpy(shape_info, info->edge[edge].shape[shape], DIR_NUM * sizeof(INT));
	return TRUE;
}

static WORD match_shape(INT era, CONST INT *shape_info)
{
	INT brush1, brush2, edge, shape;
	CONST struct ERA_INFO *info;

	DAssert(DBetween(era, 0, L_ERA_NUM)  && shape_info && g_EraInfo[era].init_flag);

	/* 统计形状数据，最多只允许出现两种地形 */
	brush1 = brush2 = shape_info[LEFT];
	if (shape_info[TOP] != brush1)
		brush2 = shape_info[TOP];
	if (shape_info[RIGHT] != brush1 && shape_info[RIGHT] != brush2) {
		if (brush1 != brush2)
			return 0;
		brush2 = shape_info[RIGHT];
	}
	if (shape_info[BOTTOM] != brush1 && shape_info[BOTTOM] != brush2) {
		if (brush1 != brush2)
			return 0;
		brush2 = shape_info[BOTTOM];
	}

	info = &g_EraInfo[era];

	DAssert(DBetween(brush1, 0, info->center_num));
	DAssert(DBetween(brush2, 0, info->center_num));

	/* 如果各个位置上的值都相同，表明该ISOM菱形是中央地形 */
	if (brush1 == brush2)
		return get_center_isom(era, brush1);

	/* 如果两种地形不连通，认为无效 */
	if (info->center[brush1].below == brush2)
		edge = info->center[brush1].below_edge;
	else if (info->center[brush2].below == brush1)
		edge = info->center[brush2].below_edge;
	else
		return 0;

	/* 查表找到形状ID，进而得到ISOM值 */
	for (shape = 0; shape < SHAPE_NUM; shape++) {
		if (!DMemCmp(shape_info, info->edge[edge].shape[shape], DIR_NUM * sizeof(INT)))
			return get_edge_isom(era, edge, shape);
	}

	/* 不存在的组合？理论上不可能走到这里 */
	DAssert(FALSE);
	return 0;
}

static INT search_brush_link(INT era, INT brush_from, INT brush_to)
{
	INT center, next_brush;
	CONST struct ERA_INFO *info;

	DAssert(DBetween(era, 0, L_ERA_NUM) && g_EraInfo[era].init_flag);
	DAssert(DBetween(brush_from, 0, MAX_CENTER) && DBetween(brush_to, 0, MAX_CENTER));

	if (brush_from == brush_to)
		return brush_from;

	info = &g_EraInfo[era];

	/* 从to一直向下找，找到from为止 */
	center = brush_to;

	while (info->center[center].below >= 0) {
		if (info->center[center].below == brush_from)
			return center;
		center = info->center[center].below;
	}

	/*
		to向下找不到from的话，那要么from可以向下找到to，此时最近画刷应该是from的below；
		要么找不到to，由于是树型结构，to与from的连通路径必然要从下面经过，
		所以也还应该是from的below。
	*/
	next_brush = info->center[brush_from].below;
	DAssert(next_brush >= 0);
	return next_brush;
}

static BOOL make_tile_map(CONST ISOM_MAP *map, CONST POINT *pos, ISOM_TILE *tile)
{
	INT from, center, low, high, edge, shape, temp;
	WORD isom;
	LISOMCPTR data;
	struct TILE_MAP tile_map;
	CONST struct ERA_PARAM *param;
	CONST struct ERA_INFO *info;

	DAssert(map && pos && tile);

	/* 只处理偶菱形（画刷中心所在菱形） */
	if (pos->x % 2 ^ pos->y % 2)
		return FALSE;

	DVarClr(tile_map);

	param = &PARAM_TABLE[map->era];

	/*
		获得该位置画刷菱形的ISOM值。
		由于中心所在菱形（右下菱形）一定是存在的，这里总是从右下菱形取值
	*/
	data = LOC_MAP_POS(map->isom, pos, &map->size);
	isom = data->left.isom;
	DAssert(isom == data->top.isom);

	center = isom_to_center(map->era, isom);

	tile_map.proj[LEFT_SIDE] = V_ABUT_NONE;
	tile_map.proj[RIGHT_SIDE] = V_ABUT_NONE;

	/* 是画刷地型还是边缘地形？ */
	if (center >= 0) {

		/* 画刷地形直接填充画刷对应的TILE边界ID */
		for_each_from (from) {
			tile_map.type[from] = param->center[center].type;
			tile_map.x_abut[from] = param->center[center].abut;
			tile_map.y_abut[from] = param->center[center].abut;
			tile_map.z_abut[from] = V_ABUT_NONE;
		}

	} else {

		if (!isom_to_edge_shape(map->era, isom, &edge, &shape))
			return FALSE;

		info = &g_EraInfo[map->era];
		DAssert(info->init_flag);

		low = info->edge[edge].low;
		high = info->edge[edge].high;

		for_each_from (from) {

			temp = SHAPE_TABLE[shape].type[from];
			tile_map.type[from] = map_edge_tile_type(map->era, low, high, edge, temp);

			temp = SHAPE_TABLE[shape].x_abut[from];
			tile_map.x_abut[from] = map_edge_hor_abuttal(map->era, low, high, edge, temp, tile_map.proj);

			temp = SHAPE_TABLE[shape].y_abut[SIDE_OF_FROM(from)];
			tile_map.y_abut[from] = map_edge_hor_abuttal(map->era, low, high, edge, temp, tile_map.proj);

			temp = SHAPE_TABLE[shape].y_abut[SIDE_OF_FROM(from)];
			tile_map.z_abut[from] = map_edge_ver_abuttal(map->era, low, high, edge, temp);
		}
	}

	/* 填写该画刷菱形对应的TILE索引 */
	map_isom_tile(map, &tile_map, pos, tile);

	return TRUE;
}

static VOID adjust_dirty_map(CONST ISOM_MAP *map, CONST POINT *pos, CONST ISOM_TILE *isom)
{
	POINT coord;

	DAssert(map && pos && isom);

	if (!GET_DIRTY(map->dirty, pos, &map->size))
		return;

	coord = *pos;
	while (LOC_MAP_POS(isom, &coord, &map->size)->up_abut && --coord.y >= 0)
		SET_DIRTY(map->dirty, &coord, &map->size);

	coord = *pos;
	while (LOC_MAP_POS(isom, &coord, &map->size)->down_abut && ++coord.y < CALC_ISOM_LINE(map->size.cy))
		SET_DIRTY(map->dirty, &coord, &map->size);
}

static VOID update_tile(CONST ISOM_MAP *map, CONST POINT *pos, CONST ISOM_TILE *isom)
{
	WORD group, mega;
	LTILEPTR tile;
	CONST ISOM_DICT *dict;

	DAssert(map && isom && pos);
	DAssert(DBetween(pos->x, 0, CALC_ISOM_ROW(map->size.cx)) && DBetween(pos->y, 0, CALC_ISOM_LINE(map->size.cy)));

	/* 出界则不处理 */
	if (pos->x * 2 >= map->size.cx || pos->y >= map->size.cy)
		return;

	/* 仅处理脏菱形 */
	if (!GET_DIRTY(map->dirty, pos, &map->size))
		return;

	isom = LOC_MAP_POS(isom, pos, &map->size);
	tile = map->tile + pos->x * 2 + map->size.cx * pos->y;

	/* 查找对应的TILE编组序号 */
	dict = lookup_tile(map->era, isom);

	if (!dict || !dict->group_no) {
		DAssert(FALSE);
//		DMemClr(tile, 2 * sizeof(LISOMTILE));
		return;
	}

	/* 随机生成MegaTile序号 */
	mega = gen_mega_tile_index(map->era, map->size.cx, pos->y, dict, isom, tile);

	group = dict->group_no;

	/* 一次填充相邻的一对奇偶菱形 */
	tile->mega_index = mega;
	tile->group_no = group;

	group++;
	tile++;

	tile->mega_index = mega;
	tile->group_no = group;
}

static VOID map_isom_tile(CONST ISOM_MAP *map, CONST struct TILE_MAP *tile_map, CONST POINT *pos, ISOM_TILE *tile)
{
	INT from, opp, max_center_type;
	POINT corner;
	WORD *h_abut;
	WORD *v_abut;
	ISOM_TILE *isom;

	DAssert(map && tile_map && pos && tile);
	DAssert(check_pos(map, pos));

	max_center_type = g_EraInfo[map->era].max_center_type;

	for_each_from (from) {

		calc_corner_pos(from, pos, &corner);

		/* 地图出界检查 */
		if (!check_pos(map, &corner))
			continue;

		isom = LOC_MAP_POS(tile, &corner, &map->size);

		/* 源地形如果是边缘地形也一定会覆盖目标，目标地形如果是中央地形也一定被源地形覆盖 */
		if (tile_map->type[from] > max_center_type || isom->type <= max_center_type)
			isom->type = tile_map->type[from];

		/*
			对画刷来说也许是左上角，但对ISOM菱形来说确是左上角菱形的右下角，
			因此在此先求出from相对的方向
		*/
		opp = OPPOSITE(from);

		h_abut = (WORD *)&isom->left_abut;

		/* 填充对应的TILE边界ID */
		h_abut[X_DIR_OF_FROM(opp)] = tile_map->x_abut[from];
		h_abut[Y_DIR_OF_FROM(opp)] = tile_map->y_abut[from];

		v_abut = (WORD *)&isom->up_abut;

		/*
			此处的对应关系为：
				from			v_abut	bool
				LEFT_TOP		DOWN	TRUE
				TOP_RIGHT		DOWN	TRUE
				RIGHT_BOTTOM	UP		FALSE
				BOTTOM_LEFT		UP		FALSE
		*/
		if (!v_abut[IS_FROM_DIR(from, TOP)])
			v_abut[IS_FROM_DIR(from, TOP)] = tile_map->z_abut[from];
	}

	/* ISOM菱形左下方向邻接角落菱形投影处理 */
	project_abuttal(map, tile, BOTTOM_LEFT, tile_map->proj[LEFT_SIDE], pos);

	/* ISOM菱形右下方向邻接角落菱形投影处理 */
	project_abuttal(map, tile, RIGHT_BOTTOM, tile_map->proj[RIGHT_SIDE], pos);
}

static CONST ISOM_DICT *lookup_tile(INT era, CONST ISOM_TILE *tile)
{
	UINT i;
	CONST ISOM_DICT *dict;

	DAssert(DBetween(era, 0, L_ERA_NUM) && tile && g_InitFlag);
	DAssert(g_EraInfo[era].init_flag);
	DAssert(g_TileDictNum[era] && g_TileDict[era]);

	dict = g_TileDict[era];

	for (i = 0; i < g_TileDictNum[era]; i++, dict++) {
		if (!DMemCmp(tile, &dict->tile, sizeof(ISOM_TILE)))
			return dict;
	}

	return NULL;
}

static INT gen_mega_tile_index(INT era, INT map_cx, INT y, CONST ISOM_DICT *dict, CONST ISOM_TILE *isom, LTILECPTR tile)
{
	INT i, often_num, seldom_num;
	INT often_list[16];
	INT seldom_list[16];

	DAssert(DBetween(era, 0, L_ERA_NUM));
	DAssert(map_cx && y >= 0 && dict && isom && tile);

	DAssert((isom->up_abut == isom->up_abut) || !isom->up_abut || !isom->down_abut);

	/* 第一行或者上层邻接关系ID为0则必然重新随机 */
	if (!y || !isom->up_abut) {

		/* 没有可用的MegaTile（比如Space），返回零 */
		if (!dict->mega_mask)
			return 0;

		i = 0;

		/* 生成常见MegaTile列表，并统计数目 */
		for (often_num = 0; i < 16; i++) {
			if (!(dict->mega_mask & (1 << i)))
				break;
			often_list[often_num++] = i;
		}

		/* 生成罕见MegaTile列表，并统计数目 */
		for (i++, seldom_num = 0; i < 16; i++) {
			if (!(dict->mega_mask & (1 << i)))
				break;
			seldom_list[seldom_num++] = i;
		}

#ifndef NDEBUG
		for (i++; i < 16; i++)
			DAssert(!(dict->mega_mask & (1 << i)));
#endif
		DAssert(often_num);

		/* 随机一个有效的MegaTile索引 */
		if (!seldom_num || DRandom() % 16)
			return often_list[DRandom() % often_num];
		else
			return seldom_list[DRandom() % seldom_num];
	}

	/* 否则与上一行同一列的MegaTile索引保持一致 */
	tile -= map_cx;
	return tile->mega_index;
}

static WORD map_edge_tile_type(INT era, INT low, INT high, INT edge, INT temp)
{
	CONST struct ERA_PARAM *param;
	CONST struct ERA_INFO *info;

	DAssert(DBetween(era, 0, L_ERA_NUM));

	param = &PARAM_TABLE[era];
	info = &g_EraInfo[era];

	switch (temp & MAP_FLAG_MASK) {
	case CLIFF_FLAG:
		temp = info->edge[edge].cliff ? AS_EDGE : (temp & ~MAP_FLAG_MASK);
		break;
	case DIFFUSE_FLAG:
		temp = (param->center[high].style == S_DIFFUSE) ? (temp & ~MAP_FLAG_MASK) : AS_EDGE;
		break;
	}

	/* 边缘地形则根据模板构建TILE边界映射信息 */
	switch (temp) {
	case AS_LOW:
		return param->center[low].type;
	case AS_HIGH:
		return param->center[high].type;
	default:
		return param->edge[edge].type;
	}
}

static WORD map_edge_hor_abuttal(INT era, INT low, INT high, INT edge, INT temp, WORD *proj)
{
	CONST struct ERA_PARAM *param;
	CONST struct ERA_INFO *info;

	DAssert(DBetween(era, 0, L_ERA_NUM) && proj);

	param = &PARAM_TABLE[era];
	info = &g_EraInfo[era];

	switch (temp & MAP_FLAG_MASK) {
	case CENTER_FLAG:
		return param->center[low].abut;
	case DIFFUSE_FLAG:
		if (param->center[high].style == S_DIFFUSE)
			return param->center[high].abut;
		return M_SIDE(temp & ~MAP_FLAG_MASK);
	case CLIFF_FLAG:
		DAssert(DBetween(temp & ~MAP_FLAG_MASK, 0, CLIFF_NUM));
		if (info->edge[edge].cliff) {
			if (param->center[high].style != S_TILE && (temp & ~CLIFF_FLAG) == 2)
				proj[RIGHT_SIDE] = V_ABUT_LEFT;
			else if (param->center[high].style != S_TILE && (temp & ~CLIFF_FLAG) == 3)
				proj[LEFT_SIDE] = V_ABUT_RIGHT;
			return param->edge[edge].cliff[temp & ~MAP_FLAG_MASK];
		}
		return param->center[low].abut;
	case SIDE_FLAG:
		return temp;
	}

	DAssert(FALSE);
	return 0;
}

static WORD map_edge_ver_abuttal(INT era, INT low, INT high, INT edge, INT temp)
{
	CONST struct ERA_PARAM *param;
	CONST struct ERA_INFO *info;

	DAssert(DBetween(era, 0, L_ERA_NUM));

	param = &PARAM_TABLE[era];
	info = &g_EraInfo[era];

	switch (temp & MAP_FLAG_MASK) {
	case CENTER_FLAG:
		temp = 0;
		break;
	case DIFFUSE_FLAG:
		if (param->center[high].style == S_DIFFUSE)
			temp = 0;
		else
			temp &= ~DIFFUSE_FLAG;
		break;
	case CLIFF_FLAG:
		if (info->edge[edge].cliff)
			return V_ABUT_CENTER;
		temp = 0;
		break;
	case SIDE_FLAG:
		temp &= ~SIDE_FLAG;
		break;
	default:
		DAssert(FALSE);
		temp = 0;
		break;
	}

	if (!temp)
		return V_ABUT_NONE;

	if (temp <= 4) {
		if ((param->center[high].style == S_STEP || param->center[high].style == S_ROOF) && temp % 2)
			return V_ABUT_NONE;
		return V_ABUT_CENTER;
	}

	if (param->center[high].style == S_ROOF && temp == 7)
		return V_ABUT_LEFT;

	if (param->center[high].style == S_ROOF && temp == 8)
		return V_ABUT_RIGHT;

	if (temp % 2)
		return (info->edge[edge].cliff || param->center[high].style == S_TILE) ? V_ABUT_NONE : V_ABUT_LEFT;

	return (info->edge[edge].cliff || param->center[high].style == S_TILE) ? V_ABUT_NONE : V_ABUT_RIGHT;
}

static VOID project_abuttal(CONST ISOM_MAP *map, ISOM_TILE *tile, INT from, WORD proj, CONST POINT *pos)
{
	POINT corner;
	ISOM_TILE *isom;

	calc_corner_pos(from, pos, &corner);

	/* 地图出界检查 */
	if (!check_pos(map, &corner))
		return;

	isom = LOC_MAP_POS(tile, &corner, &map->size);
	isom->down_abut = proj;

	corner.y++;

	/* 地图出界检查 */
	if (!check_pos(map, &corner))
		return;

	/* 填充对应的边缘模式（上方） */
	isom = LOC_MAP_POS(tile, &corner, &map->size);
	isom->up_abut = proj;
}

static BOOL init_pos_quene(CONST POINT *pos)
{
	struct POS_QUENE *node;

	DAssert(pos && g_InitFlag);

	node = DAlloc(sizeof(struct POS_QUENE));
	if (!node)
		return FALSE;

	node->pos = *pos;
	node->next = NULL;

	DVarClr(g_PosQueneHead.pos);
	g_PosQueneHead.next = node;
	g_PosQueneTail = node;
	return TRUE;
}

static VOID exit_pos_quene(VOID)
{
	struct POS_QUENE *node, *next;

	DAssert(g_InitFlag);

	node = g_PosQueneHead.next;
	while (node) {
		next = node->next;
		DFree(node);
		node = next;
	}
}

static BOOL push_pos_quene(CONST POINT *pos)
{
	struct POS_QUENE *node;

	DAssert(g_PosQueneTail && pos);

	node = DAlloc(sizeof(struct POS_QUENE));
	if (!node)
		return FALSE;

	node->pos = *pos;
	node->next = NULL;

	g_PosQueneTail->next = node;
	g_PosQueneTail = node;
	return TRUE;
}

static BOOL pop_pos_quene(VOID)
{
	struct POS_QUENE *node;

	DAssert(g_PosQueneTail);

	node = g_PosQueneHead.next;
	if (!node)
		return FALSE;

	if (g_PosQueneTail == node)
		g_PosQueneTail = &g_PosQueneHead;

	g_PosQueneHead.next = node->next;
	DFree(node);
	return TRUE;
}

static CONST POINT *peek_pos_quene(VOID)
{
	DAssert(g_PosQueneTail);

	if (!g_PosQueneHead.next)
		return NULL;

	return &g_PosQueneHead.next->pos;
}

static BOOL is_pos_quene_empty(VOID)
{
	DAssert(g_PosQueneTail);

	if (!g_PosQueneHead.next)
		return TRUE;

	return FALSE;
}

/************************************************************************/
