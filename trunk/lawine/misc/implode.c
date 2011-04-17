/************************************************************************/
/* File Name   : implode.c                                              */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : May 23rd, 2010                                         */
/* Module      : Lawine library                                         */
/* Descript    : PKWare DCL implode compression API implementation      */
/************************************************************************/

#include "implode.h"

/*
	该源文件改自ShadowFlare的pkimplode.c和pkexplode.c文件。
	如要引用该代码请务必注明原作者为ShadowFlare。
*/

/************************************************************************/

/* Truncate value to a specified number of bits */
#define TRUNCATE_VALUE(v, b)	((v) & ((1 << (b)) - 1))

/************************************************************************/

/* Bit sequences used to represent literal bytes */
static WORD s_ChCode[] = {
	0x0490, 0x0fe0, 0x07e0, 0x0be0, 0x03e0, 0x0de0, 0x05e0, 0x09e0,
	0x01e0, 0x00b8, 0x0062, 0x0ee0, 0x06e0, 0x0022, 0x0ae0, 0x02e0,
	0x0ce0, 0x04e0, 0x08e0, 0x00e0, 0x0f60, 0x0760, 0x0b60, 0x0360,
	0x0d60, 0x0560, 0x1240, 0x0960, 0x0160, 0x0e60, 0x0660, 0x0a60,
	0x000f, 0x0250, 0x0038, 0x0260, 0x0050, 0x0c60, 0x0390, 0x00d8,
	0x0042, 0x0002, 0x0058, 0x01b0, 0x007c, 0x0029, 0x003c, 0x0098,
	0x005c, 0x0009, 0x001c, 0x006c, 0x002c, 0x004c, 0x0018, 0x000c,
	0x0074, 0x00e8, 0x0068, 0x0460, 0x0090, 0x0034, 0x00b0, 0x0710,
	0x0860, 0x0031, 0x0054, 0x0011, 0x0021, 0x0017, 0x0014, 0x00a8,
	0x0028, 0x0001, 0x0310, 0x0130, 0x003e, 0x0064, 0x001e, 0x002e,
	0x0024, 0x0510, 0x000e, 0x0036, 0x0016, 0x0044, 0x0030, 0x00c8,
	0x01d0, 0x00d0, 0x0110, 0x0048, 0x0610, 0x0150, 0x0060, 0x0088,
	0x0fa0, 0x0007, 0x0026, 0x0006, 0x003a, 0x001b, 0x001a, 0x002a,
	0x000a, 0x000b, 0x0210, 0x0004, 0x0013, 0x0032, 0x0003, 0x001d,
	0x0012, 0x0190, 0x000d, 0x0015, 0x0005, 0x0019, 0x0008, 0x0078,
	0x00f0, 0x0070, 0x0290, 0x0410, 0x0010, 0x07a0, 0x0ba0, 0x03a0,
	0x0240, 0x1c40, 0x0c40, 0x1440, 0x0440, 0x1840, 0x0840, 0x1040,
	0x0040, 0x1f80, 0x0f80, 0x1780, 0x0780, 0x1b80, 0x0b80, 0x1380,
	0x0380, 0x1d80, 0x0d80, 0x1580, 0x0580, 0x1980, 0x0980, 0x1180,
	0x0180, 0x1e80, 0x0e80, 0x1680, 0x0680, 0x1a80, 0x0a80, 0x1280,
	0x0280, 0x1c80, 0x0c80, 0x1480, 0x0480, 0x1880, 0x0880, 0x1080,
	0x0080, 0x1f00, 0x0f00, 0x1700, 0x0700, 0x1b00, 0x0b00, 0x1300,
	0x0da0, 0x05a0, 0x09a0, 0x01a0, 0x0ea0, 0x06a0, 0x0aa0, 0x02a0,
	0x0ca0, 0x04a0, 0x08a0, 0x00a0, 0x0f20, 0x0720, 0x0b20, 0x0320,
	0x0d20, 0x0520, 0x0920, 0x0120, 0x0e20, 0x0620, 0x0a20, 0x0220,
	0x0c20, 0x0420, 0x0820, 0x0020, 0x0fc0, 0x07c0, 0x0bc0, 0x03c0,
	0x0dc0, 0x05c0, 0x09c0, 0x01c0, 0x0ec0, 0x06c0, 0x0ac0, 0x02c0,
	0x0cc0, 0x04c0, 0x08c0, 0x00c0, 0x0f40, 0x0740, 0x0b40, 0x0340,
	0x0300, 0x0d40, 0x1d00, 0x0d00, 0x1500, 0x0540, 0x0500, 0x1900,
	0x0900, 0x0940, 0x1100, 0x0100, 0x1e00, 0x0e00, 0x0140, 0x1600,
	0x0600, 0x1a00, 0x0e40, 0x0640, 0x0a40, 0x0a00, 0x1200, 0x0200,
	0x1c00, 0x0c00, 0x1400, 0x0400, 0x1800, 0x0800, 0x1000, 0x0000,
};

/* Lengths of bit sequences used to represent literal bytes */
static BYTE s_ChBits[] = {
	0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x08, 0x07, 0x0c, 0x0c, 0x07, 0x0c, 0x0c,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	0x04, 0x0a, 0x08, 0x0c, 0x0a, 0x0c, 0x0a, 0x08, 0x07, 0x07, 0x08, 0x09, 0x07, 0x06, 0x07, 0x08,
	0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x08, 0x07, 0x07, 0x08, 0x08, 0x0c, 0x0b, 0x07, 0x09, 0x0b,
	0x0c, 0x06, 0x07, 0x06, 0x06, 0x05, 0x07, 0x08, 0x08, 0x06, 0x0b, 0x09, 0x06, 0x07, 0x06, 0x06,
	0x07, 0x0b, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x09, 0x09, 0x0b, 0x08, 0x0b, 0x09, 0x0c, 0x08,
	0x0c, 0x05, 0x06, 0x06, 0x06, 0x05, 0x06, 0x06, 0x06, 0x05, 0x0b, 0x07, 0x05, 0x06, 0x05, 0x05,
	0x06, 0x0a, 0x05, 0x05, 0x05, 0x05, 0x08, 0x07, 0x08, 0x08, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
	0x0d, 0x0c, 0x0d, 0x0d, 0x0d, 0x0c, 0x0d, 0x0d, 0x0d, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0c, 0x0d,
	0x0d, 0x0d, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
};

/* Bit sequences used to represent the base values of the copy length */
static BYTE s_LenCode[] = {
	0x05, 0x03, 0x01, 0x06, 0x0a, 0x02, 0x0c, 0x14, 0x04, 0x18, 0x08, 0x30, 0x10, 0x20, 0x40, 0x00,
};

/* Lengths of bit sequences used to represent the base values of the copy length */
static BYTE s_LenBits[] = {
	0x03, 0x02, 0x03, 0x03, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x07, 0x07,
};

/* Base values used for the copy length */
static WORD s_LenBase[] = {
	0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009,
	0x000a, 0x000c, 0x0010, 0x0018, 0x0028, 0x0048, 0x0088, 0x0108,
};

/* Lengths of extra bits used to represent the copy length */
static BYTE s_ExLenBits[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
};

/* Bit sequences used to represent the most significant 6 bits of the copy offset */
static BYTE s_OffsCode[] = {
	0x03, 0x0d, 0x05, 0x19, 0x09, 0x11, 0x01, 0x3e, 0x1e, 0x2e, 0x0e, 0x36, 0x16, 0x26, 0x06, 0x3a,
	0x1a, 0x2a, 0x0a, 0x32, 0x12, 0x22, 0x42, 0x02, 0x7c, 0x3c, 0x5c, 0x1c, 0x6c, 0x2c, 0x4c, 0x0c,
	0x74, 0x34, 0x54, 0x14, 0x64, 0x24, 0x44, 0x04, 0x78, 0x38, 0x58, 0x18, 0x68, 0x28, 0x48, 0x08,
	0xf0, 0x70, 0xb0, 0x30, 0xd0, 0x50, 0x90, 0x10, 0xe0, 0x60, 0xa0, 0x20, 0xc0, 0x40, 0x80, 0x00,
};

/* Lengths of bit sequences used to represent the most significant 6 bits of the copy offset */
static BYTE s_OffsBits[] = {
	0x02, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
};

/************************************************************************/

BOOL implode(INT type, INT dict, VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT i;					// Index into tables
	BYTE ch;				// Byte from input buffer
	INT max_copy_len;		// Length of longest duplicate data in the dictionary
	BUFPTR max_copy_ptr;	// Pointer to longest duplicate data in the dictionary
	INT copy_len;			// Length of duplicate data in the dictionary
	INT copy_off;			// Offset used in actual compressed data
	INT new_copy_off;		// Secondary offset used in actual compressed data
	BUFPTR copy_ptr;		// Pointer to duplicate data in the dictionary
	BUFPTR bak_copy_ptr;	// Temporarily holds previous value of copy_ptr
	BUFCPTR new_rd_ptr;		// Secondary offset into input buffer
	BUFPTR new_dict_ptr;	// Secondary offset into dictionary
	BUFCPTR	rd_ptr;			// Current position in input buffer
	BUFPTR wrt_ptr;			// Current position in output buffer
	BUFCPTR src_end_ptr;	// Pointer to the end of source buffer
	BUFPTR dest_end_ptr;	// Pointer to the end of dest buffer
	BYTE bit_num;			// Number of bits in bit buffer
	DWORD bit_buf;			// Stores bits until there are enough to output a byte of data
	BUFPTR dict_ptr;		// Position in dictionary
	UINT dict_size;			// Maximum size of dictionary
	UINT cur_dict_size;		// Current size of dictionary
	BYTE dict_buf[4096];	// Sliding dictionary used for compression and decompression

	// Check for a valid compression type
	if (type != IMPLODE_BINARY && type != IMPLODE_ASCII)
		return FALSE;

	// Only dictionary sizes of 1024, 2048, and 4096 are allowed.
	// The values 4, 5, and 6 correspond with those sizes
	switch (dict) {
	case IMPLODE_DICT_1K:
		// Store actual dictionary size
		dict_size = 1024;
		break;
	case IMPLODE_DICT_2K:
		// Store actual dictionary size
		dict_size = 2048;
		break;
	case IMPLODE_DICT_4K:
		// Store actual dictionary size
		dict_size = 4096;
		break;
	default:
		return FALSE;
	}

	// Initialize buffer positions
	rd_ptr = src;
	wrt_ptr = dest;
	src_end_ptr = rd_ptr + src_size;
	dest_end_ptr = wrt_ptr + *dest_size;

	// Initialize dictionary position
	dict_ptr = dict_buf;

	// Initialize current dictionary size to zero
	cur_dict_size = 0;

	// If the output buffer size is less than 4, there
	// is not enough room for the compressed data
	if (*dest_size < 4 && !(src_size == 0 && *dest_size == 4))
		return FALSE;

	// Store compression type and dictionary size
	*wrt_ptr++ = type;
	*wrt_ptr++ = dict;

	// Initialize bit buffer
	bit_buf = 0;
	bit_num = 0;

	// Compress until input buffer is empty
	while (rd_ptr < src_end_ptr) {

		// Get a byte from the input buffer
		ch = *rd_ptr++;
		max_copy_len = 0;

		// If the dictionary is not empty, search for duplicate data in the dictionary
		if (cur_dict_size > 1 && src_end_ptr - rd_ptr > 1) {

			// Initialize offsets and lengths used in search
			copy_ptr = dict_buf;
			max_copy_ptr = copy_ptr;
			max_copy_len = 0;

			// Store position of last written dictionary byte
			new_dict_ptr = dict_ptr - 1;
			if (new_dict_ptr < dict_buf)
				new_dict_ptr = dict_buf + cur_dict_size - 1;

			// Search dictionary for duplicate data
			for (; copy_ptr < dict_buf + cur_dict_size; copy_ptr++) {

				// Check for a match with first byte
				if (ch != *copy_ptr)
					continue;

				bak_copy_ptr = copy_ptr;
				copy_len = 0;
				new_rd_ptr = rd_ptr - 1;

				// If there was a match, check for additional duplicate bytes
				do {

					// Increment pointers and length
					copy_len++;
					new_rd_ptr++;
					copy_ptr++;

					// Wrap around pointer to beginning of dictionary buffer if the end of the buffer was reached
					if (copy_ptr >= dict_buf + dict_size)
						copy_ptr = dict_buf;

					// Wrap dictionary bytes if end of the dictionary was reached
					if (copy_ptr == dict_ptr)
						copy_ptr = bak_copy_ptr;

					// Stop checking for additional bytes if there is no more input or maximum length was reached
					if (copy_len >= 518 || new_rd_ptr >= src_end_ptr)
						break;

				} while (*new_rd_ptr == *copy_ptr);

				// Return the pointer to the beginning of the matching data
				copy_ptr = bak_copy_ptr;

				// Copying less than two bytes from dictionary wastes space, so don't do it ;)
				if (copy_len < 2 || copy_len < max_copy_len)
					continue;

				// Store the offset that will be outputted into the compressed data
				new_copy_off = (new_dict_ptr - (copy_ptr - cur_dict_size)) % cur_dict_size;

				// If the length is equal, check for a more efficient offset
				if (copy_len == max_copy_len) {

					// Use the most efficient offset
					if (new_copy_off < copy_off) {
						copy_off = new_copy_off;
						max_copy_ptr = copy_ptr;
						max_copy_len = copy_len;
					}
				}
				// Only use the most efficient length and offset in dictionary
				else {

					// Store the offset that will be outputted into the compressed data
					copy_off = new_copy_off;

					// If the copy length is 2, check for a valid dictionary offset
					if (copy_len > 2 || copy_off <= 255) {
						max_copy_ptr = copy_ptr;
						max_copy_len = copy_len;
					}
				}
			}

			// If there were at least 2 matching bytes in the dictionary that were found, output the length/offset pair
			if (max_copy_len >= 2) {

				// Reset the input pointers to the bytes that will be added to the dictionary
				rd_ptr--;
				new_rd_ptr = rd_ptr + max_copy_len;

				while (rd_ptr < new_rd_ptr) {

					// Add a byte to the dictionary
					*dict_ptr++ = ch;

					// If the dictionary is not full yet, increment the current dictionary size
					if (cur_dict_size < dict_size)
						cur_dict_size++;

					// If the current end of the dictionary is past the end of the buffer,
					// wrap around back to the start
					if (dict_ptr >= dict_buf + dict_size)
						dict_ptr = dict_buf;

					// Get the next byte to be added
					if (++rd_ptr < new_rd_ptr)
						ch = *rd_ptr;
				}

				// Find bit code for the base value of the length from the table
				for (i = 0; i < 0x0F; i++) {

					if (s_LenBase[i] <= max_copy_len && max_copy_len < s_LenBase[i + 1])
						break;
				}

				// Store the base value of the length
				bit_buf += (1 + (s_LenCode[i] << 1)) << bit_num;
				bit_num += 1 + s_LenBits[i];

				// Store the extra bits for the length
				bit_buf += (max_copy_len - s_LenBase[i]) << bit_num;
				bit_num += s_ExLenBits[i];

				// Output the data from the bit buffer
				while (bit_num >= 8) {

					// If output buffer has become full, stop immediately!
					if (wrt_ptr >= dest_end_ptr)
						return FALSE;

					*wrt_ptr++ = (BYTE)bit_buf;
					bit_buf >>= 8;
					bit_num -= 8;
				}

				// The most significant 6 bits of the dictionary offset are encoded with a
				// bit sequence then the first 2 after that if the copy length is 2,
				// otherwise it is the first 4, 5, or 6 (based on the dictionary size)
				if (max_copy_len == 2) {

					// Store most significant 6 bits of offset using bit sequence
					bit_buf += s_OffsCode[copy_off >> 2] << bit_num;
					bit_num += s_OffsBits[copy_off >> 2];

					// Store the first 2 bits
					bit_buf += (copy_off & 0x03) << bit_num;
					bit_num += 2;
				}
				else {

					// Store most significant 6 bits of offset using bit sequence
					bit_buf += s_OffsCode[copy_off >> dict] << bit_num;
					bit_num += s_OffsBits[copy_off >> dict];

					// Store the first 4, 5, or 6 bits
					bit_buf += TRUNCATE_VALUE(copy_off, dict) << bit_num;
					bit_num += dict;
				}
			}
		}

		// If the copy length was less than two, include the byte as a literal byte
		if (max_copy_len < 2) {

			if (type == IMPLODE_BINARY) {

				// Store a fixed size literal byte
				bit_buf += ch << (bit_num + 1);
				bit_num += 9;
			}
			else {

				// Store a variable size literal byte
				bit_buf += s_ChCode[ch] << (bit_num + 1);
				bit_num += 1 + s_ChBits[ch];
			}

			// Add the byte into the dictionary
			*dict_ptr++ = ch;

			// If the dictionary is not full yet, increment the current dictionary size
			if (cur_dict_size < dict_size)
				cur_dict_size++;

			// If the current end of the dictionary is past the end of the buffer,
			// wrap around back to the start
			if (dict_ptr >= dict_buf + dict_size)
				dict_ptr = dict_buf;
		}

		// Write any whole bytes from the bit buffer into the output buffer
		while (bit_num >= 8) {

			// If output buffer has become full, stop immediately!
			if (wrt_ptr >= dest_end_ptr)
				return FALSE;

			*wrt_ptr++ = (BYTE)bit_buf;
			bit_buf >>= 8;
			bit_num -= 8;
		}
	}

	// Store the code for the end of the compressed data stream
	bit_buf += (1 + (s_LenCode[0x0f] << 1)) << bit_num;
	bit_num += 1 + s_LenBits[0x0f];

	bit_buf += 0xff << bit_num;
	bit_num += 8;

	// Write any remaining bits from the bit buffer into the output buffer
	while (bit_num > 0) {

		// If output buffer has become full, stop immediately!
		if (wrt_ptr >= dest_end_ptr)
			return FALSE;

		*wrt_ptr++ = (BYTE)bit_buf;
		bit_buf >>= 8;
		if (bit_num >= 8)
			bit_num -= 8;
		else
			bit_num = 0;
	}

	// Store the compressed size
	*dest_size = wrt_ptr - (BUFPTR)dest;

	return TRUE;
}

BOOL explode(VCPTR src, UINT src_size, VPTR dest, UINT *dest_size)
{
	INT i;					// Index into tables
	INT copy_len;			// Length of data to copy from the dictionary
	BUFPTR copy_off;		// Offset to data to copy from the dictionary
	BYTE type;				// Specifies whether to use fixed or variable size literal bytes
	BYTE dict;				// Dictionary size; valid values are 4, 5, and 6 which represent 1024, 2048, and 4096 respectively
	BUFCPTR	rd_ptr;			// Current position in input buffer
	BUFPTR wrt_ptr;			// Current position in output buffer
	BUFCPTR src_end_ptr;	// Pointer to the end of source buffer
	BUFPTR dest_end_ptr;	// Pointer to the end of dest buffer
	BYTE bit_num;			// Number of bits in bit buffer
	DWORD bit_buf;			// Stores bits until there are enough to output a byte of data
	BUFPTR dict_ptr;		// Position in dictionary
	UINT dict_size;			// Maximum size of dictionary
	UINT cur_dict_size;		// Current size of dictionary
	BYTE dict_buf[0x1000];	// Sliding dictionary used for compression and decompression

	// Compressed data cannot be less than 4 bytes;
	// this is not possible in any case whatsoever
	if (src_size < 4) {
		*dest_size = 0;
		return FALSE;
	}

	// Initialize buffer positions
	rd_ptr = src;
	wrt_ptr = dest;
	src_end_ptr = rd_ptr + src_size;
	dest_end_ptr = wrt_ptr + *dest_size;

	// Get header from compressed data
	type = *rd_ptr++;
	dict = *rd_ptr++;

	// Check for a valid compression type
	if (type != IMPLODE_BINARY && type != IMPLODE_ASCII)
		return FALSE;

	// Only dictionary sizes of 1024, 2048, and 4096 are allowed.
	// The values 4, 5, and 6 correspond with those sizes
	switch (dict) {
	case IMPLODE_DICT_1K:
		// Store actual dictionary size
		dict_size = 1024;
		break;
	case IMPLODE_DICT_2K:
		// Store actual dictionary size
		dict_size = 2048;
		break;
	case IMPLODE_DICT_4K:
		// Store actual dictionary size
		dict_size = 4096;
		break;
	default:
		return FALSE;
	}

	// Initialize dictionary position
	dict_ptr = dict_buf;

	// Initialize current dictionary size to zero
	cur_dict_size = 0;

	// Get first 16 bits
	bit_buf = *rd_ptr++;
	bit_buf += *rd_ptr++ << 8;
	bit_num = 16;

	// Decompress until output buffer is full
	while (wrt_ptr < dest_end_ptr) {

		// Fill bit buffer with at least 16 bits
		while (bit_num < 16) {

			// If input buffer is empty before end of stream, buffer is incomplete
			if (rd_ptr >= src_end_ptr) {

				// Store the current size of output
				*dest_size = wrt_ptr - (BUFPTR)dest;
				return FALSE;
			}

			bit_buf += *rd_ptr++ << bit_num;
			bit_num += 8;
		}

		// First bit is 1; copy from dictionary
		if (bit_buf & 1) {

			// Remove first bit from bit buffer
			bit_buf >>= 1;
			bit_num--;

			// Find the base value for the copy length
			for (i = 0; i <= 0x0F; i++) {

				if (TRUNCATE_VALUE(bit_buf, s_LenBits[i]) == s_LenCode[i])
					break;
			}

			// Remove value from bit buffer
			bit_buf >>= s_LenBits[i];
			bit_num -= s_LenBits[i];

			// Store the copy length
			copy_len = s_LenBase[i] + TRUNCATE_VALUE(bit_buf, s_ExLenBits[i]);

			// Remove the extra bits from the bit buffer
			bit_buf >>= s_ExLenBits[i];
			bit_num -= s_ExLenBits[i];

			// If copy length is 519, the end of the stream has been reached
			if (copy_len == 519)
				break;

			// Fill bit buffer with at least 14 bits
			while (bit_num < 14) {

				// If input buffer is empty before end of stream, buffer is incomplete
				if (rd_ptr >= src_end_ptr) {

					// Store the current size of output
					*dest_size = wrt_ptr - (BUFPTR)dest;
					return FALSE;
				}

				bit_buf += *rd_ptr++ << bit_num;
				bit_num += 8;
			}

			// Find most significant 6 bits of offset into the dictionary
			for (i = 0; i <= 0x3f; i++) {

				if (TRUNCATE_VALUE(bit_buf, s_OffsBits[i]) == s_OffsCode[i])
					break;
			}

			// Remove value from bit buffer
			bit_buf >>= s_OffsBits[i];
			bit_num -= s_OffsBits[i];

			// If the copy length is 2, there are only two more bits in the dictionary
			// offset; otherwise, there are 4, 5, or 6 bits left, depending on what
			// the dictionary size is
			if (copy_len == 2) {

				// Store the exact offset to a byte in the dictionary
				copy_off = dict_ptr - 1 - ((i << 2) + (bit_buf & 0x03));

				// Remove the rest of the dictionary offset from the bit buffer
				bit_buf >>= 2;
				bit_num -= 2;
			}
			else {

				// Store the exact offset to a byte in the dictionary
				copy_off = dict_ptr - 1 - ((i << dict) + TRUNCATE_VALUE(bit_buf, dict));

				// Remove the rest of the dictionary offset from the bit buffer
				bit_buf >>= dict;
				bit_num -= dict;
			}

			// While there are still bytes left, copy bytes from the dictionary
			while (copy_len-- > 0) {

				// If output buffer has become full, stop immediately!
				if (wrt_ptr >= dest_end_ptr) {

					// Store the current size of output
					*dest_size = wrt_ptr - (BUFPTR)dest;
					return FALSE;
				}

				// Check whether the offset is a valid one into the dictionary
				while (copy_off < dict_buf)
					copy_off += cur_dict_size;
				while (copy_off >= dict_buf + cur_dict_size)
					copy_off -= cur_dict_size;

				// Copy the byte from the dictionary and add it to the end of the dictionary
				*dict_ptr++ = *wrt_ptr++ = *copy_off++;

				// If the dictionary is not full yet, increment the current dictionary size
				if (cur_dict_size < dict_size)
					cur_dict_size++;

				// If the current end of the dictionary is past the end of the buffer,
				// wrap around back to the start
				if (dict_ptr >= dict_buf + dict_size)
					dict_ptr = dict_buf;
			}
		}

		// First bit is 0; literal byte
		else {

			// Fixed size literal byte
			if (type == IMPLODE_BINARY) {

				// Copy the byte and add it to the end of the dictionary
				*dict_ptr++ = (BYTE)(bit_buf >> 1);
				*wrt_ptr++ = (BYTE)(bit_buf >> 1);

				// Remove the byte from the bit buffer
				bit_buf >>= 9;
				bit_num -= 9;
			}

			// Variable size literal byte
			else {

				// Remove the first bit from the bit buffer
				bit_buf >>= 1;
				bit_num--;

				// Find the actual byte from the bit sequence
				for (i = 0; i <= 0xff; i++) {
					if (TRUNCATE_VALUE(bit_buf, s_ChBits[i]) == s_ChCode[i])
						break;
				}

				// Copy the byte and add it to the end of the dictionary
				*dict_ptr++ = i;
				*wrt_ptr++ = i;

				// Remove the byte from the bit buffer
				bit_buf >>= s_ChBits[i];
				bit_num -= s_ChBits[i];
			}

			// If the dictionary is not full yet, increment the current dictionary size
			if (cur_dict_size < dict_size)
				cur_dict_size++;

			// If the current end of the dictionary is past the end of the buffer,
			// wrap around back to the start
			if (dict_ptr >= dict_buf + dict_size)
				dict_ptr = dict_buf;
		}
	}

	// Store the decompressed size
	*dest_size = wrt_ptr - (BUFPTR)dest;

	return TRUE;
}

/************************************************************************/
