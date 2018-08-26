#pragma once

template<class TYPE, class ARG_TYPE = const TYPE&>
class CMxArray
{
public:
	// Construction
	CMxArray();
	// Attributes
	int GetSize() const;
	int GetCount() const;
	bool IsEmpty() const;
	int GetUpperBound() const;
	void SetMaxSize(int nGrowBy) { m_nGrowBy = nGrowBy; }
	void SetSize(int nNewSize, int nGrowBy = -1);

	// Operations
	// Clean up
	void FreeExtra();
	void RemoveAll();

	// Accessing elements
	const TYPE& GetAt(int nIndex) const;
	TYPE& GetAt(int nIndex);
	void SetAt(int nIndex, ARG_TYPE newElement);
	const TYPE& ElementAt(int nIndex) const;
	TYPE& ElementAt(int nIndex);

	// Direct Access to the element data (may return NULL)
	const TYPE* GetData() const;
	TYPE* GetData();

	// Potentially growing the array
	void SetAtGrow(int nIndex, ARG_TYPE newElement);
	int Add(ARG_TYPE newElement);
	int Append(const CMxArray& src);
	void Copy(const CMxArray& src);

	// overloaded operator helpers
	const TYPE& operator[](int nIndex) const;
	TYPE& operator[](int nIndex);

	// Operations that move elements around
	void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CMxArray* pNewArray);

	// Implementation
protected:
	TYPE* m_pData;   // the actual array of data
	int m_nSize;     // # of elements (upperBound - 1)
	int m_nMaxSize;  // max allocated
	int m_nGrowBy;   // grow amount

public:
	~CMxArray();
	//void Serialize(CVxArchive&);
};

/////////////////////////////////////////////////////////////////////////////
// CVxArray<TYPE, ARG_TYPE> inline functions

template<class TYPE, class ARG_TYPE>
inline int CMxArray<TYPE, ARG_TYPE>::GetSize() const
{
	return m_nSize;
}
template<class TYPE, class ARG_TYPE>
inline int CMxArray<TYPE, ARG_TYPE>::GetCount() const
{
	return m_nSize;
}
template<class TYPE, class ARG_TYPE>
inline bool CMxArray<TYPE, ARG_TYPE>::IsEmpty() const
{
	return m_nSize == 0;
}
template<class TYPE, class ARG_TYPE>
inline int CMxArray<TYPE, ARG_TYPE>::GetUpperBound() const
{
	return m_nSize - 1;
}
template<class TYPE, class ARG_TYPE>
inline void CMxArray<TYPE, ARG_TYPE>::RemoveAll()
{
	SetSize(0, -1);
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxArray<TYPE, ARG_TYPE>::GetAt(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_nSize)
		return m_pData[nIndex];
	_vxThrowInvalidArgException();
	return m_pData[0];
}

template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < m_nSize)
		return m_pData[nIndex];
	_vxThrowInvalidArgException();
	return m_pData[0];
}
template<class TYPE, class ARG_TYPE>
inline void CMxArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement)
{
	if (nIndex >= 0 && nIndex < m_nSize)
		m_pData[nIndex] = newElement;
	else
		_vxThrowInvalidArgException();
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxArray<TYPE, ARG_TYPE>::ElementAt(int nIndex) const
{
	if (nIndex >= 0 && nIndex < m_nSize)
		return m_pData[nIndex];
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxArray<TYPE, ARG_TYPE>::ElementAt(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_nSize)
		return m_pData[nIndex];
	//_vxThrowInvalidArgException();
	return m_pData[0];
}
template<class TYPE, class ARG_TYPE>
inline const TYPE* CMxArray<TYPE, ARG_TYPE>::GetData() const
{
	return (const TYPE*)m_pData;
}
template<class TYPE, class ARG_TYPE>
inline TYPE* CMxArray<TYPE, ARG_TYPE>::GetData()
{
	return (TYPE*)m_pData;
}
template<class TYPE, class ARG_TYPE>
inline int CMxArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
{
	int nIndex = m_nSize;
	SetAtGrow(nIndex, newElement);
	return nIndex;
}
template<class TYPE, class ARG_TYPE>
inline const TYPE& CMxArray<TYPE, ARG_TYPE>::operator[](int nIndex) const
{
	return GetAt(nIndex);
}
template<class TYPE, class ARG_TYPE>
inline TYPE& CMxArray<TYPE, ARG_TYPE>::operator[](int nIndex)
{
	return ElementAt(nIndex);
}

/////////////////////////////////////////////////////////////////////////////
// CVxArray<TYPE, ARG_TYPE> out-of-line functions

template<class TYPE, class ARG_TYPE>
CMxArray<TYPE, ARG_TYPE>::CMxArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CMxArray<TYPE, ARG_TYPE>::~CMxArray()
{
	if (m_pData != NULL)
	{
		for (int i = 0; i < m_nSize; i++)
			(m_pData + i)->~TYPE();
		//_vxfree(m_pData);
	}
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy)
{
	if (nGrowBy >= 0)
		m_nGrowBy = nGrowBy;  // set new size

	if (nNewSize == 0)
	{
		// shrink to nothing
		if (m_pData != NULL)
		{
			for (int i = 0; i < m_nSize; i++)
				(m_pData + i)->~TYPE();
			//_vxfree(m_pData);
			free(m_pData);
			m_pData = NULL;
		}
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create buffer big enough to hold number of requested elements or
		// m_nGrowBy elements, whichever is larger.
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
		int nAllocSize = __max(nNewSize, m_nGrowBy);
		//m_pData = (TYPE*)_vxmallocz(nAllocSize * sizeof(TYPE));
		m_pData = (TYPE*)malloc(nAllocSize * sizeof(TYPE));
		memset(m_pData, 0, sizeof(nAllocSize * sizeof(TYPE)));
		for (int i = 0; i < nNewSize; i++)
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
			::new((void*)(m_pData + i)) TYPE;
#pragma pop_macro("new")
#else
			::new((void*)(m_pData + i)) TYPE;
#endif
		m_nSize = nNewSize;
		m_nMaxSize = nAllocSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		// it fits
		if (nNewSize > m_nSize)
		{
			// initialize the new elements
			memset((void*)(m_pData + m_nSize), 0, (size_t)(nNewSize - m_nSize) * sizeof(TYPE));
			for (int i = 0; i < nNewSize - m_nSize; i++)
			{
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
				::new((void*)(m_pData + m_nSize + i)) TYPE;
#pragma pop_macro("new")
#else
				::new((void*)(m_pData + m_nSize + i)) TYPE;
#endif
			}
		}
		else if (m_nSize > nNewSize)
		{
			// destroy the old elements
			for (int i = 0; i < m_nSize - nNewSize; i++)
				(m_pData + nNewSize + i)->~TYPE();
		}
		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = m_nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush



		//TYPE* pNewData = (TYPE*)_vxmalloc((size_t)nNewMax * sizeof(TYPE));
		TYPE* pNewData = (TYPE*)malloc((size_t)nNewMax * sizeof(TYPE));

		// copy new data from old
		memcpy(pNewData, m_pData, (size_t)m_nSize * sizeof(TYPE));
		memset((void*)(pNewData + m_nSize), 0, (size_t)(nNewSize - m_nSize) * sizeof(TYPE));
		for (int i = 0; i < nNewSize - m_nSize; i++)
		{
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
			::new((void*)(pNewData + m_nSize + i)) TYPE;
#pragma pop_macro("new")
#else
			::new((void*)(pNewData + m_nSize + i)) TYPE;
#endif
		}
		// get rid of old stuff (note: no destructors called)
		//_vxfree(m_pData);
		free(m_pData);
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

template<class TYPE, class ARG_TYPE>
int CMxArray<TYPE, ARG_TYPE>::Append(const CMxArray& src)
{
	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	vxCopyElements<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
	return nOldSize;
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::Copy(const CMxArray& src)
{
	if (this != &src)
	{
		SetSize(src.m_nSize);
		vxCopyElements<TYPE>(m_pData, src.m_pData, src.m_nSize);
	}
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::FreeExtra()
{


	if (m_nSize != m_nMaxSize)
	{
		TYPE* pNewData = NULL;
		if (m_nSize != 0)
		{
			//pNewData = (TYPE*)_vxmalloc(m_nSize * sizeof(TYPE));
			pNewData = (TYPE*)malloc(m_nSize * sizeof(TYPE));
			// copy new data from old
			memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
		}

		// get rid of old stuff (note: no destructors called)
		_vxfree(m_pData);
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement)
{
	if (nIndex >= m_nSize)
		SetSize(nIndex + 1, -1);
	m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount /*=1*/)
{
	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount, -1);   // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount, -1);  // grow it to new size
										// destroy intial data before copying over it
		for (int i = 0; i < nCount; i++)
			(m_pData + nOldSize + i)->~TYPE();
		// shift old data up to fill gap
		memmove(m_pData + nIndex + nCount, m_pData + nIndex, (nOldSize - nIndex) * sizeof(TYPE));

		// re-init slots we copied from
		memset((void*)(m_pData + nIndex), 0, (size_t)nCount * sizeof(TYPE));
		for (int i = 0; i < nCount; i++)
#ifdef _WIN32			
#pragma push_macro("new")
#undef new
			::new((void*)(m_pData + nIndex + i)) TYPE;
#pragma pop_macro("new")
#else
			::new((void*)(m_pData + nIndex + i)) TYPE;
#endif
	}

	// insert new value in the gap
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount)
{
	int nUpperBound = nIndex + nCount;

	// just remove a range
	int nMoveCount = m_nSize - (nUpperBound);
	for (int i = 0; i < nCount; i++)
		(m_pData + nIndex + i)->~TYPE();
	if (nMoveCount)
	{
		memmove(m_pData + nIndex, m_pData + nUpperBound, (size_t)nMoveCount * sizeof(TYPE));
	}
	m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CMxArray* pNewArray)
{
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

/*template<class TYPE, class ARG_TYPE>
void CMxArray<TYPE, ARG_TYPE>::Serialize(CVxArchive& ar)
{
	if (ar.IsStoring())
	{
		ar.WriteCount(m_nSize);
	}
	else
	{
		DWORD_PTR nOldSize = ar.ReadCount();
		SetSize(nOldSize, -1);
	}
	vxSerializeElements<TYPE>(ar, m_pData, m_nSize);
}*/