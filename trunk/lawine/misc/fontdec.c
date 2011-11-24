/************************************************************************/
/* File Name   : fontdec.c                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 7th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : Font decryption API implementation                     */
/************************************************************************/

#include "fontdec.h"
#include "sha.h"
#include "idea.h"

/************************************************************************/

#define KEY_VERIFY_CODE		"sgubon"

/************************************************************************/

/* 加密数据包的尾部结构 */
struct DATA_TAIL {
	DWORD checksum;
	BYTE zero;
	BYTE size;
	WORD unused;
};

/************************************************************************/

static UINT s_RandSeed;
static BOOL s_InitFlag;
static BUFCPTR s_Gid;
static CHAR s_Key[FONT_GID_SIZE];
static struct SHA_CONTEXT s_Sha;
static struct IDEA_KEY s_IdeaKey;

/************************************************************************/

static VOID rand_seed(UINT seed);
static INT random(VOID);
static VOID rand_serial(STRPTR rand_buf, UINT buf_size);
static INT generate_key(VOID);
static INT decrypt_data(STRCPTR key, STRPTR data, UINT buf_size);
static VOID get_decrypt_key(VPTR ekey);
static VOID prepare_decrypt(STRCPTR key);

/************************************************************************/

BOOL init_font_decrypt(BUFCPTR gid, BUFPTR ccd, UINT ccd_size)
{
	UINT count;

	if (s_InitFlag)
		return TRUE;

	if (!gid || !ccd || !ccd_size)
		return FALSE;

	s_Gid = gid;

	/* 生成全局密钥 */
	if (!generate_key()) {
		exit_font_decrypt();
		return FALSE;
	}

	/* 使用该密钥对CCD文件解密 */
	count = decrypt_data(s_Key, (STRPTR)ccd, ccd_size);

	/* 通过对解密得到的明文的前7个字节进行校验来确定密钥的有效性 */
	if (count > ccd_size || strncmp((STRCPTR)ccd, KEY_VERIFY_CODE, 7)) {
		exit_font_decrypt();
		return FALSE;
	}

	s_InitFlag = TRUE;
	return TRUE;
}

VOID exit_font_decrypt(VOID)
{
	DVarClr(s_Key);
	DVarClr(s_Sha);
	DVarClr(s_IdeaKey);
	s_Gid = NULL;
	s_InitFlag = FALSE;
}

BOOL decrypt_font(BUFPTR fnt_src, UINT size)
{
	if (!s_InitFlag || !fnt_src || !size)
		return FALSE;

	// 使用密钥解密字体文件
	if (!decrypt_data(s_Key, (STRPTR)fnt_src, size))
		return FALSE;

	return TRUE;
}

/************************************************************************/

/* rand_seed/random函数的实现等完全同于VC的CRT库函数srand与rand的实现，
   这里提供一个复制的实现只是为了确保你使用VC CRT以外的库也能得到正确结果；当然，
   你在VC下完全可以使用srand/rand来替代这些函数，实际上WIN版的星际争霸本身就是这么做的，
   此外星际争霸在每次用完一个随机种子后都会用srand(time(NULL))来复位CRT的随机种子。 */

VOID rand_seed(UINT seed)
{
	s_RandSeed = seed;
}

INT random(VOID)
{
	s_RandSeed = s_RandSeed * 214013 + 2531011;
	return (s_RandSeed >> 16) & 0x7fff;
}

VOID rand_serial(STRPTR rand_buf, UINT buf_size)
{
	UINT i;

	rand_seed(0x150b);

	for (i = 0; i < buf_size;) {
		CHAR ch = random();
		if (ch)
			rand_buf[i++] = ch;
	}

	rand_buf[buf_size - 1] = '\0';
}

INT generate_key(VOID)
{
	INT i;
	UINT size, count;
	CHAR rand[0x14];

	/* 产生特定随机序列（20字节）：
	   { 0xde, 0x88, 0x2a, 0x14, 0xdb, 0x23, 0xe3, 0x8f, 0xb3, 0xfb, 
	     0x7b, 0xa4, 0x22, 0xeb, 0x34, 0x18, 0x22, 0x15, 0x7a, 0x00, } */
	rand_serial(rand, sizeof(rand));

	/* GID文件内容为被上述序列加密后了的真正密钥 */
	size = DMin(sizeof(s_Key), FONT_GID_SIZE);
	DMemCpy(s_Key, s_Gid, size);

	/* 用随机序列作为密钥解密GID，从而得到真正的密钥 */
	count = decrypt_data(rand, s_Key, size);
	if (!count || count >= size)
		return FALSE;

	/* 密钥为0结束字符串 */
	s_Key[count] = '\0';

	/* 密钥有效性验证（字符串长度必须小于128） */
	if (count < 0x80)
		return TRUE;

	for (i = 0; i < 0x80; i++) {
		if (!s_Key[i])
			return TRUE;
	}

	return FALSE;
}

INT decrypt_data(STRCPTR key, STRPTR data, UINT buf_size)
{
	INT i, j;
	UINT data_size, leave_size;
	STRCPTR degist;
	struct DATA_TAIL *tail;
	CHAR buf[0x40], tmp[0x40];

	if (!key || !data || !buf_size)
		return 0;

	/* 准备好IDEA解密密钥和需要用到的SHA-0摘要 */
	prepare_decrypt(key);

	/* 每个加密封包都有8字节的尾部，所以其大小不可能小于或等于8字节 */
	if (buf_size <= sizeof(struct DATA_TAIL))
		return 0;

	/* 去掉8字节的尾部就是密文本身的大小 */
	data_size = buf_size - sizeof(struct DATA_TAIL);

	/* 加密封包数据部分为64字节一组，所以其总大小必须是64的整倍数 */
	if (data_size % 0x40)
		return 0;

	leave_size = data_size;
	degist = (STRCPTR)&s_Sha;

	/* 64字节为一组执行解密处理 */
	for (i = 0; leave_size; i++, data += 0x40, leave_size -= 0x40) {

		DMemCpy(buf, data, 0x40);

		/* 每8组密文数据中只有首组需要IDEA解密 */
		if (!(i % 8)) {

			/* IDEA每次只能解密8个字节，要分8次才能解完一组 */
			for (j = 0; j < 0x40; j += 8)
				idea_encrypt(tmp + j, buf + j, &s_IdeaKey);

		/* 其余7组不解密直接复制源数据 */
		} else {
			DMemCpy(tmp, buf, 0x40);
		}

		/* 使用SHA-0摘要对数据做还原计算，从而得到明文 */
		for (j = 0; j < 0x40; j++)
			buf[j] = tmp[j] - degist[(0x3f - j) % 0x14];

		/* 从首组起，每处理16组（1024字节）更新一次SHA-0摘要 */
		if (!(i % 0x10))
			sha_update(&s_Sha, buf);

		/* 明文写回 */
		DMemCpy(data, buf, 0x40);
	}

	/* 用加密数据包尾部校验结果 */
	tail = (struct DATA_TAIL *)data;
	if (tail->zero > 0)
		return 0;

	if (tail->checksum != s_Sha.h0)
		return 0;

	/* 根据数据包尾部的信息计算得到明文的实际有效长度（显然该长度不可能大于密文的长度） */
	return tail->size + data_size - 0x40;
}

VOID get_decrypt_key(VPTR ekey)
{
	INT i, j;
	WORD *p, *q;

	p = q = ekey;

	/* 一些复杂的变形处理 */
	for (i = 0, j = 0; i < 0x2c; i++) {
		p[j + 8] = (q[(j + 1) % 8] << 9) | (q[(j + 2) % 8] >> 7);
		if (!(++j % 8))
			q += 8;
	}

	/* 使用IDEA加密密钥计算得到解密密钥 */
	idea_decrypt_key(ekey, &s_IdeaKey);
}

VOID prepare_decrypt(STRCPTR key)
{
	INT i, j;
	STRCPTR digest;
	CHAR kbuf[0x40], ekey[0x70];

	if (!key)
		return;

	/* 密钥串（0结束）长度不超过64字节，过多的部分会被截去
	   当密钥串长不够64字节时，循环地复制串内容直到填满64字节 */
	for (i = 0, j = 0; i < 0x40; i++, j++) {
		if (!key[j])
			j = 0;
		kbuf[i] = key[j];
	}

	/* 计算密钥的SHA-0摘要 */
	sha_init(&s_Sha);
	sha_update(&s_Sha, kbuf);

	digest = (STRCPTR)&s_Sha;

	/* 用特殊循环序列与上述摘要进行异或运算，得到原始IDEA加密密钥 */
	rand_seed(0x4fa7);
	for (i = 0; i < 0x70; i++)
		ekey[i] = random() ^ digest[i % 0x14];

	/* 计算原始IDEA加密密钥后64字节的SHA-0摘要 */
	sha_init(&s_Sha);
	sha_update(&s_Sha, ekey + 0x30);

	/* 从原始IDEA加密密钥获得IDEA加密密钥，进而得到IDEA解密密钥 */
	get_decrypt_key(ekey);
}

/************************************************************************/
