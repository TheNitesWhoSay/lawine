/************************************************************************/
/* File Name   : mpq.cpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 4th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DMpq class implementation                              */
/************************************************************************/

#include <array.hpp>
#include "mpq.hpp"
#include "../misc/adpcm.h"
#include "../misc/implode.h"
#include "../misc/huffman.h"

/************************************************************************/

CONST WORD SUPPORT_VERSION = 0;
CONST WORD SUPPORT_PLATFORM = 0;
CONST WORD SUPPORT_SECTOR_SHIFT = 3;
CONST DWORD SUPPORT_HEADER_SIZE = 32UL;			// sizeof(DMpq::HEADER)

CONST WORD PHYSICAL_SECTOR_SHIFT = 9;			// 512 bytes

CONST DWORD MPQ_IDENTIFIER = '\x1aQPM';			// FourCC 'MPQ\x1a'

CONST DWORD HASH_ENTRY_INVALID = 0xfffffffeUL;	// Block index for deleted hash entry
CONST DWORD HASH_ENTRY_EMPTY = 0xffffffffUL;	// Block index for free hash entry

CONST DWORD BLOCK_COMP_MASK = 0x0000ff00UL;		// File compressed mask
CONST DWORD BLOCK_IMPLODE = 0x00000100UL;		// Implode method (by PKWare Data Compression Library)
CONST DWORD BLOCK_COMPRESS = 0x00000200UL;		// Compress methods (by multiple methods)
CONST DWORD BLOCK_ENCRYPT = 0x00010000UL;		// Indicates whether file is encrypted
CONST DWORD BLOCK_FIX_KEY = 0x00020000UL;		// File decryption key has to be fixed
CONST DWORD BLOCK_EXIST = 0x80000000UL;			// Set if file exists, reset when the file was deleted

CONST BYTE COMP_NONE = 0x00;					// Not compressed
CONST BYTE COMP_HUFFMAN = 0x01;					// Huffman compression
CONST BYTE COMP_IMPLODE = 0x08;					// PKWARE DCL compression
CONST BYTE COMP_ADPCM_BETA_MONO = 0x10;			// ADPCM compression for Starcraft Beta (mono)
CONST BYTE COMP_ADPCM_BETA_STEREO = 0x20;		// ADPCM compression for Starcraft Beta (stereo)
CONST BYTE COMP_ADPCM_MONO = 0x40;				// ADPCM compression (mono)
CONST BYTE COMP_ADPCM_STEREO = 0x80;			// ADPCM compression (stereo)

CONST UINT HASH_NUM_MIN = 0x00000001U;			// Minimum acceptable hash size
CONST UINT HASH_NUM_MAX = 0x00080000U;			// Maximum acceptable hash size

CONST UINT PHYSICAL_SECTOR_SIZE = 1 << PHYSICAL_SECTOR_SHIFT;

CONST STRCPTR HASH_TABLE_KEY = "(hash table)";
CONST STRCPTR BLOCK_TABLE_KEY = "(block table)";

CONST BYTE VALID_COMP[] = {						// In fix order!
	COMP_ADPCM_BETA_MONO, COMP_ADPCM_BETA_STEREO,
	COMP_ADPCM_MONO, COMP_ADPCM_STEREO,
	COMP_HUFFMAN, COMP_IMPLODE,
};

/************************************************************************/

LCID DMpq::s_Locale;
DString DMpq::s_BashPath;
DWORD DMpq::s_HashTable[HASH_TABLE_NUM][0x100];

/************************************************************************/

DMpq::DMpq() :
	m_HashNum(0U),
	m_Access(NULL),
	m_HashTable(NULL)
{

}

DMpq::~DMpq()
{
	Clear();
}

/************************************************************************/

BOOL DMpq::CreateArchive(STRCPTR mpq_name, UINT &max_hash_num)
{
	if (!mpq_name)
		return FALSE;

	if (m_Access)
		return FALSE;

	UINT adjust_size = HASH_NUM_MIN;
	for (; max_hash_num > adjust_size; adjust_size <<= 1);
	if (adjust_size > HASH_NUM_MAX)
		return FALSE;

	m_Access = new DAccess;

	if (!Create(mpq_name, adjust_size)) {
		Clear();
		DFile::Remove(mpq_name);
		return FALSE;
	}

	max_hash_num = adjust_size;
	return TRUE;
}

BOOL DMpq::OpenArchive(STRCPTR mpq_name)
{
	if (!mpq_name)
		return FALSE;

	if (m_Access)
		return FALSE;

	m_Access = new DAccess;

	if (!Load(mpq_name)) {
		Clear();
		return FALSE;
	}

	return TRUE;
}

BOOL DMpq::CloseArchive(VOID)
{
	if (!m_Access)
		return FALSE;

	Clear();
	return TRUE;
}

BOOL DMpq::FileExist(STRCPTR file_name)
{
	if (!file_name || !*file_name)
		return FALSE;

	if (!m_Access || !m_Access->Readable())
		return FALSE;

	HASHENTRY *hash = Lookup(file_name);
	if (!hash || hash->block_index >= m_BlockTable.size())
		return FALSE;

	BLOCKENTRY *block = &m_BlockTable[hash->block_index];
	if (!(block->flags & BLOCK_EXIST))
		return FALSE;

	return TRUE;
}

BOOL DMpq::FileExist(HANDLE file)
{
	if (!file)
		return FALSE;

	DSubFile *sub = static_cast<DSubFile *>(file);
	if (sub->GetAccess() != m_Access)
		return FALSE;

	return TRUE;
}

HANDLE DMpq::OpenFile(STRCPTR file_name)
{
	if (!file_name || !*file_name)
		return NULL;

	if (!m_Access || !m_Access->Readable())
		return NULL;

	HASHENTRY *hash = Lookup(file_name);
	if (!hash || hash->block_index >= m_BlockTable.size())
		return NULL;

	BLOCKENTRY *block = &m_BlockTable[hash->block_index];
	if (!(block->flags & BLOCK_EXIST))
		return NULL;

	DWORD key = CalcFileKey(file_name, *block);
	DSubFile *sub = new DSubFile;
	if (!sub->Open(m_Access, hash->block_index, *block, key)) {
		delete sub;
		return NULL;
	}

	m_FileList.push_back(sub);
	return static_cast<HANDLE>(sub);
}

BOOL DMpq::CloseFile(HANDLE file)
{
	for (DFileList::iterator it = m_FileList.begin(); it != m_FileList.end(); ++it) {
		DSubFile *sub = *it;
		if (sub != file)
			continue;
		DVerify(sub->Close());
		delete sub;
		m_FileList.erase(it);
		return TRUE;
	}

	return FALSE;
}

HANDLE DMpq::OpenHandle(STRCPTR file_name)
{
	if (!file_name || !*file_name)
		return NULL;

	if (!m_Access || !m_Access->Readable())
		return NULL;

	HASHENTRY *hash = Lookup(file_name);
	if (!hash || hash->block_index >= m_BlockTable.size())
		return NULL;

	BLOCKENTRY *block = &m_BlockTable[hash->block_index];

	if (!(block->flags & BLOCK_EXIST))
		return NULL;

	if (block->flags & BLOCK_COMP_MASK)
		return NULL;

	if (block->flags & BLOCK_ENCRYPT)
		return NULL;

	if (!m_Access->Seek(block->offset))
		return NULL;

	return m_Access->ShareHandle();
}

BOOL DMpq::AddFile(STRCPTR file_name, STRCPTR real_path, BOOL compress, BOOL encrypt)
{
	if (!file_name || !*file_name || !real_path)
		return FALSE;

	if (!m_Access || !m_Access->Writable())
		return FALSE;

	DFile file;
	if (!file.Open(real_path))
		return FALSE;

	BOOL ret = AddFile(file_name, compress, encrypt, file);
	file.Close();

	return ret;
}

BOOL DMpq::NewFile(STRCPTR file_name, BUFCPTR file_data, UINT size, BOOL compress, BOOL encrypt)
{
	if (!file_name || !*file_name)
		return FALSE;

	if (!file_data && size)
		return FALSE;

	if (!m_Access || !m_Access->Writable())
		return FALSE;

	UINT block_idx;
	DWORD key;
	BYTE comp;
	BLOCKENTRY block;
	HASHENTRY *hash = PrepareAdd(file_name, size, compress, encrypt, block_idx, block, key, comp);
	if (!hash)
		return FALSE;

	DSubFile *sub = new DSubFile;
	if (!NewFile(sub, hash, block_idx, block, key, comp, file_data)) {
		delete sub;
		return FALSE;
	}

	m_FileList.push_back(sub);
	return TRUE;
}

BOOL DMpq::DelFile(STRCPTR file_name)
{
	if (!file_name || !*file_name)
		return FALSE;

	if (!m_Access || !m_Access->Writable())
		return FALSE;

	HASHENTRY *hash = Lookup(file_name);
	if (!hash)
		return FALSE;

	// reserve block index before delete
	UINT block_idx = hash->block_index;

	DMemSet(hash, 0xff, sizeof(HASHENTRY));

	// mark as deleted
	UINT entry = (hash - m_HashTable + 1) & (m_HashNum - 1);
	if (m_HashTable[entry].block_index == HASH_ENTRY_EMPTY)
		hash->block_index = HASH_ENTRY_EMPTY;
	else
		hash->block_index = HASH_ENTRY_INVALID;

	if (block_idx >= m_BlockTable.size())
		return TRUE;

	BLOCKENTRY *block = &m_BlockTable[hash->block_index];

	if (block->data_size) {
		block->file_size = 0UL;
		block->flags = 0UL;
	} else {
		DMemClr(block, sizeof(BLOCKENTRY));
	}

	// TODO: 由于目前只有Create的MPQ才能进行write操作
	// 因此只需考虑文件布局如下的情况：
	//		Header
	//		File Data
	//		Hash Table
	//		Block Table
	// 因此File Data的末尾也是Hash Table的开始
	return Writeback(GetEndOfFileData());
}

UINT DMpq::GetFileSize(HANDLE file)
{
	if (!file)
		return ERROR_SIZE;

	DSubFile *sub = static_cast<DSubFile *>(file);
	return sub->GetSize();
}

UINT DMpq::ReadFile(HANDLE file, VPTR data, UINT size)
{
	if (!file)
		return 0U;

	DSubFile *sub = static_cast<DSubFile *>(file);
	return sub->Read(data, size);
}

UINT DMpq::SeekFile(HANDLE file, INT offset, SEEK_MODE mode /* = SM_BEGIN */)
{
	if (!file)
		return ERROR_POS;

	DSubFile *sub = static_cast<DSubFile *>(file);
	return sub->Seek(offset, mode);
}

BOOL DMpq::Initialize(VOID)
{
	s_Locale = 0x0409U;
	s_BashPath = DGetCwd();

	DWORD seed = 0x00100001UL;

	for (INT j = 0; j < 0x100; j++) {
		for (INT i = 0; i < HASH_TABLE_NUM; i++) {
			seed = (seed * 125 + 3) % 0x2aaaab;
			DWORD high = seed & 0xffff;
			seed = (seed * 125 + 3) % 0x2aaaab;
			DWORD low = seed & 0xffff;
			s_HashTable[i][j] = DMakeDWord(low, high);
		}
	}

	return TRUE;
}

VOID DMpq::Exit(VOID)
{
	DVarClr(s_HashTable);

	s_Locale = 0UL;
}

BOOL DMpq::SetBasePath(STRCPTR path)
{
	if (!path || *path)
		return FALSE;

	UINT size = DFile::GetFullPath(path);
	if (!size)
		return FALSE;

	DArray<CHAR> str(size);

	if (DFile::GetFullPath(path, str, size) != size)
		return FALSE;

	if (!DFile::IsDir(path))
		return FALSE;

	s_BashPath.Assign(str);
	return TRUE;
}

STRCPTR DMpq::GetBasePath(VOID)
{
	return s_BashPath;
}

VOID DMpq::SetLocale(LCID locale)
{
	s_Locale = locale;
}

LCID DMpq::GetLocale(VOID)
{
	return s_Locale;
}

/************************************************************************/

BOOL DMpq::Create(STRCPTR mpq_name, UINT hash_num)
{
	DAssert(mpq_name && hash_num);
	DAssert(m_Access && !m_HashTable && !m_BlockTable.size());

	if (!m_Access->Create(mpq_name))
		return FALSE;

	UINT hash_size = hash_num * sizeof(HASHENTRY);
	m_HashTable = new HASHENTRY[hash_num];
	DMemSet(m_HashTable, 0xff, hash_size);

	HEADER header;
	header.identifier = MPQ_IDENTIFIER;
	header.header_size = sizeof(header);
	header.archive_size = sizeof(header) + hash_size;
	header.version = SUPPORT_VERSION;
	header.sector_shift = SUPPORT_SECTOR_SHIFT;
	header.hash_table_offset = sizeof(header);
	header.block_table_offset = 0UL;
	header.hash_num = hash_num;
	header.block_num = 0UL;

	if (!m_Access->Write(&header, sizeof(header)))
		return FALSE;

	DArray<HASHENTRY> hash_table(hash_num);
	DMemCpy(hash_table, m_HashTable, hash_size);

	DWORD key = HashString(HASH_TABLE_KEY, HASH_FILE_KEY);
	EncryptData(hash_table, hash_size, key);

	if (!m_Access->Write(hash_table, hash_size))
		return FALSE;

	m_HashNum = hash_num;
	m_BlockTable.clear();

	return TRUE;
}

BOOL DMpq::Load(STRCPTR mpq_name)
{
	DAssert(mpq_name);
	DAssert(m_Access && !m_HashTable && !m_BlockTable.size());

	if (!m_Access->Open(mpq_name))
		return FALSE;

	HEADER header;
	if (!m_Access->Read(&header, sizeof(header)))
		return FALSE;

	if (header.version != SUPPORT_VERSION)
		return FALSE;

	if (header.hash_num < HASH_NUM_MIN || header.hash_num > HASH_NUM_MAX)
		return FALSE;

	m_HashNum = header.hash_num;

	if (!m_Access->Seek(header.hash_table_offset))
		return FALSE;

	m_HashTable = new HASHENTRY[m_HashNum];
	UINT size = m_HashNum * sizeof(HASHENTRY);
	if (!m_Access->Read(m_HashTable, size))
		return FALSE;

	DWORD key = HashString(HASH_TABLE_KEY, HASH_FILE_KEY);
	DecryptData(m_HashTable, size, key);

	UINT block_num = header.block_num;

	if (block_num) {

		if (!m_Access->Seek(header.block_table_offset))
			return FALSE;

		m_BlockTable.resize(block_num);
		size = block_num * sizeof(BLOCKENTRY);
		if (!m_Access->Read(&m_BlockTable.front(), size))
			return FALSE;

		key = HashString(BLOCK_TABLE_KEY, HASH_FILE_KEY);
		DecryptData(&m_BlockTable.front(), size, key);
	}

	return TRUE;
}

VOID DMpq::Clear(VOID)
{
	for (DFileList::iterator it = m_FileList.begin(); it != m_FileList.end(); ++it)
		delete *it;

	m_FileList.clear();
	m_BlockTable.clear();

	m_HashNum = 0U;

	delete [] m_HashTable;
	m_HashTable = NULL;

	delete m_Access;
	m_Access = NULL;
}

BOOL DMpq::AddFile(STRCPTR file_name, BOOL compress, BOOL encrypt, DFile &file)
{
	DAssert(file_name && file.IsOpen());

	UINT size = file.GetSize();
	if (size == ERROR_SIZE)
		return FALSE;

	UINT block_idx;
	DWORD key;
	BYTE comp;
	BLOCKENTRY block;
	HASHENTRY *hash = PrepareAdd(file_name, size, compress, encrypt, block_idx, block, key, comp);
	if (!hash)
		return FALSE;

	DSubFile *sub = new DSubFile;
	if (!AddFile(sub, hash, block_idx, block, key, comp, file)) {
		delete sub;
		return FALSE;
	}

	m_FileList.push_back(sub);
	return TRUE;
}

BOOL DMpq::AddFile(DSubFile *sub, HASHENTRY *hash, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp, DFile &file)
{
	DAssert(sub && hash && m_Access && m_HashNum);

	if (!sub->Create(m_Access, block_idx, block, key, comp))
		return FALSE;

	BOOL flag = TRUE;
	UINT size = block.file_size;
	UINT buf_size = 1 << (m_Access->SectorShift() + 3);
	DArray<BYTE> rd_buf(buf_size);

	for (UINT rd_size = 0; size && flag; size -= rd_size) {
		rd_size = file.Read(rd_buf, buf_size);
		if (!rd_size)
			break;
		if (sub->Write(rd_buf, rd_size) != rd_size)
			flag = FALSE;
	}
	DAssert(flag);

	if (!flag)
		return FALSE;

	if  (!Writeback(sub, hash, block_idx))
		return FALSE;

	return TRUE;
}

BOOL DMpq::NewFile(DSubFile *sub, HASHENTRY *hash, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp, BUFCPTR file_data)
{
	DAssert(sub && m_Access && m_HashNum);

	if (!sub->Create(m_Access, block_idx, block, key, comp))
		return FALSE;

	if (sub->Write(file_data, block.file_size) != block.file_size)
		return FALSE;

	if  (!Writeback(sub, hash, block_idx))
		return FALSE;

	return TRUE;
}

DMpq::HASHENTRY *DMpq::PrepareAdd(STRCPTR file_name, UINT file_size, BOOL compress, BOOL encrypt, UINT &block_idx, BLOCKENTRY &block, DWORD &key, BYTE &compression)
{
	DAssert(file_name && *file_name);

	// TODO: 由于目前只有Create的MPQ才能进行write操作
	// 因此只需考虑文件布局如下的情况：
	//		Header
	//		File Data
	//		Hash Table
	//		Block Table

	block_idx = AllocBlock(file_size, compress, encrypt, block);
	if (block_idx == HASH_ENTRY_INVALID || block_idx == HASH_ENTRY_EMPTY)
		return NULL;

	HASHENTRY *hash = AllocHash(file_name);
	if (!hash)
		return NULL;

	if (block.flags & BLOCK_COMPRESS) {
		// TODO: Wave file
		INT len = DStrLen(file_name);
		if (len > 4 && !DStrCmpI(file_name + len - 4, ".wav"))
			compression = COMP_ADPCM_STEREO | COMP_HUFFMAN;
		else
			compression = COMP_IMPLODE;
	} else {
		compression = COMP_NONE;
	}

	key = CalcFileKey(file_name, block);

	return hash;
}

BOOL DMpq::Writeback(DSubFile *sub, HASHENTRY *hash, UINT block_idx)
{
	DWORD org_block_idx = hash->block_index;
	hash->block_index = block_idx;

	CONST BLOCKENTRY *block = sub->GetBlock();
	DAssert(block);
	m_BlockTable.push_back(*block);

	if (Writeback(block->offset + block->data_size))
		return TRUE;

	hash->block_index = org_block_idx;
	m_BlockTable.pop_back();
	return FALSE;
}

BOOL DMpq::Writeback(UINT hash_table_offset)
{
	DAssert(m_Access && m_HashNum && m_HashTable && m_BlockTable.size());
	DAssert(m_Access->Writable());

	UINT hash_size =  m_HashNum * sizeof(HASHENTRY);
	UINT block_size = m_BlockTable.size() * sizeof(BLOCKENTRY);

	// 更新文件头并写入
	HEADER header;
	header.identifier = MPQ_IDENTIFIER;
	header.header_size = sizeof(header);
	header.hash_table_offset = hash_table_offset;
	header.block_table_offset = hash_table_offset + hash_size;
	header.archive_size = header.block_table_offset + block_size;
	header.version = SUPPORT_VERSION;
	header.sector_shift = SUPPORT_SECTOR_SHIFT;
	header.hash_num = m_HashNum;
	header.block_num = m_BlockTable.size();

	if (!m_Access->Seek(0U))
		return FALSE;

	if (!m_Access->Write(&header, sizeof(header)))
		return FALSE;

	// 写散列表
	if (!m_Access->Seek(header.hash_table_offset))
		return FALSE;

	DArray<HASHENTRY> hash_table(m_HashNum * sizeof(HASHENTRY));
	DMemCpy(hash_table, m_HashTable, hash_size);

	DWORD key = HashString(HASH_TABLE_KEY, HASH_FILE_KEY);
	EncryptData(hash_table, hash_size, key);

	BOOL ret = m_Access->Write(hash_table, hash_size);
	if (!ret)
		return FALSE;

	// 写块表
	if (!m_Access->Seek(header.block_table_offset))
		return FALSE;

	DArray<BLOCKENTRY> block_table(m_BlockTable.size());
	DMemCpy(block_table, &m_BlockTable.front(), block_size);

	key = HashString(BLOCK_TABLE_KEY, HASH_FILE_KEY);
	EncryptData(block_table, block_size, key);

	ret = m_Access->Write(block_table, block_size);
	if (!ret)
		return FALSE;

	return TRUE;
}

DMpq::HASHENTRY *DMpq::Lookup(STRCPTR file_name)
{
	DAssert(file_name && *file_name);
	DAssert(m_HashNum && m_HashTable);

	// 根据散列值计算文件的起始入口
	DWORD entry = HashString(file_name, HASH_TABLE_ENTRY);

	// 计算散列值，用以比较散列表入口冲突
	DWORD hash_low = HashString(file_name, HASH_NAME_LOW);
	DWORD hash_high = HashString(file_name, HASH_NAME_HIGH);

	LANGID lang = DLoc2Lang(GetLocale());

	INT start = entry & (m_HashNum - 1);
	INT index = start;

	HASHENTRY *best = NULL;

	// 循环查找
	do {

		HASHENTRY *hash = m_HashTable + index;
		index = ++entry & (m_HashNum - 1);

		if (hash->block_index == HASH_ENTRY_EMPTY)
			break;
		if (hash->block_index == HASH_ENTRY_INVALID)
			continue;
		if (hash->hash_low != hash_low || hash_high != hash_high)
			continue;

		if (hash->language != lang && hash->language != LANG_NEUTRAL)
			continue;
		if (hash->platform != SUPPORT_PLATFORM)
			continue;

		if (hash->language == lang && hash->platform == SUPPORT_PLATFORM)
			return hash;

		best = hash;

	} while (index != start);

	// 已经到达末尾
	return best;
}

DMpq::HASHENTRY *DMpq::AllocHash(STRCPTR file_name)
{
	DAssert(file_name && *file_name);
	DAssert(m_HashNum && m_HashTable);

	// 根据散列值计算文件的起始入口
	DWORD entry = HashString(file_name, HASH_TABLE_ENTRY);

	// 计算散列值，用以比较散列表入口冲突
	DWORD hash_low = HashString(file_name, HASH_NAME_LOW);
	DWORD hash_high = HashString(file_name, HASH_NAME_HIGH);

	LANGID lang = DLoc2Lang(GetLocale());

	INT start = entry & (m_HashNum - 1);
	INT index = start;

	// 循环查找
	do {

		HASHENTRY *hash = m_HashTable + index;
		index = ++entry & (m_HashNum - 1);

		if (hash->block_index == HASH_ENTRY_EMPTY) {
			hash->hash_low = hash_low;
			hash->hash_high = hash_high;
			hash->language = lang;
			hash->platform = SUPPORT_PLATFORM;
			return hash;
		}

		if (hash->hash_low != hash_low || hash_high != hash_high)
			continue;
		if (hash->language != lang || hash->platform != SUPPORT_PLATFORM)
			continue;

		// 文件已存在
		return NULL;

	} while (index != start);


	// 已经到达末尾
	return NULL;
}

UINT DMpq::AllocBlock(UINT file_size, BOOL compress, BOOL encrypt, BLOCKENTRY &block)
{
	// TODO: 如果有已被删除的块，可以回收利用
	// 难点在于合并几个相邻的小块来凑空间
	// 以及如果最后一个块是被删除了话的则再小也够，但优先用前面的删除块

	block.data_size = 0UL;
	block.file_size = file_size;
	block.flags = BLOCK_EXIST | BLOCK_FIX_KEY;
	block.offset = GetEndOfFileData();

	if (compress)
		block.flags |= BLOCK_COMPRESS;

	if (encrypt)
		block.flags |= BLOCK_ENCRYPT;

	return m_BlockTable.size();
}

UINT DMpq::GetEndOfFileData(VOID)
{
	// 寻找文件数据的末尾
	UINT offset = sizeof(HEADER);
	for (UINT i = 0U; i < m_BlockTable.size(); i++) {
		if (!(m_BlockTable[i].flags & BLOCK_EXIST))
			continue;
		UINT end_pos = m_BlockTable[i].offset + m_BlockTable[i].data_size;
		if (offset < end_pos)
			offset = end_pos;
	}

	return offset;
}

DWORD DMpq::CalcFileKey(STRCPTR path_name, CONST BLOCKENTRY &block)
{
	DAssert(path_name && (block.flags & BLOCK_EXIST));

	// 从路径中找出文件名部分
	STRCPTR name = DStrRChr(path_name, '\\');
	name = name ? ++name : path_name;

	// 计算文件名的散列值，得到基础密钥
	DWORD key = HashString(name, HASH_FILE_KEY);

	// 必要时用偏移调整密钥
	if (block.flags & BLOCK_FIX_KEY)
		key = (key + block.offset) ^ block.file_size;

	return key;
}

DWORD DMpq::HashString(STRCPTR str, INT hash_type)
{
	INT ch;
	DWORD seed1, seed2;

	DAssert(str && DBetween(hash_type, 0, HASH_TYPE_NUM));

	seed1 = 0x7fed7fedUL;
	seed2 = 0xeeeeeeeeUL;

	while (*str) {

		ch = DToUpper(*str++);

		seed1 = s_HashTable[hash_type][ch] ^ (seed1 + seed2);
		seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
	}

	return seed1;
}

VOID DMpq::EncryptData(VPTR buf, UINT size, DWORD key)
{
	DAssert(buf && size);

	DWORD seed = 0xeeeeeeeeUL;
	DWORD *cur = static_cast<DWORD *>(buf);

	for (size /= sizeof(DWORD); size > 0; size--, cur++) {

		seed += s_HashTable[CRYPT_TABLE_INDEX][key & 0xff];
		DWORD val = *cur;
		*cur ^= key + seed;

		key = ((~key << 21) + 0x11111111) | (key >> 11);
		seed = val + seed + (seed << 5) + 3;
	}
}

VOID DMpq::DecryptData(VPTR buf, UINT size, DWORD key)
{
	DAssert(buf && size);

	DWORD seed = 0xeeeeeeeeUL;
	DWORD *cur = static_cast<DWORD *>(buf);

	for (size /= sizeof(DWORD); size > 0; size--, cur++) {

		seed += s_HashTable[CRYPT_TABLE_INDEX][key & 0xff];
		*cur ^= key + seed;

		key = ((~key << 21) + 0x11111111) | (key >> 11);
		seed = *cur + seed + (seed << 5) + 3;
	}
}

/************************************************************************/

DMpq::DAccess::DAccess() :
	m_ReadAccess(FALSE),
	m_WriteAccess(FALSE),
	m_ArchiveOff(0U),
	m_SectorShift(0U),
	m_SectorBuffer(NULL)
{

}

DMpq::DAccess::~DAccess()
{
	Clear();
}

BOOL DMpq::DAccess::Readable(VOID) CONST
{
	return m_ReadAccess;
}

BOOL DMpq::DAccess::Writable(VOID) CONST
{
	return m_WriteAccess;
}

UINT DMpq::DAccess::SectorShift(VOID) CONST
{
	return m_SectorShift;
}

BOOL DMpq::DAccess::Create(STRCPTR mpq_name)
{
	if (!mpq_name)
		return FALSE;

	if (m_File.IsOpen())
		return FALSE;

	if (!m_File.Open(mpq_name, DFile::OM_WRITE | DFile::OM_CREATE | DFile::OM_TRUNCATE))
		return FALSE;

	m_ReadAccess = FALSE;
	m_WriteAccess = TRUE;
	m_ArchiveOff = 0U;
	m_SectorShift = SUPPORT_SECTOR_SHIFT + PHYSICAL_SECTOR_SHIFT;

	return TRUE;
}

BOOL DMpq::DAccess::Open(STRCPTR mpq_name)
{
	if (!mpq_name)
		return FALSE;

	if (m_File.IsOpen())
		return FALSE;

	if (!m_File.Open(mpq_name))
		return FALSE;

	if (!Load()) {
		Clear();
		return FALSE;
	}

	return TRUE;
}

BOOL DMpq::DAccess::Close(VOID)
{
	if (!m_File.IsOpen())
		return FALSE;

	Clear();
	return TRUE;
}

BOOL DMpq::DAccess::Read(VPTR buf, UINT size)
{
	if (!buf || !size)
		return FALSE;

	if (!m_File.IsOpen() || !m_ReadAccess)
		return FALSE;

	if (m_File.Read(buf, size) != size)
		return FALSE;

	return TRUE;
}

BOOL DMpq::DAccess::Write(VCPTR buf, UINT size)
{
	if (!buf || !size)
		return FALSE;

	if (!m_File.IsOpen() || !m_WriteAccess)
		return FALSE;

	if (m_File.Write(buf, size) != size)
		return FALSE;

	return TRUE;
}

BOOL DMpq::DAccess::Seek(UINT pos)
{
	if (!m_File.IsOpen() || !m_WriteAccess && !m_ReadAccess)
		return FALSE;

	if (m_File.Seek(m_ArchiveOff + pos) == ERROR_POS)
		return FALSE;

	return TRUE;
}

HANDLE DMpq::DAccess::ShareHandle(VOID)
{
	if (!m_File.IsOpen())
		return NULL;

	UINT pos = m_File.Position();
	if (pos == ERROR_POS)
		return NULL;

	DFile file;
	if (!file.Open(m_File.GetName()))
		return NULL;

	if (file.Seek(pos) == ERROR_POS)
		return NULL;

	return file.Detach();
}

DMpq::DFileBuffer *DMpq::DAccess::GetBuffer(UINT block_idx)
{
	DBufferMap::iterator it = m_BufferMap.find(block_idx);
	if (it == m_BufferMap.end())
		return NULL;

	return it->second;
}

VOID DMpq::DAccess::SetBuffer(UINT block_idx, DFileBuffer *buf)
{
	DBufferMap::iterator it = m_BufferMap.find(block_idx);

	if (it != m_BufferMap.end()) {
		if (it->second == buf)
			return;
		delete it->second;
		it->second = buf;
	} else {
		m_BufferMap[block_idx] = buf;
	}
}

/************************************************************************/

BOOL DMpq::DAccess::Load(VOID)
{
	DAssert(m_File.IsOpen());

	BOOL found = FALSE;
	UINT arc_offset = 0U;
	HEADER header;

	// SC仅要求MPQ头大小不小于32字节即可
	while (m_File.Read(&header, SUPPORT_HEADER_SIZE) == SUPPORT_HEADER_SIZE) {
		if (header.identifier == MPQ_IDENTIFIER && header.header_size >= SUPPORT_HEADER_SIZE) {
			found = TRUE;
			break;
		}
		arc_offset += PHYSICAL_SECTOR_SIZE;
		if (m_File.Seek(arc_offset) == ERROR_POS)
			return FALSE;
	}

	if (!found)
		return FALSE;

	if (m_File.Seek(arc_offset) == ERROR_POS)
		return FALSE;

	UINT size = m_File.GetSize();
	if (size == ERROR_SIZE)
		return FALSE;

	if (arc_offset + header.archive_size > size)
		return FALSE;

	m_ArchiveOff = arc_offset;
	m_SectorShift = header.sector_shift + PHYSICAL_SECTOR_SHIFT;
	m_ReadAccess = TRUE;
	m_WriteAccess = FALSE;

	return TRUE;
}

VOID DMpq::DAccess::Clear(VOID)
{
	for (DBufferMap::iterator it = m_BufferMap.begin(); it != m_BufferMap.end(); ++it)
		delete it->second;
	m_BufferMap.clear();

	m_ReadAccess = FALSE;
	m_WriteAccess = FALSE;
	m_ArchiveOff = 0U;
	m_SectorShift = 0U;

	delete [] m_SectorBuffer;
	m_SectorBuffer = NULL;

	m_File.Close();
}

BUFPTR DMpq::DAccess::SectorBuffer(VOID)
{
	if (!m_SectorShift)
		return NULL;

	if (!m_SectorBuffer)
		m_SectorBuffer = new BYTE[1 << m_SectorShift];

	return m_SectorBuffer;
}

/************************************************************************/

DMpq::DSubFile::DSubFile() :
	m_FileSize(0U),
	m_Position(0U),
	m_FileBuffer(NULL)
{

}

DMpq::DSubFile::~DSubFile()
{

}

DMpq::DAccess *DMpq::DSubFile::GetAccess(VOID) CONST
{
	if (!m_FileBuffer)
		return NULL;

	return m_FileBuffer->GetAccess();
}

UINT DMpq::DSubFile::GetSize(VOID) CONST
{
	if (!m_FileBuffer)
		return ERROR_SIZE;

	return m_FileSize;
}

CONST DMpq::BLOCKENTRY *DMpq::DSubFile::GetBlock(VOID) CONST
{
	if (!m_FileBuffer)
		return NULL;

	return &m_FileBuffer->GetBlock();
}

BOOL DMpq::DSubFile::Create(DAccess *archive, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp)
{
	DAssert(archive && (block.flags & BLOCK_EXIST));
	DAssert(block_idx != HASH_ENTRY_INVALID && block_idx != HASH_ENTRY_EMPTY);
	DAssert(archive->Writable());
	DAssert(!archive->GetBuffer(block_idx));

	if (m_FileBuffer)
		return FALSE;

	DFileBuffer *buf = new DFileBuffer;
	if (!buf->Create(archive, block, key, comp)) {
		DAssert(FALSE);
		delete buf;
		return FALSE;
	}

	archive->SetBuffer(block_idx, buf);

	m_FileBuffer = buf;
	m_FileSize = block.file_size;
	m_Position = 0U;

	return TRUE;
}

BOOL DMpq::DSubFile::Open(DAccess *archive, UINT block_idx, CONST BLOCKENTRY &block, DWORD key)
{
	DAssert(archive && (block.flags & BLOCK_EXIST));
	DAssert(block_idx != HASH_ENTRY_INVALID && block_idx != HASH_ENTRY_EMPTY);
	DAssert(archive->Readable());

	if (m_FileBuffer)
		return FALSE;

	m_FileBuffer = archive->GetBuffer(block_idx);

	if (!m_FileBuffer) {

		DFileBuffer *buf = new DFileBuffer;
		if (!buf->Open(archive, block, key)) {
			DAssert(FALSE);
			delete buf;
			return FALSE;
		}

		archive->SetBuffer(block_idx, buf);
		m_FileBuffer = buf;
	}

	m_FileSize = block.file_size;
	m_Position = 0U;

	return TRUE;
}

BOOL DMpq::DSubFile::Close(VOID)
{
	if (!m_FileBuffer)
		return FALSE;

	m_FileBuffer = NULL;
	m_FileSize = 0U;
	m_Position = 0U;

	return TRUE;
}

UINT DMpq::DSubFile::Read(VPTR buf, UINT size)
{
	if (!buf || !m_FileBuffer)
		return 0U;

	if (!GetAccess()->Readable())
		return 0U;

	// Adjust size
	if (m_Position + size > m_FileSize)
		size = m_FileSize - m_Position;

	if (!size)
		return 0U;

	UINT sector_shift = m_FileBuffer->SectorShift();
	DAssert(sector_shift);

	UINT sector_beg = m_Position >> sector_shift;
	UINT sector_end = (m_Position + size - 1) >> sector_shift;

	// Overflow
	if (sector_end < sector_beg)
		sector_end = (m_FileSize - 1) >> sector_shift;

	BUFPTR data = static_cast<BUFPTR>(buf);
	UINT rd_size = 0U;
	UINT sector_offset = sector_beg << sector_shift;

	for (UINT i = sector_beg; i <= sector_end; i++) {

		UINT data_size;
		BUFCPTR sector_data = m_FileBuffer->GetSector(i, data_size);
		if (!sector_data)
			break;

		UINT sector_pos = m_Position + rd_size - sector_offset;
		if (sector_pos + size < data_size) {
			DMemCpy(data, sector_data + sector_pos, size);
			rd_size += size;
			break;
		}

		UINT copy_size = data_size - sector_pos;
		DMemCpy(data, sector_data + sector_pos, copy_size);
		sector_offset += data_size;
		size -= copy_size;
		rd_size += copy_size;
		data += copy_size;
	}

	m_Position += rd_size;
	return rd_size;
}

UINT DMpq::DSubFile::Write(VCPTR data, UINT size)
{
	if (!data || !size || !m_FileBuffer)
		return 0U;

	if (!GetAccess()->Writable())
		return 0U;

	UINT sector_shift = m_FileBuffer->SectorShift();

	// TODO: 目前因为没有写缓冲，所以必须写在sector边界上
	DAssert(!(m_Position & ((1 << sector_shift) - 1)));

	UINT sector_beg = m_Position >> sector_shift;
	UINT sector_end = (m_Position + size - 1) >> sector_shift;

	// Overflow
	if (sector_end < sector_beg)
		sector_end = (m_FileSize - 1) >> sector_shift;

	UINT wrt_size = 0U;
	BUFCPTR sector_data = static_cast<BUFCPTR>(data);

	for (UINT i = sector_beg; i <= sector_end; i++) {
		UINT data_size;
		if (!m_FileBuffer->SetSector(i, sector_data, size, data_size))
			break;
		sector_data += data_size;
		size -= data_size;
		wrt_size += data_size;
	}

	m_Position += wrt_size;
	return wrt_size;
}

UINT DMpq::DSubFile::Seek(INT offset, SEEK_MODE mode)
{
	if (!GetAccess())
		return ERROR_POS;

	switch (mode) {
	case SM_BEGIN:
		if (offset < 0 || (!GetAccess()->Writable() && offset > static_cast<INT>(m_FileSize)))
			return ERROR_POS;
		m_Position = offset;
		break;
	case SM_CURRENT:
		if (!GetAccess()->Writable() && (m_Position + offset > m_FileSize))
			return ERROR_POS;
		m_Position += offset;
		break;
	case SM_END:
		if ((!GetAccess()->Writable() && offset > 0) || -offset > static_cast<INT>(m_FileSize))
			return ERROR_POS;
		m_Position = m_FileSize + offset;
		break;
	default:
		return ERROR_POS;
	}

	return m_Position;
}

/************************************************************************/

DMpq::DFileBuffer::DFileBuffer() :
	m_Access(NULL),
	m_SectorNum(0U),
	m_Key(0UL),
	m_Compression(COMP_NONE),
	m_OffTable(NULL),
	m_CurCache(0),
	m_SwapBuffer(NULL)
{
	DVarClr(m_Block);
	DVarClr(m_Cache);
}

DMpq::DFileBuffer::~DFileBuffer()
{
	Clear();
}

DMpq::DAccess *DMpq::DFileBuffer::GetAccess(VOID) CONST
{
	return m_Access;
}

CONST DMpq::BLOCKENTRY &DMpq::DFileBuffer::GetBlock(VOID) CONST
{
	return m_Block;
}

UINT DMpq::DFileBuffer::SectorShift(VOID) CONST
{
	if (!m_Access)
		return 0U;

	return m_Access->SectorShift();
}

BOOL DMpq::DFileBuffer::Create(DAccess *archive, CONST BLOCKENTRY &block, DWORD key, BYTE comp)
{
	DAssert(archive && (block.flags & BLOCK_EXIST));
	DAssert(archive->Writable());
	DAssert(archive->SectorShift());

	if (m_Access)
		return FALSE;

	UINT sector_shift = archive->SectorShift();
	UINT sector_num = (block.file_size + (1 << sector_shift) - 1) >> sector_shift;

	if (sector_num) {
		if (!archive->Seek(block.offset))
			return FALSE;
		if (block.flags & BLOCK_COMP_MASK)
			m_OffTable = new DWORD[sector_num + 1];
		m_Block = block;
		m_Key = key;
	} else {
		m_Block.file_size = 0;
		m_Block.data_size = 0;
		m_Block.flags = BLOCK_EXIST;
		m_Block.offset = block.offset;
		m_Key = 0UL;
	}

	m_Compression = comp;
	m_Access = archive;
	m_SectorNum = sector_num;

	if (!Create()) {
		Clear();
		return FALSE;
	}

	return TRUE;
}

BOOL DMpq::DFileBuffer::Open(DAccess *archive, CONST BLOCKENTRY &block, DWORD key)
{
	DAssert(archive && (block.flags & BLOCK_EXIST));
	DAssert(archive->Readable());
	DAssert(archive->SectorShift());

	if (m_Access)
		return FALSE;

	UINT sector_shift = archive->SectorShift();
	UINT sector_num = (block.file_size + (1 << sector_shift) - 1) >> sector_shift;

	if (sector_num && (block.flags & BLOCK_COMP_MASK)) {

		if (!archive->Seek(block.offset))
			return FALSE;

		DWORD *off_table = new DWORD[sector_num + 1];
		UINT size = (sector_num + 1) * sizeof(DWORD);
		if (!archive->Read(off_table, size)) {
			delete [] off_table;
			return FALSE;
		}

		if (block.flags & BLOCK_ENCRYPT)
			DecryptData(off_table, size, key - 1);

		m_OffTable = off_table;
	}

	m_Access = archive;
	m_Block = block;
	m_Key = key;
	m_SectorNum = sector_num;

	return TRUE;
}

VOID DMpq::DFileBuffer::Clear(VOID)
{
	for (INT i = 0; i < MAX_CACHE_SECTOR; i++)
		delete [] m_Cache[i].data;
	DVarClr(m_Cache);

	delete [] m_SwapBuffer;
	m_SwapBuffer = NULL;

	delete [] m_OffTable;
	m_OffTable = NULL;

	m_Access = NULL;
	m_SectorNum = 0U;
	m_Key = 0UL;
	m_Compression = COMP_NONE;
	m_CurCache = 0;
	DVarClr(m_Block);
}

BUFCPTR DMpq::DFileBuffer::GetSector(UINT sector, UINT &size)
{
	DAssert(m_Access);

	if (sector >= m_SectorNum)
		return NULL;

	for (INT i = 0; i < MAX_CACHE_SECTOR; i++) {
		CACHESECTOR *cs = &m_Cache[i];
		if (cs->data && cs->sector == sector) {
			size = cs->size;
			return cs->data;
		}
	}

	UINT sector_size = 1 << SectorShift();
	if (sector == m_SectorNum - 1 && (m_Block.file_size & (sector_size - 1)))
		size = m_Block.file_size & (sector_size - 1);
	else
		size = sector_size;

	DAssert(size);
	BUFPTR data = new BYTE[size];
	if (!ReadSector(sector, data, size)) {
		delete [] data;
		return NULL;
	}

	DAssert(DBetween(m_CurCache, 0, MAX_CACHE_SECTOR));

	CACHESECTOR *cs = &m_Cache[m_CurCache];
	delete [] cs->data;
	cs->sector = sector;
	cs->size = size;
	cs->data = data;

	m_CurCache = (m_CurCache + 1) % MAX_CACHE_SECTOR;

	return data;
}

BOOL DMpq::DFileBuffer::SetSector(UINT sector, BUFCPTR buf, UINT buf_size, UINT &size)
{
	DAssert(m_Access);

	if (sector >= m_SectorNum || !buf || !buf_size)
		return FALSE;

	// 对于压缩文件来说，每个扇区只能写一次，并且必需按顺序写
	if (m_OffTable && (!m_OffTable[sector] || m_OffTable[sector + 1]))
		return FALSE;

	UINT sector_size = 1 << SectorShift();
	if (sector == m_SectorNum - 1 && (m_Block.file_size & (sector_size - 1)))
		size = m_Block.file_size & (sector_size - 1);
	else
		size = sector_size;

	if (buf_size < size)
		return FALSE;

	UINT data_size;
	if (!WriteSector(sector, buf, size, data_size))
		return FALSE;

	if (m_OffTable)
		m_OffTable[sector + 1] = m_OffTable[sector] + data_size;

	if (sector != m_SectorNum - 1)
		return TRUE;

	if (!m_OffTable) {
		m_Block.data_size = m_Block.file_size;
		return TRUE;
	}

	// 写回Offset Table
	if (!m_Access->Seek(m_Block.offset))
		return FALSE;

	UINT tab_size = (m_SectorNum + 1) * sizeof(DWORD);

	// 必要时加密
	if (m_Block.flags & BLOCK_ENCRYPT) {
		DArray<DWORD> off_table(m_SectorNum + 1);
		DMemCpy(off_table, m_OffTable, tab_size);
		EncryptData(off_table, tab_size, m_Key - 1);
		if (!m_Access->Write(off_table, tab_size))
			return FALSE;
	} else {
		if (!m_Access->Write(m_OffTable, tab_size))
			return FALSE;
	}

	m_Block.data_size = m_OffTable[m_SectorNum];

	return TRUE;
}

BOOL DMpq::DFileBuffer::Create(VOID)
{
	DAssert(m_Access);

	if (!m_SectorNum)
		return TRUE;

	UINT file_size = m_Block.file_size;
	DAssert(file_size);
	UINT sector_size = 1 << SectorShift();

	if (m_OffTable) {

		DAssert(m_Block.flags & BLOCK_COMP_MASK);

		UINT tab_size = (m_SectorNum + 1) * sizeof(DWORD);
		DMemClr(m_OffTable, tab_size);

		// 写空Offset Table占位
		if (!m_Access->Write(m_OffTable, tab_size)) 
			return FALSE;

		*m_OffTable = tab_size;
	}

	return TRUE;
}

BOOL DMpq::DFileBuffer::ReadSector(UINT sector, BUFPTR buf, UINT size)
{
	DAssert(sector < m_SectorNum && buf && size);

	if (m_Block.flags & BLOCK_COMP_MASK) {

		DAssert(m_OffTable);
		UINT offset = m_OffTable[sector];
		if (!m_Access->Seek(m_Block.offset + offset))
			return FALSE;

		if (m_OffTable[sector + 1] <= m_OffTable[sector])
			return FALSE;

		UINT data_size = m_OffTable[sector + 1] - offset;
		if (!data_size)
			return FALSE;

		if (data_size >= size) {
			if (!m_Access->Read(buf, size))
				return FALSE;
			if (m_Block.flags & BLOCK_ENCRYPT)
				DecryptData(buf, size, m_Key + sector);
		} else {
			DArray<BYTE> data(data_size);
			if (!m_Access->Read(data, data_size))
				return FALSE;
			if (m_Block.flags & BLOCK_ENCRYPT)
				DecryptData(data, data_size, m_Key + sector);
			if (!Decompress(data, data_size, buf, size))
				return FALSE;
		}

	} else {

		UINT offset = m_Block.offset + (sector << SectorShift());
		if (!m_Access->Seek(offset))
			return FALSE;

		if (!m_Access->Read(buf, size))
			return FALSE;

		if (m_Block.flags & BLOCK_ENCRYPT)
			DecryptData(buf, size, m_Key + sector);
	}

	return TRUE;
}

BOOL DMpq::DFileBuffer::WriteSector(UINT sector, BUFCPTR buf, UINT size, UINT &data_size)
{
	DAssert(sector < m_SectorNum && buf);
	DAssert(size <= (1U << SectorShift()));

	if (m_Block.flags & BLOCK_COMP_MASK) {

		DAssert(m_OffTable);

		BUFPTR sector_buf = m_Access->SectorBuffer();
		if (!sector_buf)
			return FALSE;

		UINT offset = m_OffTable[sector];
		if (!m_Access->Seek(m_Block.offset + offset))
			return FALSE;

		data_size = 1 << SectorShift();

		BYTE comp = m_Compression;

		// 由于ADPCM是有损压缩，为了避免WAVE文件头被其破坏，仅第一个段强制使用Implode无损压缩
		if (!sector && (comp & (COMP_ADPCM_MONO | COMP_ADPCM_STEREO)))
			comp = COMP_IMPLODE;

		if (Compress(comp, buf, size, sector_buf, data_size) && data_size < size) {
			DAssert(data_size);
			if (m_Block.flags & BLOCK_ENCRYPT)
				EncryptData(sector_buf, data_size, m_Key + sector);
			if (!m_Access->Write(sector_buf, data_size))
				return FALSE;
		} else {
			if (m_Block.flags & BLOCK_ENCRYPT) {
				DMemCpy(sector_buf, buf, size);
				EncryptData(sector_buf, size, m_Key + sector);
				if (!m_Access->Write(sector_buf, size))
					return FALSE;
			} else {
				if (!m_Access->Write(buf, size))
					return FALSE;
			}
			data_size = size;
		}

	} else {

		UINT offset = m_Block.offset + (sector << SectorShift());
		if (!m_Access->Seek(offset))
			return FALSE;

		if (m_Block.flags & BLOCK_ENCRYPT) {
			BUFPTR sector_buf = m_Access->SectorBuffer();
			if (!sector_buf)
				return FALSE;
			DMemCpy(sector_buf, buf, size);
			EncryptData(sector_buf, size, m_Key + sector);
			if (!m_Access->Write(sector_buf, size))
				return FALSE;
		} else {
			if (!m_Access->Write(buf, size))
				return FALSE;
		}

		data_size = size;
	}

	return TRUE;
}

INT DMpq::DFileBuffer::CheckCompression(BYTE comp)
{
	if (!comp)
		return 0;

	INT cnt = 0;
	BYTE test = comp;

	for (INT i = 0; i < DCount(VALID_COMP); i++) {
		BYTE code = comp & VALID_COMP[i];
		if (!code)
			continue;
		cnt++;
		test &= ~code;
	}

	// unrecognizable compression found
	if (test)
		return -1;

	if (cnt > 1 && !m_SwapBuffer)
		m_SwapBuffer = new BYTE[1 << SectorShift()];

	return cnt;
}

BOOL DMpq::DFileBuffer::Compress(BYTE comp, BUFCPTR src, UINT src_size, BUFPTR dest, UINT &dest_size)
{
	DAssert(src && src_size && dest && dest_size);
	DAssert(m_Block.flags & BLOCK_COMP_MASK);
	DAssert(dest_size <= (1U << SectorShift()));

	INT dict;

	// set dictionary size follow the storm.dll's way
	switch (src_size / 0x600) {
	case 0:
		dict = IMPLODE_DICT_1K;
		break;
	case 1:
		dict = IMPLODE_DICT_2K;
		break;
	default:
		dict = IMPLODE_DICT_4K;
		break;
	}

	if (m_Block.flags & BLOCK_IMPLODE)
		return implode(IMPLODE_BINARY, dict, src, src_size, dest, &dest_size);

	if (!(m_Block.flags & BLOCK_COMPRESS))
		return FALSE;

	INT cnt = CheckCompression(comp);
	if (cnt < 0)
		return FALSE;

	*dest++ = comp;
	dest_size--;

	if (!cnt) {
		if (dest_size < src_size)
			return FALSE;
		DMemCpy(dest, src, src_size);
		dest_size = src_size + 1;
		return TRUE;
	}

	// TODO:
	if (comp == COMP_IMPLODE) {
		if (!implode(IMPLODE_BINARY, dict, src, src_size, dest, &dest_size))
			return FALSE;
	} else {
		DAssert(FALSE);
	}

	dest_size++;
	return TRUE;
}

BOOL DMpq::DFileBuffer::Decompress(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size)
{
	DAssert(src && src_size && dest && dest_size);
	DAssert(m_Block.flags & BLOCK_COMP_MASK);
	DAssert(src_size < dest_size && dest_size <= (1U << SectorShift()));

	if (m_Block.flags & BLOCK_IMPLODE)
		return explode(src, src_size, dest, &dest_size);

	if (!(m_Block.flags & BLOCK_COMPRESS))
		return FALSE;

	BYTE comp = *src;
	INT cnt = CheckCompression(comp);
	if (cnt < 0)
		return FALSE;

	src++;
	src_size--;

	if (!cnt) {
		DMemCpy(dest, src, src_size);
		dest_size = src_size;
		return TRUE;
	}

	UINT size = dest_size;

	for (INT i = DCount(VALID_COMP) - 1; i >= 0; i--) {

		BYTE code = comp & VALID_COMP[i];
		if (!code)
			continue;

		BUFPTR work = (cnt-- & 1) ? dest : m_SwapBuffer;
		dest_size = size;

		switch (code) {
		case COMP_IMPLODE:
			if (!explode(src, src_size, work, &dest_size))
				return FALSE;
			break;
		case COMP_HUFFMAN:
			if (!huff_decode(src, src_size, work, &dest_size))
				return FALSE;
			break;
		case COMP_ADPCM_STEREO:
			if (!adpcm_decode(ADPCM_STEREO, src, src_size, work, &dest_size))
				return FALSE;
			break;
		case COMP_ADPCM_MONO:
			if (!adpcm_decode(ADPCM_MONO, src, src_size, work, &dest_size))
				return FALSE;
			break;
		case COMP_ADPCM_BETA_STEREO:
			if (!adpcm_beta_decode(ADPCM_STEREO, src, src_size, work, &dest_size))
				return FALSE;
			break;
		case COMP_ADPCM_BETA_MONO:
			if (!adpcm_beta_decode(ADPCM_MONO, src, src_size, work, &dest_size))
				return FALSE;
			break;
		}

		src = work;
		src_size = dest_size;
	}

	return TRUE;
}

/************************************************************************/
