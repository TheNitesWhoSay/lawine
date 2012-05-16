/************************************************************************/
/* File Name   : adpcm.c                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 5th, 2012                                          */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard ADPCM compression API implementation          */
/************************************************************************/

#include "adpcm.h"

/*
		暴雪对WAVE格式的数据采用了ADPCM压缩以减小存储空间。该算法可能是暴雪自创的，
	我并没有找到该压缩算法所对应的标准。它具有以下特点：

		1. 该算法与WAVE文件结构无关，它只能压缩WAVE文件中data块里的数据。
		2. 仅支持单声道和立体声的16位PCM数据。
		3. 它使用和IMA-ADPCM完全相同的阶码表，但它的算法却与IMA-ADPCM有所区别。
		4. 除去头部，在产生命令码的情况下压缩/解压比略大于1:2，否则等于1:2。
		5. 压缩支持一个类型参数，该参数是一个位移偏移量，但是相关原理不明。
	但通常该值大于3时音质都可以接受。

		为了避免压缩破坏WAVE文件自身结构，暴雪通常是将WAVE文件的头4096字节用无损压缩，
	而其之后的数据使用ADPCM压缩，同时保证这些WAVE文件4096字节起的数据都是纯粹的data块数据。
	通常，这些WAVE仅保留了RIFF头、WAVE标志、fmt块头部、fmt块数据、data块头部和data块数据，
	而RIFF头、WAVE标志、fmt块头部、fmt块数据和data块头部加起来一共仅有44字节，
	所以能够保证所有文件结构相关的数据都能够集中在WAVE文件的头4096字节里。
	加之WAVE结构本身是4字节对齐的，所以对于所有的单声道和立体声的16位PCM WAVE文件，
	其4096处开始的波形数据一定是一个帧数据的起始而不可能从中间被截断。

		压缩编码以字节为单元，根据最高位（第7位）的值被分成两种：命令和数据。
		其中命令码分为：
			1. 跳过：可忽略。不产出样本值。
			2. 重复：重复上一次的样本值。产出样本值。
			3. 增阶：跳增8个阶。不产出样本值。
			4. 降阶：跳降8个阶。不产出样本值。
		而数据字节的第6位为符号位，标志着差值的增减方向，0-5位为数据位。
		需要注意暴雪的压缩代码并不会产生跳过和降阶命令，但是解压时却能识别它们。
		编码算法会根据可用输出缓冲的大小来自动判断是否产生命令码，以降低失真率。

		除了上述从星际争霸正式版开始出现的类IMA-ADPCM算法以外，还存在一种ADPCM压缩算法，
	它似乎仅存在于星际争霸Beta版中，之后再没有被使用过，并且之后的Storm库也不再支持该算法。
	此算法似乎也是暴雪所自创的，相对于正式版的算法其特点如下：

		1. 算法完全不同于正式版算法，与IMA-ADPCM无相似性。
		2. 除去头部，其压缩比精确等于1:2。
		3. 同正式版，该算法也支持一个作偏移量用的的类型参数，但是仅有当该参数为2、3、4、
	6时该算法才能正常工作，否则会宕机。

		同正式版算法，不能使用该算法对WAVE文件结构相关的部分进行压缩。

	IMA-ADPCM介绍可见：http://wiki.multimedia.cx/index.php?title=IMA_ADPCM
 */

/************************************************************************/

#define SAMPLE_MIN			-32768				/* 样本最小值 */
#define SAMPLE_MAX			32767				/* 样本最大值 */
#define SAMPLE_SIZE			2U					/* 样本大小（16位） */

#define INDEX_MIN			0					/* 最小有效阶索引 */
#define INDEX_MAX			88					/* 最大有效阶索引 */
#define INDEX_INIT			44					/* 压缩解压处理初始阶索引 */
#define INDEX_STEP			8					/* 跳阶数 */

#define MASK_CMD			0x80				/* 命令码标志位 */
#define MASK_SIGN			0x40				/* 符号标志位 */
#define CMD_IGN				(MASK_CMD | 0x02)	/* 跳过命令码 */
#define CMD_REP				(MASK_CMD | 0x00)	/* 重复命令码 */
#define CMD_STEP			(MASK_CMD | 0x01)	/* 增阶命令码 */

/* Beta版用常量 */
#define BETA_SIGN			0x01				/* 符号标志位 */
#define BETA_DIFF_MAX		0x20000				/* 差值上限 */

/************************************************************************/

/* 暴雪ADPCM算法所使用的独特的索引表 */
static CONST INT s_IndexTab[] = {
	-1, 0, -1, 4, -1, 2, -1, 6, -1, 1, -1, 5, -1, 3, -1, 7,
	-1, 1, -1, 5, -1, 3, -1, 7, -1, 2, -1, 4, -1, 6, -1, 8,
};

/* IMA-ADPCM阶码表，共89项 */
static CONST WORD s_StepTab[] = {
	0x0007, 0x0008, 0x0009, 0x000a, 0x0000b, 0x000c, 0x000d, 0x000e,
	0x0010, 0x0011, 0x0013, 0x0015, 0x00017, 0x0019, 0x001c, 0x001f,
	0x0022, 0x0025, 0x0029, 0x002d, 0x00032, 0x0037, 0x003c, 0x0042,
	0x0049, 0x0050, 0x0058, 0x0061, 0x0006b, 0x0076, 0x0082, 0x008f,
	0x009d, 0x00ad, 0x00be, 0x00d1, 0x000e6, 0x00fd, 0x0117, 0x0133,
	0x0151, 0x0173, 0x0198, 0x01c1, 0x001ee, 0x0220, 0x0256, 0x0292,
	0x02d4, 0x031c, 0x036c, 0x03c3, 0x00424, 0x048e, 0x0502, 0x0583,
	0x0610, 0x06ab, 0x0756, 0x0812, 0x008e0, 0x09c3, 0x0abd, 0x0bd0,
	0x0cff, 0x0e4c, 0x0fba, 0x114c, 0x01307, 0x14ee, 0x1706, 0x1954,
	0x1bdc, 0x1ea5, 0x21b6, 0x2515, 0x028ca, 0x2cdf, 0x315b, 0x364b,
	0x3bb9, 0x41b2, 0x4844, 0x4f7e, 0x05771, 0x602f, 0x69ce, 0x7462,
	0x7fff,
};

/* Beta版算法用阶码表 */
static CONST INT s_BetaTab2[] = { 51, 102 };
static CONST INT s_BetaTab3[] = { 58, 58, 80, 112 };
static CONST INT s_BetaTab4[] = { 58, 58, 58, 58, 77, 102, 128, 154 };
static CONST INT s_BetaTab6[] = {
	58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
	70, 83, 96, 109, 122, 134, 147, 160, 173, 186, 198, 211, 224, 237, 250, 262,
};

/************************************************************************/

BOOL adpcm_encode(INT type, INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT i, num, bits, ch, sample, raw_diff, step, base, diff;
	UINT size;
	BYTE byte, mask;
	CONST SHORT *rd_ptr, *rd_end_ptr;
	BUFPTR wrt_ptr;
	SHORT *raw_ptr;
	INT index[2], pcm_buf[2];

	/* 参数有效性检查 */
	if (!src || !dest || !dest_size || (channels != ADPCM_MONO && channels != ADPCM_STEREO))
		return FALSE;

	/* 计算输入缓冲所含有的样本个数 */
	num = src_size / SAMPLE_SIZE;

	/* 输入数据至少要有一个完整帧 */
	if (num < channels)
		return FALSE;

	/* 计算压缩结果所需要的最少字节数（2字节的头部+1个原始帧数据+1:2的压缩数据） */
	size = 2 + channels * SAMPLE_SIZE + num;

	/* 输出缓冲不足以放下完整的结果 */
	if (*dest_size < size)
		return FALSE;

	/* 计算出盈余空间的大小以供后面跳阶编码使用 */
	size -= *dest_size;

	rd_ptr = src;
	rd_end_ptr = rd_ptr + num;
	wrt_ptr = dest;

	/* 写入数据头并跳过 */
	*wrt_ptr++ = 0;
	*wrt_ptr++ = type - 1;
	raw_ptr = (SHORT *)wrt_ptr;

	/* 开头先是一帧的无压缩数据 */
	for (ch = 0; ch < channels; ch++, rd_ptr++) {
		*raw_ptr++ = *rd_ptr;
		index[ch] = INDEX_INIT;
		pcm_buf[ch] = *rd_ptr;
	}

	wrt_ptr = (BUFPTR)raw_ptr;

	/* 先根据类型（偏移量）一次性计算出编码位数 */
	bits = type - 1;
	if (bits <= 0 || bits > 6)
		bits = 6;

	/* 主循环，每次向输出缓冲写一个样本的压缩码 */
	for (ch = 0; rd_ptr < rd_end_ptr; ch++) {

		/* 在多个声道间来回切换 */
		if (ch >= channels)
			ch = 0;

		/* 取样本原始数据 */
		sample = *rd_ptr++;

		/* 压缩码初始化 */
		byte = 0x00;

		/* 计算相对于上帧的差值 */
		raw_diff = sample - pcm_buf[ch];

		/* 如果是负数，取绝对值并且将符号位置位 */
		if (raw_diff < 0) {
			raw_diff = -raw_diff;
			byte |= MASK_SIGN;
		}

		step = s_StepTab[index[ch]];

		/* 如果差值相对于当前阶码来说太小，则认为和前一帧的样本值相同 */
		if (raw_diff < (step >> type)) {
			if (index[ch])
				index[ch]--;
			*wrt_ptr++ = CMD_REP;
			continue;
		}
		
		/* 根据具体情况跳阶 */
		while (raw_diff > (step << 1)) {

			/* 阶码已经达到上限或者写缓冲里已经没有足够的空间去写多余的命令字节了，则中止增阶处理直接强行转入ADPCM压缩处理 */
			if (index[ch] >= INDEX_MAX || !size)
				break;

			/* 一次跳增8个阶 */
			index[ch] += INDEX_STEP;
			if (index[ch] > INDEX_MAX)
				index[ch] = INDEX_MAX;

			/* 向缓冲写入跳阶命令码 */
			*wrt_ptr++ = CMD_STEP;

			/* 更新当前阶码 */
			step = s_StepTab[index[ch]];

			/* 由于产生了跳阶码的压缩帧数据大于一个字节，需要减小盈余空间的大小 */
			size--;
		}

		/* 数据压缩编码处理 */
		base = step >> (type - 1);

		/* 注意：下面这个循环的代码的执行效率直接影响到解压速度 */
		for (i = 0, diff = 0, mask = 0x01; i < bits; i++, mask <<= 1, step >>= 1) {
			if (diff + step <= raw_diff) {
				diff += step;
				byte |= mask;
			}
		}

		diff += base;
		sample = pcm_buf[ch];

		/* 通过补回差值还原样本波形，由于ADPCM是有损压缩，该样本值可能与原始值不同 */
		if (byte & MASK_SIGN) {
			sample -= diff;
			if (sample < SAMPLE_MIN)
				sample = SAMPLE_MIN;
		} else {
			sample += diff;
			if (sample > SAMPLE_MAX)
				sample = SAMPLE_MAX;
		}

		/* 写压缩字节码到输出缓冲 */
		*wrt_ptr++ = byte;

		/* 更新该声道各压缩参数 */
		pcm_buf[ch] = sample;
		index[ch] += s_IndexTab[byte & 0x1f];
		if (index[ch] < INDEX_MIN)
			index[ch] = INDEX_MIN;
		else if (index[ch] > INDEX_MAX)
			index[ch] = INDEX_MAX;
	}

	/* 计算输出数据的大小 */
	*dest_size = (UINT)(wrt_ptr - (BUFPTR)dest);

	return TRUE;
}

BOOL adpcm_decode(INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT i, num, ch, type, sample, diff, step;
	BYTE byte, mask;
	BUFCPTR rd_ptr, rd_end_ptr;
	SHORT *wrt_ptr, *wrt_end_ptr;
	CONST SHORT *raw_ptr;
	INT index[2], pcm_buf[2];

	/* 参数有效性检查 */
	if (!src || !dest || !dest_size || (channels != ADPCM_MONO && channels != ADPCM_STEREO))
		return FALSE;

	/* 压缩数据最小4或6字节 */
	if (src_size < 2 + channels * SAMPLE_SIZE)
		return FALSE;

	/* 计算输出缓冲所能容纳的最大样本数 */
	num = *dest_size / SAMPLE_SIZE;

	/* 输出缓冲至少要能存的下一个完整帧 */
	if (num < channels)
		return FALSE;

	rd_ptr = src;
	rd_end_ptr = rd_ptr + src_size;
	wrt_ptr = dest;
	wrt_end_ptr = wrt_ptr + num;

	/* 获得压缩类型并跳过数据头 */
	type = *++rd_ptr;
	raw_ptr = (CONST SHORT *)(++rd_ptr);

	/* 开头先是一帧的无压缩数据 */
	for (ch = 0; ch < channels; ch++, raw_ptr++) {
		*wrt_ptr++ = *raw_ptr;
		index[ch] = INDEX_INIT;
		pcm_buf[ch] = *raw_ptr;
	}

	rd_ptr = (BUFCPTR)raw_ptr;

	/* 主循环，每次处理压缩码的一个字节 */
	for (ch = 0; rd_ptr < rd_end_ptr; ) {

		byte = *rd_ptr++;

		/* 判断是否命令码 */
		if (byte & MASK_CMD) {

			switch (byte) {

			/* 忽略 */
			case CMD_IGN:
				break;

			/* 重复前一次的样本 */
			case CMD_REP:
				if (wrt_ptr >= wrt_end_ptr)
					return FALSE;
				if (index[ch])
					index[ch]--;
				*wrt_ptr++ = pcm_buf[ch];
				break;

			/* 跳增阶 */
			case CMD_STEP:
				index[ch] += INDEX_STEP;
				if (index[ch] > INDEX_MAX)
					index[ch] = INDEX_MAX;
				/* 跳阶后的下一个处理并不需要切换声道 */
				continue;

			/* 跳降阶 */
			default:
				index[ch] -= INDEX_STEP;
				if (index[ch] < INDEX_MIN)
					index[ch] = INDEX_MIN;
				/* 跳阶后的下一个处理并不需要切换声道 */
				continue;
			}

		} else {

			/* 写缓冲不足，失败 */
			if (wrt_ptr >= wrt_end_ptr)
				return FALSE;

			/* 差值还原处理 */
			step = s_StepTab[index[ch]];
			diff = step >> type;

			/* 注意：下面这个循环的代码的执行效率直接影响到解压速度 */
			for (i = 0, mask = 0x01; i < 6; i++, mask <<= 1, step >>= 1) {
				if (byte & mask)
					diff += step;
			}

			sample = pcm_buf[ch];

			/* 通过补回差值还原样本波形，由于ADPCM是有损压缩，该样本值可能与原始值不同 */
			if (byte & MASK_SIGN) {
				sample -= diff;
				if (sample < SAMPLE_MIN)
					sample = SAMPLE_MIN;
			} else {
				sample += diff;
				if (sample > SAMPLE_MAX)
					sample = SAMPLE_MAX;
			}

			/* 写样本数据到输出缓冲 */
			*wrt_ptr++ = sample;

			/* 更新该声道各压缩参数 */
			pcm_buf[ch] = sample;
			index[ch] += s_IndexTab[byte & 0x1f];
			if (index[ch] < INDEX_MIN)
				index[ch] = INDEX_MIN;
			else if (index[ch] > INDEX_MAX)
				index[ch] = INDEX_MAX;
		}

		/* 在多个声道间来回切换 */
		if (++ch >= channels)
			ch = 0;
	}

	/* 计算输出数据的大小 */
	*dest_size = (UINT)(wrt_ptr - (SHORT *)dest) * SAMPLE_SIZE;

	return TRUE;
}

/************************************************************************/

BOOL adpcm_beta_encode(INT type, INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT ch, num, step, diff, sample, base;
	UINT size;
	BYTE byte;
	CONST SHORT *rd_ptr, *rd_end_ptr, *next;
	BUFPTR wrt_ptr;
	CONST INT *step_tab;
	INT pc_pcm[2], last[2], pc_diff[2];

	/* 参数有效性检查 */
	if (!src || !dest || !dest_size || (channels != ADPCM_MONO && channels != ADPCM_STEREO))
		return FALSE;

	/* 根据压缩类型选择对应的阶码表 */
	switch (type) {
	case 2:
		step_tab = s_BetaTab2;
		break;
	case 3:
		step_tab = s_BetaTab3;
		break;
	case 4:
		step_tab = s_BetaTab4;
		break;
	case 6:
		step_tab = s_BetaTab6;
		break;
	default:
		return FALSE;
	}

	/* 计算输入缓冲所含有的样本个数 */
	num = src_size / SAMPLE_SIZE;

	/* 输入数据至少要有两个完整帧 */
	if (num < channels * 2)
		return FALSE;

	/* 计算压缩结果所需要的字节数（1字节的类型值+末尾补正值+初始差值+1个原始帧数据+1:2的压缩数据） */
	size = 1 + channels * (3 + SAMPLE_SIZE) + (num - channels);

	/* 输出缓冲不足以放下完整的结果 */
	if (*dest_size < size)
		return FALSE;

	rd_ptr = src;
	rd_end_ptr = rd_ptr + num;
	wrt_ptr = dest;

	/* 通过压缩类型计算阶长 */
	step = 1 << type;

	/* 第一个字节是压缩类型 */
	*wrt_ptr++ = type;

	/* 先跳过写缓冲的下一个或两个字节，留到最后再填 */
	wrt_ptr += channels;

	/* 定位到第二帧数据的起始处 */
	next = rd_ptr + channels;

	/* 计算初始差值 */
	for (ch = 0; ch < channels; ch++, next++, wrt_ptr += 2) {

		/* 获得第一帧的样本 */
		sample = *rd_ptr++;

		/* 0.9倍处理 */
		sample = (sample * 230 + 128) >> 8;

		/* 计算与下一帧样本的差值的绝对值 */
		if (*next < sample)
			diff = sample - *next;
		else
			diff = *next - sample;

		diff <<= 2;

		if (diff < step)
			diff = step;
		else if (diff > BETA_DIFF_MAX)
			diff = BETA_DIFF_MAX;

		/* 文件中保存的值是样本差值所跨阶的数目 */
		diff >>= type;
		*(WORD *)wrt_ptr = diff;

		/* 作为预测偏差值的初始值保存 */
		pc_diff[ch] = diff << type;
	}

	/* 保存第一帧的原始数据作为预测样本值缓冲的初始值并直接写至输出缓冲 */
	for (rd_ptr = src, ch = 0; ch < channels; ch++, wrt_ptr += 2) {
		sample = *rd_ptr++;
		pc_pcm[ch] = sample;
		*(SHORT *)wrt_ptr = sample;
	}

	/* 主循环，每次处理一个样本并产生一个字节的压缩码 */
	for (ch = 0; rd_ptr < rd_end_ptr; ch++) {

		/* 在多个声道间来回切换 */
		if (ch >= channels)
			ch = 0;

		sample = *rd_ptr++;

		/* 该值将在循环之外被使用到，届时里面存放的是最后一帧的样本值 */
		last[ch] = sample;

		/* 编码初始化 */
		byte = 0x00;
		base = 0;

		/* 0.9倍处理 */
		pc_pcm[ch] = (pc_pcm[ch] * 230 + 128) >> 8;

		/* 计算预测值与实际值的偏差量 */
		if (sample < pc_pcm[ch]) {
			diff = pc_pcm[ch] - sample;
			byte |= BETA_SIGN;
		} else {
			diff = sample - pc_pcm[ch];
		}

		diff <<= type;

		/* 如果预测值与实际值不符，将差异量化编码 */
		if (diff > pc_diff[ch]) {
			base = (diff - (pc_diff[ch] / 2)) / pc_diff[ch];
			if (base > step / 2 - 1)
				base = step / 2 - 1;
			byte |= base << 1;
		}

		/* 将编码结果写入输出缓冲 */
		*wrt_ptr++ = byte;

		/* 计算编码后的样本差值 */
		diff = ((base + 1) * pc_diff[ch] + step / 2) >> type;

		/* 根据差值计算下一帧的预测样本值 */
		if (byte & BETA_SIGN) {
			pc_pcm[ch] -= diff;
			if (pc_pcm[ch] < SAMPLE_MIN)
				pc_pcm[ch] = SAMPLE_MIN;
		} else {
			pc_pcm[ch] += diff;
			if (pc_pcm[ch] > SAMPLE_MAX)
				pc_pcm[ch] = SAMPLE_MAX;
		}

		/* 计算下一帧的预测偏差值 */
		pc_diff[ch] = (step_tab[base] * pc_diff[ch] + 128) >> 6;
		if (pc_diff[ch] < step)
			pc_diff[ch] = step;
		else if (pc_diff[ch] > BETA_DIFF_MAX)
			pc_diff[ch] = BETA_DIFF_MAX;
	}

	/* 计算输出数据的大小 */
	*dest_size = (UINT)(wrt_ptr - (BUFPTR)dest);

	/* 重定位写指针到缓冲头部，并跳过开头的一个字节 */
	wrt_ptr = dest;
	wrt_ptr++;

	/* 补上头部空出的一或两个字节 */
	for (ch = 0; ch < channels; ch++) {

		/* 编码初始化 */
		byte = 0x00;

		/* 注意：如果波形数据是立体声的且输入数据的最后少了一个声道的样本时，两个last内的值并不会对应同一个帧。 */
		/* 为了同星际争霸Beta版相兼容，此逻辑不应被改变 */

		/* 计算差值，该值在解压时作为末尾补正帧的数目使用 */
		if (pc_pcm[ch] < last[ch]) {
			diff = last[ch] - pc_pcm[ch];
			byte |= BETA_SIGN;
		} else {
			diff = pc_pcm[ch] - last[ch];
		}

		/* 限制补正帧数上限 */
		if (diff > 127)
			diff = 127;

		/* 保存至字节的高7位 */
		byte |= diff << 1;

		/* 写一个字节到写缓冲 */
		*wrt_ptr++ = byte;
	}

	return TRUE;
}

BOOL adpcm_beta_decode(INT channels, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT ch, num, type, step, diff, sample, base, rest;
	UINT size;
	BYTE byte, adj[2];
	BUFCPTR rd_ptr, rd_end_ptr;
	SHORT *wrt_ptr;
	CONST INT *step_tab;
	INT pc_pcm[2], pc_diff[2];

	/* 参数有效性检查 */
	if (!src || !dest || !dest_size || (channels != ADPCM_MONO && channels != ADPCM_STEREO))
		return FALSE;

	/* 计算头部字节数（1字节的类型值+末尾补正值+初始差值+1个原始帧数据） */
	size = 1 + channels * (3 + SAMPLE_SIZE);

	/* 压缩数据最小7或11字节 */
	if (src_size < size)
		return FALSE;

	/* 从输入缓冲的大小计算出样本总数 */
	num = src_size - size + channels;

	/* 输出缓冲不足以放下完整的结果 */
	if (*dest_size < num * SAMPLE_SIZE)
		return FALSE;

	rd_ptr = src;
	rd_end_ptr = rd_ptr + src_size;
	wrt_ptr = dest;

	/* 获得压缩类型 */
	type = *rd_ptr++;

	/* 根据压缩类型选择对应的阶码表 */
	switch (type) {
	case 2:
		step_tab = s_BetaTab2;
		break;
	case 3:
		step_tab = s_BetaTab3;
		break;
	case 4:
		step_tab = s_BetaTab4;
		break;
	case 6:
		step_tab = s_BetaTab6;
		break;
	default:
		return FALSE;
	}

	/* 通过压缩类型计算阶长 */
	step = 1 << type;

	/* 保存下来的一或两个字节作为末尾补正值 */
	for (ch = 0; ch < channels; ch++)
		adj[ch] = *rd_ptr++;

	/* 读取预测偏差值的初始值 */
	for (ch = 0; ch < channels; ch++, rd_ptr += 2) {
		diff = *(const WORD *)rd_ptr;
		pc_diff[ch] = diff << type;
	}

	/* 读取第一帧原始样本值作为初始预测样本值，并直接写至输出缓冲 */
	for (ch = 0; ch < channels; ch++, rd_ptr += SAMPLE_SIZE) {
		sample = *(const SHORT *)rd_ptr;
		pc_pcm[ch] = sample;
		*wrt_ptr++ = sample;
	}

	/* 主循环，一次处理一个字节的压缩编码并生成一个对应的样本结果 */
	for (ch = 0; rd_ptr < rd_end_ptr; ch++) {

		/* 在多个声道间来回切换 */
		if (ch >= channels)
			ch = 0;

		byte = *rd_ptr++;

		/* 获取量化后的差异编码 */
		base = byte >> 1;

		/* 0.9倍处理 */
		pc_pcm[ch] = (pc_pcm[ch] * 230 + 128) >> 8;

		/* 根据编码计算样本差值 */
		diff = ((base + 1) * pc_diff[ch] + step / 2) >> type;

		/* 从差值还原样本 */
		if (byte & BETA_SIGN) {
			pc_pcm[ch] -= diff;
			if (pc_pcm[ch] < SAMPLE_MIN)
				pc_pcm[ch] = SAMPLE_MIN;
		} else {
			pc_pcm[ch] += diff;
			if (pc_pcm[ch] > SAMPLE_MAX)
				pc_pcm[ch] = SAMPLE_MAX;
		}

		/* 计算下一帧的预测偏差值 */
		pc_diff[ch] = (step_tab[base] * pc_diff[ch] + 128) >> 6;
		if (pc_diff[ch] < step)
			pc_diff[ch] = step;
		else if (pc_diff[ch] > BETA_DIFF_MAX)
			pc_diff[ch] = BETA_DIFF_MAX;

		sample = pc_pcm[ch];

		/* 计算未处理帧数 */
		rest = (UINT)(rd_end_ptr - rd_ptr) / channels;

		/* 计算补正绝对值 */
		diff = adj[ch] >> 1;

		/* 对最后几帧进行补正（原理不明，大概是为了减小误差） */
		if (rest < diff) {
			if (adj[ch] & BETA_SIGN) {
				sample += diff - rest;
				if (sample > SAMPLE_MAX)
					sample = SAMPLE_MAX;
			} else {
				sample -= diff - rest;
				if (sample < SAMPLE_MIN)
					sample = SAMPLE_MIN;
			}
		}

		/* 写样本值到输出缓冲 */
		*wrt_ptr++ = sample;
	}

	/* 计算输出数据的大小 */
	*dest_size = (UINT)(wrt_ptr - (SHORT *)dest) * SAMPLE_SIZE;

	return TRUE;
}

/************************************************************************/
