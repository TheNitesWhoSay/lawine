/************************************************************************/
/* File Name   : mpq.hpp                                                */
/* Creator     : ax.minaduki@gmail.com                                  */
/* Create Time : Jan 4th, 2009                                          */
/* Module      : Lawine library                                         */
/* Descript    : DMpq class declaration                                 */
/************************************************************************/

#ifndef __SD_LAWINE_DATA_MPQ_HPP__
#define __SD_LAWINE_DATA_MPQ_HPP__

/************************************************************************/

#include <vector>
#include <list>
#include <map>
#include <common.h>
#include <ref.hpp>
#include <file.hpp>
#include <mutex.hpp>

/************************************************************************/

class DMpq {

public:

	DMpq();
	~DMpq();

	BOOL CreateArchive(STRCPTR mpq_name, UINT &hash_num);
	BOOL OpenArchive(STRCPTR mpq_name);
	BOOL CloseArchive(VOID);

	BOOL FileExist(STRCPTR file_name);
	BOOL FileExist(HANDLE file);
	HANDLE OpenFile(STRCPTR file_name);
	BOOL CloseFile(HANDLE file);
	HANDLE OpenHandle(STRCPTR file_name);

	BOOL AddFile(STRCPTR file_name, STRCPTR real_path, BOOL compress, BOOL encrypt);
	BOOL NewFile(STRCPTR file_name, BUFCPTR file_data, UINT size, BOOL compress, BOOL encrypt);
	BOOL DelFile(STRCPTR file_name);

	static UINT GetFileSize(HANDLE file);
	static UINT ReadFile(HANDLE file, VPTR data, UINT size);
	static UINT SeekFile(HANDLE file, INT offset, SEEK_MODE mode = SM_BEGIN);

	static BOOL Initialize(VOID);
	static VOID Exit(VOID);
	static BOOL SetBasePath(STRCPTR path);
	static STRCPTR GetBasePath(VOID);
	static VOID SetLocale(LCID locale);
	static LCID GetLocale(VOID);

protected:

	enum HASH_TYPE {
		HASH_TABLE_ENTRY,
		HASH_NAME_LOW,
		HASH_NAME_HIGH,
		HASH_FILE_KEY,
		HASH_TYPE_NUM,
	};

	static CONST INT CRYPT_TABLE_INDEX = HASH_TYPE_NUM;
	static CONST INT HASH_TABLE_NUM = CRYPT_TABLE_INDEX + 1;

	struct HEADER {
		DWORD identifier;			// Must be ASCII "MPQ\x1a".
		DWORD header_size;			// Size of the this header structure.
		DWORD archive_size;			// Size of the whole archive, including the header.
		WORD version;				// MoPaQ format version.
		WORD sector_shift;			// Power of two exponent specifying the number of 512-byte disk sectors in each logical sector.
		DWORD hash_table_offset;	// Offset to the beginning of the hash table, relative to the beginning of the archive.
		DWORD block_table_offset;	// Offset to the beginning of the block table, relative to the beginning of the archive.
		DWORD hash_num;				// Number of entries in the hash table. Must be a power of two, and must be less than 2^16.
		DWORD block_num;			// Number of entries in the block table.
	};

	struct BLOCKENTRY {
		DWORD offset;				// Offset of the beginning of the block, relative to the beginning of the archive.
		DWORD data_size;			// Size of the block data in the archive.
		DWORD file_size;			// Size of the file data stored in the block. Only valid if the block is a file.
		DWORD flags;				// Bit mask of the flags for the block.
	};

	struct HASHENTRY {
		DWORD hash_low;				// The hash of the file path, using method A.
		DWORD hash_high;			// The hash of the file path, using method B.
		LANGID language;			// The language of the file.
		WORD platform;				// The platform the file is used for. 0 indicates the default platform.
		DWORD block_index;			// The index into the block table of the file.
	};

	class DAccess;
	class DSubFile;
	class DFileBuffer;

	typedef std::vector<BLOCKENTRY>			DBlockTable;
	typedef std::list<DSubFile *>			DFileList;
	typedef std::map<UINT, DFileBuffer *>	DBufferMap;

	BOOL Create(STRCPTR mpq_name, UINT hash_num);
	BOOL Load(STRCPTR mpq_name);
	VOID Clear(VOID);
	BOOL AddFile(STRCPTR file_name, BOOL compress, BOOL encrypt, DFile &file);
	BOOL AddFile(DSubFile *sub, HASHENTRY *hash, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp, DFile &file);
	BOOL NewFile(DSubFile *sub, HASHENTRY *hash, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp, BUFCPTR file_data);
	HASHENTRY *PrepareAdd(STRCPTR file_name, UINT file_size, BOOL compress, BOOL encrypt, UINT &block_idx, BLOCKENTRY &block, DWORD &key, BYTE &compression);
	BOOL Writeback(DSubFile *sub, HASHENTRY *hash, UINT block_idx);
	BOOL Writeback(UINT hash_table_offset);
	HASHENTRY *Lookup(STRCPTR file_name);
	HASHENTRY *AllocHash(STRCPTR file_name);
	UINT AllocBlock(UINT file_size, BOOL compress, BOOL encrypt, BLOCKENTRY &block);
	UINT GetEndOfFileData(VOID);

	static DWORD CalcFileKey(STRCPTR path_name, CONST BLOCKENTRY &block);
	static DWORD HashString(STRCPTR str, INT hash_type);
	static VOID EncryptData(VPTR buf, UINT size, DWORD key);
	static VOID DecryptData(VPTR buf, UINT size, DWORD key);

	UINT			m_HashNum;
	DFileList		m_FileList;
	DBlockTable		m_BlockTable;
	DAccess			*m_Access;
	HASHENTRY		*m_HashTable;

	static LCID		s_Locale;
	static DString	s_BashPath;
	static DWORD	s_HashTable[HASH_TABLE_NUM][0x100];

};

/************************************************************************/

class DMpq::DAccess {

public:

	DAccess();
	~DAccess();

	BOOL Readable(VOID) CONST;
	BOOL Writable(VOID) CONST;
	UINT SectorShift(VOID) CONST;

	BOOL Create(STRCPTR mpq_name);
	BOOL Open(STRCPTR mpq_name);
	BOOL Close(VOID);

	BOOL Read(VPTR buf, UINT size);
	BOOL Write(VCPTR buf, UINT size);
	BOOL Seek(UINT pos);

	HANDLE ShareHandle(VOID);

	DFileBuffer *GetBuffer(UINT block_idx);
	VOID SetBuffer(UINT block_idx, DFileBuffer *buf);

	BUFPTR SectorBuffer(VOID);

protected:

	BOOL Load(VOID);
	VOID Clear(VOID);

	DFile		m_File;
	BOOL		m_ReadAccess;
	BOOL		m_WriteAccess;
	UINT		m_ArchiveOff;
	UINT		m_SectorShift;
	BUFPTR		m_SectorBuffer;
	DBufferMap	m_BufferMap;

};

/************************************************************************/

class DMpq::DSubFile {

public:

	DSubFile();
	~DSubFile();

	DAccess *GetAccess(VOID) CONST;
	UINT GetSize(VOID) CONST;
	CONST BLOCKENTRY *GetBlock(VOID) CONST;
	BOOL Create(DAccess *archive, UINT block_idx, CONST BLOCKENTRY &block, DWORD key, BYTE comp);
	BOOL Open(DAccess *archive, UINT block_idx, CONST BLOCKENTRY &block, DWORD key);
	BOOL Close(VOID);
	UINT Read(VPTR buf, UINT size);
	UINT Write(VCPTR data, UINT size);
	UINT Seek(INT offset, SEEK_MODE mode);

protected:

	UINT		m_FileSize;
	UINT		m_Position;
	DFileBuffer	*m_FileBuffer;

};

/************************************************************************/

class DMpq::DFileBuffer {

public:

	DFileBuffer();
	~DFileBuffer();

	DAccess *GetAccess(VOID) CONST;
	CONST BLOCKENTRY &GetBlock(VOID) CONST;
	UINT SectorShift(VOID) CONST;
	BOOL Create(DAccess *archive, CONST BLOCKENTRY &block, DWORD key, BYTE comp);
	BOOL Open(DAccess *archive, CONST BLOCKENTRY &block, DWORD key);
	VOID Clear(VOID);
	BUFCPTR GetSector(UINT sector, UINT &size);
	BOOL SetSector(UINT sector, BUFCPTR buf, UINT buf_size, UINT &size);

protected:

	static CONST INT MAX_CACHE_SECTOR = 16;

	struct CACHESECTOR {
		UINT		sector;
		UINT		size;
		BUFPTR		data;
	};

	BOOL Create(VOID);
	BOOL ReadSector(UINT sector, BUFPTR buf, UINT size);
	BOOL WriteSector(UINT sector, BUFCPTR buf, UINT size, UINT &data_size);
	INT CheckCompression(BYTE comp);
	BOOL Compress(BYTE comp, BUFCPTR src, UINT src_size, BUFPTR dest, UINT &dest_size);
	BOOL Decompress(BUFCPTR src, UINT src_size, BUFPTR dest, UINT dest_size);

	DAccess		*m_Access;
	UINT		m_SectorNum;
	DWORD		m_Key;
	BYTE		m_Compression;
	BLOCKENTRY	m_Block;
	DWORD		*m_OffTable;
	BUFPTR		m_SwapBuffer;

	INT			m_CurCache;
	CACHESECTOR	m_Cache[MAX_CACHE_SECTOR];

};

/************************************************************************/

#endif	/* __SD_LAWINE_DATA_ARCHIVE_HPP__ */
