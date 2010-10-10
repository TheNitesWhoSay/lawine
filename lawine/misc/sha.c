/************************************************************************/
/* File Name   : sha.c                                                  */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : March 19th, 2010                                       */
/* Module      : Lawine library                                         */
/* Descript    : Blizzard's SHA-0 digest algorithm API implementation   */
/************************************************************************/

#include "sha.h"

/*
		暴雪对FNT文件加密采用的摘要算法是SHA-0，该算法由于存在安全问题，
	发布不久就被收回，现已被SHA-1替代。据说两者的差别仅仅在一个循环左移运算上
	（具体可见代码中的注释），但由于文档不足，并不能百分之百确定。

		另外必须要说明的是，暴雪使用的SHA-0算法并非标准SHA算法，有三点差异：

		1. SHA-0标准要求整个摘要计算全部以大端字序形式进行，对于IBM-PC这样的小端机器，
	必须做小端→大端转换，但是暴雪并没有做该转换，其计算全部以小端形式进行。
		2. SHA-0标准要求对被计算数据的末尾做1个补充位，暴雪并没有做这一步。
		3. 暴雪在计算循环位移时运算数被当作有符号数进行，这是一个明显的BUG（具体见下面的注释）。

		这些不同之处意味着我们不能直接用现成的安全库，比如OpenSSL来帮我们做解密算法部分，
	必须要自己在这里写特殊的甚至是错误的算法，不然无法得到正确的结果！

	SHA系列算法的介绍可见：http://en.wikipedia.org/wiki/SHA_hash_functions
*/

/************************************************************************/

/*
	32位整数的循环左移，该算法正是暴雪的BUG；由于x是个有符号数，
	右移运算的部分可能会导致高位补1，因此该函数并没有达到SHA标准算法所要求的循环左移的效果，
	大概是由于暴雪员工对自己写的代码没有很好的测试吧。
	糟糕的是我们在这里并不能帮暴雪修正错误，因为他们就是用错的算法加的密，
	只能用错的办法去解密。
*/
#define sha_rol(x, n)	(((x) << (n)) | (x >> (32 - (n))))

/************************************************************************/

VOID sha_init(struct SHA_CONTEXT *sha)
{
	sha->h0 = 0x67452301;
	sha->h1 = 0xefcdab89;
	sha->h2 = 0x98badcfe;
	sha->h3 = 0x10325476;
	sha->h4 = 0xc3d2e1f0;
}

VOID sha_update(struct SHA_CONTEXT *sha, VCPTR buf)
{
	INT i;
	INT a, b, c, d, e, f, tmp;	/* 注意是INT而不是DWORD，暴雪的BUG根源 */
	DWORD w[0x50];

	/*
		暴雪的SHA-0算法仅处理输入数据大小为64字节的情况，
		而它的应用也仅限于对64的整倍数的数据进行操作的情况。
	*/
	DMemCpy(w, buf, sizeof(DWORD) * 0x40);

	for (i = 0x10; i < 0x50; i++)
		/* 已知SHA-1与SHA-0唯一的不同就是下面这个式子，SHA-1需要在这个计算结果上再做1位的循环左移。 */
		w[i] = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];

	a = sha->h0;
	b = sha->h1;
	c = sha->h2;
	d = sha->h3;
	e = sha->h4;

	for (i = 0; i < 0x50; i++) {

		switch (i / 0x14) {
		case 0:
			f = ((b & c) | (~b & d)) + 0x5a827999;
			break;
		case 1:
			f = (b ^ c ^ d) + 0x6ed9eba1;
			break;
		case 2:
			f = ((b & c) | (b & d) | (c & d)) + 0x8f1bbcdc;
			break;
		case 3:
			f = (b ^ c ^ d) + 0xca62c1d6;
			break;
		}

		tmp = sha_rol(a, 5) + f + e + w[i];
		e = d;
		d = c;
		c = sha_rol(b, 30);
		b = a;
		a = tmp;
	}

	sha->h0 += a;
	sha->h1 += b;
	sha->h2 += c;
	sha->h3 += d;
	sha->h4 += e;
}

/************************************************************************/
