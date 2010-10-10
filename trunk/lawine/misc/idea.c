/************************************************************************/
/* File Name   : idea.c                                                 */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 19th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard's IDEA encryption algorithm API implementation*/
/************************************************************************/

#include "idea.h"

/*
		暴雪对FNT加密使用了IDEA加密算法。但是同SHA-0时的一样，它并没有使用标准的IDEA。
	与标准IDEA相比，存在以下不同之处：

		1. IDEA标准要求解密时密文输入数据与输出的明文数据都为大端字序，
	对于小端机器需要对数据做大端→小端转换。暴雪依然不做该转换。
		2. 暴雪使用基数为16位的IDEA，这么一来密钥长度也就短了一倍。
	由于我手头资料不足，我并不能确定这是否合乎于IDEA标准。但值得注意的是，
	你完全可以使用32位的IDEA计算，同样可以得到正确的结果。但你需要自己做密钥格式转换。
	方法很简单，低16位复制高16位补零。

		由于暴雪的算法并没有像写SHA-0时那样的BUG，所以你可以使用各种安全库来帮你运算。
	你可以先在外部把数据转成大端的再交给IDEA函数解密，最后结果再在外部转回小端。
	但显然这不是个高效的方法，因此我仍然在此提供了完整的暴雪版IDEA解密算法的实现。

	IDEA介绍可见：http://en.wikipedia.org/wiki/International_Data_Encryption_Algorithm
*/

/************************************************************************/

static WORD idea_mul(WORD a, WORD b);
static WORD idea_inverse(WORD n);

/************************************************************************/

VOID idea_decrypt_key(CONST struct IDEA_KEY *ekey, struct IDEA_KEY *dkey)
{
	INT i;
	CONST WORD *p;
	WORD *q;

	p = (CONST WORD *)ekey;

	q = dkey->data[8];
	q[0] = idea_inverse(*p++);
	q[1] = -*p++;			/* 16位情况下的简化，本应是(0x10000 - *p++) & 0xffff */
	q[2] = -*p++;			/* 16位情况下的简化，本应是(0x10000 - *p++) & 0xffff */
	q[3] = idea_inverse(*p++);

	for (i = 7; i >= 0; i--) {
		q = dkey->data[i];
		q[4] = *p++;
		q[5] = *p++;
		q[0] = idea_inverse(*p++);
		q[2] = -*p++;		/* 16位情况下的简化，本应是(0x10000 - *p++) & 0xffff */
		q[1] = -*p++;		/* 16位情况下的简化，本应是(0x10000 - *p++) & 0xffff */
		q[3] = idea_inverse(*p++);
	}

	DSwap(q[1], q[2]);
}

VOID idea_encrypt(VPTR dest, VCPTR src, CONST struct IDEA_KEY *key)
{
	INT i;
	WORD a, b, c, d, t1, t2;
	CONST WORD *p;
	WORD *q;

	p = src;

	/* 该函数一次只处理8个字节，src和dest都应该是8字节的 */
	a = p[0];
	b = p[1];
	c = p[2];
	d = p[3];

	p = (CONST WORD *)key;

	for (i = 0; i < 8; i++) {

		a = idea_mul(a, *p++);
		b += *p++;
		c += *p++;
		d = idea_mul(d, *p++);

		t1 = idea_mul(c ^ a, *p++);
		t2 = b;
		b = idea_mul((d ^ b) + t1, *p++);
		t1 += b;

		a ^= b;
		b ^= c;
		c = t1 ^ t2;
		d ^= t1;
	}

	q = dest;

	q[0] = idea_mul(a, *p++);
	q[1] = c + *p++;
	q[2] = b + *p++;
	q[3] = idea_mul(d, *p++);
}

/************************************************************************/

WORD idea_inverse(WORD n)
{
#if 0
	UINT mod1, mod2;
	INT div1, div2;

	if (n <= 1)
		return n;

	div1 = 0x10001 / n;
	mod1 = 0x10001 % n;

	if (mod1 != 1) {

		div2 = n / mod1 * div1 + 1;
		mod2 = n % mod1;

		while (mod2 != 1) {

			div1 += mod1 / mod2 * div2;
			mod1 = mod1 % mod2;

			if (mod1 != 1) {
				div2 += mod2 / mod1 * div1;
				mod2 = mod2 % mod1;
			} else {
				return (WORD)(1 - div1);
			}
		}
		return (WORD)div2;
	}

	return (WORD)(1 - div1);
#else
	INT x, y, d, m, q, t;

	if (!n)
		return 0;

	d = 0x10001;
	x = 1;
	y = 0;

	do {

		m = d % n;

		if (!m) {

			if (x >= 0)
				return (WORD)x;

			/* 16位情况下的简化，本应是0x10001 + x */
			return (WORD)(1 + x);
		}

		q = (d - m) / n;
		d = n;
		n = m;
		t = x;
		x = y - q * t;
		y = t;

	} while (m);

	return (WORD)x;
#endif
}

WORD idea_mul(WORD a, WORD b)
{
	UINT c;

	if (!a)
		/* 16位情况下的简化，本应是0x10001 - b */
		return 1 - b;

	if (!b)
		/* 16位情况下的简化，本应是0x10001 - a */
		return 1 - a;

	c = a * b;
	a = DLoWord(c);
	b = DHiWord(c);

	if (a >= b)
		return a - b;

	/* 16位情况下的简化，本应是0x10001 + a - b */
	return 1 + a - b;
}

/************************************************************************/
