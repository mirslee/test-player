#include "stdafx.h"
#include "MxSystem.h"
#include "MxObject.h"
#include "MxMediaDefine.h"
#include "MxSynchronize.h"
#include "MxMemory.h"

struct _FileHash
{
	_FileHash();
	~_FileHash();

	CMxMutex m_csLock;

	struct DATA
	{
		MxObject* vxobj;
	};

	struct ITEM
	{
		DWORD	refcode;	//	非唯一CODE,HPOS才是唯一CODE
		int		ref;
		char	name[MX_MAXPATH];
		DATA	data;
	};

	typedef mxuvoidptr	HPOS;	//	HASHCODE,唯一而且不变的引用

	HPOS	GetHeadPosition();
	ITEM&	GetNext(HPOS&);
	ITEM&	GetAt(HPOS);

	HPOS	Find(const char* filename);
	HPOS	Create(const char* filename);
	void	Remove(HPOS);

	HPOS	Ref(const char* filename);
	LONG	Unref(HPOS);
	void	Empty();

	//	使用者重写这两个函数
	void	_construct(ITEM&);	//	refcode和name已经填充好
	void	_destroy(ITEM&);
private:
	DWORD _refcode(const char* filename);
	void _copyToLow(const char* filename, char* refname, int n);

	struct ITEMX
	{
		ITEMX*	next;
		ITEMX*	prev;
		ITEM	item;
	};

	struct SLOTX
	{
		SLOTX*	next;
		ITEMX	data[1];
	};

	ITEMX*	m_free;
	ITEMX*	m_head;
	SLOTX*	m_slot;
	int		m_growup;
	int		m_count;

	HPOS	AddHead(ITEMX*);
	void	FreeItem(ITEMX*);
	ITEMX*	NewItem();
	void	Free(SLOTX*);
	void	Alloc();

	HPOS	FindLow(const char* lowname);
	HPOS	CreateLow(const char* lowname);
public:
#ifdef _DUMPNOW
	void Dump();
	int successTimes;
	int failedTimes;
#endif//!_DUMPNOW
};

#pragma warning(disable: 4311 4312)

mxinline _FileHash::HPOS _FileHash::Find(const char* filename)
{
	char lowname[MX_MAXPATH] = { 0 };
	_copyToLow(filename, lowname, MX_MAXPATH);
	return FindLow(lowname);
}

mxinline _FileHash::HPOS _FileHash::FindLow(const char* lowname)
{
	ITEMX* head = m_head;
	if (head)
	{
		DWORD refcode = _refcode(lowname);
		while (head)
		{
			if (head->item.refcode == refcode)
			{
				if (strcmp(lowname, (const char*)head->item.name) == 0)
#ifndef _DUMPNOW
				{
					return 	(HPOS)head;
				}
#else
				{
					successTimes++;
					return	(HPOS)head;
				}
				else
				{
					failedTimes++;
				}
#endif//!_DUMPNOW
			}
			head = head->next;
		}
	}
	return 0;
}

mxinline _FileHash::HPOS _FileHash::Create(const char* filename)
{
	char lowname[MX_MAXPATH] = { 0 };
	_copyToLow(filename, lowname, MX_MAXPATH);
	return CreateLow(lowname);
}

mxinline _FileHash::HPOS _FileHash::CreateLow(const char* lowname)
{
	DWORD refcode = _refcode(lowname);
	ITEMX* newitem = NewItem();

	strncpy((char*)(newitem->item.name), lowname, MX_MAXPATH);
	newitem->item.refcode = refcode;
	newitem->item.ref = 1;
	_construct(newitem->item);

	return AddHead(newitem);
}

mxinline _FileHash::HPOS _FileHash::AddHead(ITEMX* newitem)
{
	newitem->next = m_head;
	newitem->prev = 0;
	if (m_head)
	{
		m_head->prev = newitem;
	}
	m_head = newitem;
	m_count++;
	return (HPOS)newitem;
}

mxinline void _FileHash::Remove(HPOS pos)
{
	ITEMX* xpos = (ITEMX*)pos;
	if (xpos->prev)
	{
		xpos->prev->next = xpos->next;
	}
	else
	{
		m_head = xpos->next;
	}
	if (xpos->next)
	{
		xpos->next->prev = xpos->prev;
	}

	_destroy(xpos->item);
	FreeItem(xpos);
	m_count--;
}

mxinline _FileHash::HPOS _FileHash::Ref(const char* filename)
{
	char lowname[MX_MAXPATH] = { 0 };
	_copyToLow(filename, lowname, MX_MAXPATH);

	HPOS pos = FindLow(lowname);
	if (pos == 0)
	{
		return CreateLow(lowname);
	}
	else
	{
		((ITEMX*)pos)->item.ref++;
		return pos;
	}
}

mxinline LONG _FileHash::Unref(HPOS pos)
{
	if (pos && --((ITEMX*)pos)->item.ref == 0)
	{
		Remove(pos);
		return 1;
	}
	return 0;
}

mxinline void _FileHash::Empty()
{
	ITEMX* head = m_head;
	while (head)
	{
		_destroy(head->item);
		head = head->next;
	}
	m_free = 0;
	m_head = 0;

	SLOTX* slot = m_slot;
	while (slot)
	{
		SLOTX* curr = slot;
		slot = slot->next;
		Free(curr);
	}
	m_slot = 0;
}

mxinline _FileHash::ITEMX* _FileHash::NewItem()
{
	if (m_free == 0)
	{
		Alloc();
	}

	ITEMX* newItem = m_free;
	m_free = m_free->next;
	return newItem;
}

mxinline void _FileHash::FreeItem(ITEMX* item)
{
	item->next = m_free;
	m_free = item;
}

mxinline _FileHash::HPOS _FileHash::GetHeadPosition()
{
	return (HPOS)m_head;
}

mxinline _FileHash::ITEM& _FileHash::GetNext(HPOS& pos)
{
	HPOS cur = pos;
	pos = (HPOS)((ITEMX*)pos)->next;
	return ((ITEMX*)cur)->item;
}

mxinline _FileHash::ITEM& _FileHash::GetAt(HPOS pos)
{
	return ((ITEMX*)pos)->item;
}

mxinline void _FileHash::_construct(ITEM& item)
{
	memset(&item.data, 0, sizeof(item.data));
}

mxinline void _FileHash::_destroy(ITEM& item)
{

}

mxinline DWORD _FileHash::_refcode(const char* filename)
{
	//	本函数控制查找的效率,从文件名可以得到唯一的REFCODE,而同一refcode可能对应多个文件名
	const char *eos = filename;
	while (*eos++);
	int l = (int)(eos - filename - 1);

	BYTE code[4];
	if (l <= 4)
	{
		for (int i = 0; i < l; i++)
		{
			code[i] = (BYTE)filename[i];
		}
	}
	else
	{
		int dot = l;
		while (dot--)
		{
			if (filename[dot] == '.')
			{
				if ((dot < 3) || (dot >= l - 1))
				{
					*(DWORD*)code = *(DWORD*)filename;
				}
				else
				{
					BYTE code[4];
					code[0] = filename[dot - 3];
					code[1] = filename[dot - 2];
					code[2] = filename[dot - 1];
					code[3] = filename[dot + 1];
					return *(DWORD*)code;
				}
			}
		}
		*(DWORD*)code = *(DWORD*)filename;
	}
	return *(DWORD*)code;
}

mxinline void _FileHash::_copyToLow(const char* filename, char* refname, int n)
{
	const char*	fsrc = filename;
	char* fdst = refname;
	if (fsrc)
	{
		while (*fsrc && (--n))
		{
			if (*fsrc >= 'A' && *fsrc <= 'Z')
			{
				*fdst++ = *fsrc++ + ('a' - 'A');
			}
			else if (*fsrc == '\\')
			{
				*fdst++ = '/';
				*fsrc++;
			}
			else
			{
				*fdst++ = *fsrc++;
			}
		}
	}
	*fdst++ = 0;
}

mxinline _FileHash::_FileHash()
{
	m_free = 0;
	m_head = 0;
	m_slot = 0;
	m_growup = 64;
	m_count = 0;
#ifdef _DUMPNOW
	successTimes = 0;
	failedTimes = 0;
#endif//_DUMPNOW
	mxMutexInit(&m_csLock);
}

mxinline _FileHash::~_FileHash()
{
	Empty();
	mxMutexDestroy(&m_csLock);
}

mxinline void _FileHash::Free(SLOTX* slot)
{
	mx_free(slot);
}

mxinline void _FileHash::Alloc()
{
	int items = min(max(m_growup, 4), 256);
	SLOTX* slot = (SLOTX*)mx_mallocz(sizeof(SLOTX) + sizeof(ITEMX) * (items - 1));
	slot->next = m_slot;
	m_slot = slot;
	for (int i = 0; i < items - 1; i++)
	{
		slot->data[i].next = &slot->data[i + 1];
	}
	slot->data[items - 1].next = m_free;
	m_free = &slot->data[0];
}

#ifdef _DUMPNOW
void _FileHash::Dump()
{
	SLOTX* slot = m_slot;
	int slots = 0;
	while (slot)
	{
		slots++;
		slot = slot->next;
	}

	ITEMX* free = m_free;
	int frees = 0;
	while (free)
	{
		frees++;
		free = free->next;
	}

	TRACE("\n========================================================\n");
	TRACE("Count : %d, Free : %d, Slot : %d\n", m_count, frees, slots);

	HPOS pos = GetHeadPosition();
	while (pos)
	{
		ITEM& item = GetNext(pos);
		TRACE("    ");
		TRACE(item.name);
		TRACE("    REF:%d( %08x )\n", item.ref, item.refcode);
	}

	if (successTimes + failedTimes > 0)
	{
		TRACE("Search Hit : %1.1f%%", float(successTimes * 100) / float(successTimes + failedTimes));
	}
}
#endif//!_DUMPNOW



_FileHash g_filehash;

MXCORE_API mxuvoidptr mxRefFile(const char* file)
{
	CMxMutexLocker lock(&g_filehash.m_csLock);
	return g_filehash.Ref(file);
}

MXCORE_API void mxUnrefFile(mxuvoidptr fid)
{
	CMxMutexLocker lock(&g_filehash.m_csLock);
	g_filehash.Unref(fid);
}
