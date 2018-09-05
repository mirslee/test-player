#ifndef __CMXLIST_H__
#define __CMXLIST_H__

struct CMxPlex     // warning variable length structure
{
	CMxPlex* pNext;
#ifndef _WIN64
#if (_AFX_PACKING >= 8)
	DWORD dwReserved[1];    // align on 8 byte boundary
#endif
#endif
							// BYTE data[maxNum*elementSize];

	void* data() { return this + 1; }

	static CMxPlex* Create(CMxPlex*& head, unsigned int nMax, unsigned int cbElement)
	{
		CMxPlex* p = (CMxPlex*)mx_mallocz(sizeof(CMxPlex) + nMax * cbElement);
		// may throw exception
		p->pNext = head;
		head = p;  // change head (adds in reverse order for simplicity)
		return p;
	}

	void FreeDataChain()     // free this one and links
	{
		CMxPlex* p = this;
		while (p != NULL)
		{
			BYTE* bytes = (BYTE*)p;
			CMxPlex* pNext = p->pNext;
			mx_free(bytes);
			p = pNext;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// CMxList<TYPE, ARG_TYPE>

struct __MXPOSITION {};
typedef __MXPOSITION* MXPOSITION;

/////////////////////////////////////////////////////////////////////////////
// CMxList<TYPE, ARG_TYPE>

template<class TYPE, class ARG_TYPE = const TYPE&>
class CMxList
{
protected:
	struct CMxNode
	{
		CMxNode* pNext;
		CMxNode* pPrev;
		TYPE data;
	};
public:
	// Construction
	/* explicit */ CMxList(int nBlockSize = 10);

	// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	int GetSize() const;
	bool IsEmpty() const;

	// peek at head or tail
	TYPE& GetHead();
	const TYPE& GetHead() const;
	TYPE& GetTail();
	const TYPE& GetTail() const;

	// Operations
	// get head or tail (and remove it) - don't call on empty list !
	TYPE RemoveHead();
	TYPE RemoveTail();

	// add before head or after tail
	MXPOSITION AddHead(ARG_TYPE newElement);
	MXPOSITION AddTail(ARG_TYPE newElement);

	// add another list of elements before head or after tail
	void AddHead(CMxList* pNewList);
	void AddTail(CMxList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	MXPOSITION GetHeadPosition() const;
	MXPOSITION GetTailPosition() const;
	TYPE& GetNext(MXPOSITION& rPosition); // return *Position++
	const TYPE& GetNext(MXPOSITION& rPosition) const; // return *Position++
	TYPE& GetPrev(MXPOSITION& rPosition); // return *Position--
	const TYPE& GetPrev(MXPOSITION& rPosition) const; // return *Position--

													  // getting/modifying an element at a given position
	TYPE& GetAt(MXPOSITION position);
	const TYPE& GetAt(MXPOSITION position) const;
	void SetAt(MXPOSITION pos, ARG_TYPE newElement);
	void RemoveAt(MXPOSITION position);

	// inserting before or after a given position
	MXPOSITION InsertBefore(MXPOSITION position, ARG_TYPE newElement);
	MXPOSITION InsertAfter(MXPOSITION position, ARG_TYPE newElement);

	// helper functions (note: O(n) speed)
	MXPOSITION Find(ARG_TYPE searchValue, MXPOSITION startAfter = NULL) const;
	// defaults to starting at the HEAD, return NULL if not found
	MXPOSITION FindIndex(int nIndex) const;
	// get the 'nIndex'th element (may return NULL)

	// Implementation
protected:
	CMxNode* m_pNodeHead;
	CMxNode* m_pNodeTail;
	int m_nCount;
	CMxNode* m_pNodeFree;
	struct CMxPlex* m_pBlocks;
	int m_nBlockSize;

	CMxNode* NewNode(CMxNode*, CMxNode*);
	void FreeNode(CMxNode*);

public:
	~CMxList();
	//void Serialize(CVxArchive&);
	/*
	#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
	#endif
	*/
};

/////////////////////////////////////////////////////////////////////////////
// CMxList<TYPE, ARG_TYPE> inline functions

template<class TYPE, class ARG_TYPE>
inline int CMxList<TYPE, ARG_TYPE>::GetCount() const
{
	return m_nCount;
}
template<class TYPE, class ARG_TYPE>
inline int CMxList<TYPE, ARG_TYPE>::GetSize() const
{
	return m_nCount;
}
template<class TYPE, class ARG_TYPE>
inline bool CMxList<TYPE, ARG_TYPE>::IsEmpty() const
{
	return m_nCount == 0;
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxList<TYPE, ARG_TYPE>::GetHead()
{
	return m_pNodeHead->data;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxList<TYPE, ARG_TYPE>::GetHead() const
{
	return m_pNodeHead->data;
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxList<TYPE, ARG_TYPE>::GetTail()
{
	return m_pNodeTail->data;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxList<TYPE, ARG_TYPE>::GetTail() const
{
	return m_pNodeTail->data;
}
template<class TYPE, class ARG_TYPE>
inline MXPOSITION CMxList<TYPE, ARG_TYPE>::GetHeadPosition() const
{
	return (MXPOSITION)m_pNodeHead;
}
template<class TYPE, class ARG_TYPE>
inline MXPOSITION CMxList<TYPE, ARG_TYPE>::GetTailPosition() const
{
	return (MXPOSITION)m_pNodeTail;
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxList<TYPE, ARG_TYPE>::GetNext(MXPOSITION& rPosition) // return *Position++
{
	CMxNode* pNode = (CMxNode*)rPosition;
	rPosition = (MXPOSITION)pNode->pNext;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxList<TYPE, ARG_TYPE>::GetNext(MXPOSITION& rPosition) const // return *Position++
{
	CMxNode* pNode = (CMxNode*)rPosition;
	rPosition = (MXPOSITION)pNode->pNext;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxList<TYPE, ARG_TYPE>::GetPrev(MXPOSITION& rPosition) // return *Position--
{
	CMxNode* pNode = (CMxNode*)rPosition;
	rPosition = (MXPOSITION)pNode->pPrev;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxList<TYPE, ARG_TYPE>::GetPrev(MXPOSITION& rPosition) const // return *Position--
{
	CMxNode* pNode = (CMxNode*)rPosition;
	rPosition = (MXPOSITION)pNode->pPrev;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxList<TYPE, ARG_TYPE>::GetAt(MXPOSITION position)
{
	CMxNode* pNode = (CMxNode*)position;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxList<TYPE, ARG_TYPE>::GetAt(MXPOSITION position) const
{
	CMxNode* pNode = (CMxNode*)position;
	return pNode->data;
}
template<class TYPE, class ARG_TYPE>
inline void CMxList<TYPE, ARG_TYPE>::SetAt(MXPOSITION pos, ARG_TYPE newElement)
{
	CMxNode* pNode = (CMxNode*)pos;
	pNode->data = newElement;
}

template<class TYPE, class ARG_TYPE>
CMxList<TYPE, ARG_TYPE>::CMxList(int nBlockSize)
{
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::RemoveAll()
{
	// destroy elements
	CMxNode* pNode;
	for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
		pNode->data.~TYPE();

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CMxList<TYPE, ARG_TYPE>::~CMxList()
{
	RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
//
// Implementation note: CMxNode's are stored in CMxPlex blocks and
//  chained together. Free blocks are maintained in a singly linked list
//  using the 'pNext' member of CMxNode with 'm_pNodeFree' as the head.
//  Used blocks are maintained in a doubly linked list using both 'pNext'
//  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
//   as the head/tail.
//
// We never free a CMxPlex block unless the List is destroyed or RemoveAll()
//  is used - so the total number of CMxPlex blocks may grow large depending
//  on the maximum past size of the list.
//
/*
#ifndef _WIN32
inline void * operator new(size_t, void *_Where)
{return (_Where); }
#endif
*/
template<class TYPE, class ARG_TYPE>
typename CMxList<TYPE, ARG_TYPE>::CMxNode*
CMxList<TYPE, ARG_TYPE>::NewNode(CMxNode* pPrev, CMxNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CMxPlex* pNewBlock = CMxPlex::Create(m_pBlocks, m_nBlockSize,
			sizeof(CMxNode));

		// chain them into free list
		CMxNode* pNode = (CMxNode*)pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize - 1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	CMxNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
	::new((void*)(&pNode->data)) TYPE;
#pragma pop_macro("new")
#else
	::new((void*)(&pNode->data)) TYPE;
#endif

	return pNode;
}

template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::FreeNode(CMxNode* pNode)
{
	pNode->data.~TYPE();
	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	// if no more elements, cleanup completely
	if (m_nCount == 0)
		RemoveAll();
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{


	CMxNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (MXPOSITION)pNewNode;
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
	CMxNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = (TYPE)newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (MXPOSITION)pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::AddHead(CMxList* pNewList)
{
	// add a list of same elements to head (maintain order)
	MXPOSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::AddTail(CMxList* pNewList)
{
	// add a list of same elements
	MXPOSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

template<class TYPE, class ARG_TYPE>
TYPE CMxList<TYPE, ARG_TYPE>::RemoveHead()
{
	CMxNode* pOldNode = m_pNodeHead;
	TYPE returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

template<class TYPE, class ARG_TYPE>
TYPE CMxList<TYPE, ARG_TYPE>::RemoveTail()
{

	CMxNode* pOldNode = m_pNodeTail;
	TYPE returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::InsertBefore(MXPOSITION position, ARG_TYPE newElement)
{


	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

									// Insert it before position
	CMxNode* pOldNode = (CMxNode*)position;
	CMxNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (MXPOSITION)pNewNode;
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::InsertAfter(MXPOSITION position, ARG_TYPE newElement)
{


	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

									// Insert it before position
	CMxNode* pOldNode = (CMxNode*)position;
	CMxNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (MXPOSITION)pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::RemoveAt(MXPOSITION position)
{


	CMxNode* pOldNode = (CMxNode*)position;
	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{


	if (nIndex >= m_nCount || nIndex < 0)
		return NULL;  // went too far

	CMxNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (MXPOSITION)pNode;
}

template<class TYPE, class ARG_TYPE>
MXPOSITION CMxList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, MXPOSITION startAfter) const
{


	CMxNode* pNode = (CMxNode*)startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (vxCompareElements<TYPE>(&pNode->data, &searchValue))
			return (MXPOSITION)pNode;
	return NULL;
}

/*template<class TYPE, class ARG_TYPE>
void CMxList<TYPE, ARG_TYPE>::Serialize(CVxArchive& ar)
{
	if (ar.IsStoring())
	{
		ar.WriteCount(m_nCount);
		for (CMxNode* pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
		{
			TYPE* pData;
			pData = reinterpret_cast< TYPE* >(&reinterpret_cast< int& >(static_cast< TYPE& >(pNode->data)));
			vxSerializeElements<TYPE>(ar, pData, 1);
		}
	}
	else
	{
		DWORD_PTR nNewCount = ar.ReadCount();
		while (nNewCount--)
		{
			TYPE newData[1];
			vxSerializeElements<TYPE>(ar, newData, 1);
			AddTail(newData[0]);
		}
	}
}*/


#endif //__CMXLIST_H__
