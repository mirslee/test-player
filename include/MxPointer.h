#pragma once


template<class T> class CMxObjectPointer {
public:
	/*默认构造*/
    CMxObjectPointer() {
        this->pointer = nullptr;
    }

	/*带参数构造*/
	CMxObjectPointer(T* pointer) {
		this->pointer = pointer;
	}

	/*拷贝构造*/
	CMxObjectPointer(const CMxObjectPointer<T> & other) {
		if (other)
			other->addRef();
		pointer = other;
	}

	/*共享指针赋值给普通指针，*/
	T* operator=(T* pointer) {
		if (pointer)
			pointer->addRef();
		if (this->pointer)
			pointer->unRef();
		this->pointer = pointer;
		return this->pointer;
	}

	/*赋值构造*/
	T* operator=(const CMxObjectPointer<T> & other) {
		if (other)
			other->addRef();
		if (pointer)
			pointer->unRef();
		this->pointer = pointer;
		return this->pointer;
	}

	~CMxObjectPointer() {
		this->pointer->unRef();
	}

	/*指针是否为空*/
	bool operator!() const {
		return pointer == nullptr;
	}

	/*判断共享指针和普通指针是否相等*/
	bool operator==(T* pointer) const {
		return this->pointer == pointer;
	}

	/*    强制转换        */
	operator T*() const {
		return (T*)pointer;
	}

	/*    取指针地址，&pointer     */
	T** operator&() {
		return &this->pointer;
	}

	/*    指针调用， pointer->     */
    T* operator->() const {
		return pointer;
	}
private:
	T* pointer;
};

template<class T> class CMxSharedPointer {
public:
    CMxSharedPointer() {
        this->pointer = nullptr;
    }
    ~CMxSharedPointer() {
        if (pointer) {
            delete pointer;
            pointer = nullptr;
        }
    }
    CMxSharedPointer(T* pointer) {
        this->pointer = pointer;
    }
    T* operator=(T* pointer) {
        if (pointer) {
            delete pointer;
            pointer = nullptr;
        }
        
        this->pointer = pointer;
        
    }
private:
    T* operator=(const CMxSharedPointer<T> & other) {
        if (other)
            other->addRef();
        if (pointer)
            pointer->unRef();
        this->pointer = pointer;
        return this->pointer;
    }
private:
    T* pointer;
};

template <class T>
class CMemArrayPtr
{
private:
	T* p;
public:
	CMemArrayPtr()
	{
		p = nullptr;
	}
	CMemArrayPtr(T* lp)
	{
		p = lp;
	}
	~CMemArrayPtr()
	{
		if (p) delete[] p;
	}

	operator T*() const
	{
		return (T*)p;
	}
	T& operator*() const
	{
		assert(p != nullptr);
		return *p;
	}
	T** operator&()
	{
		return &p;
	}

	T* operator=(T* lp)
	{
		if (p) delete[] p;
		return (p = lp);
	}

	bool operator!() const
	{
		return (p == NULL);
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}

	bool operator!=(T* pT) const
	{
		return p != pT;
	}

	T* operator->() const
	{
		assert(p != nullptr);
		return p;
	}
};

template <class T>
class CMemClassPtr
{
private:
	T* p;
public:
	CMemClassPtr()
	{
		p = nullptr;
	}
	CMemClassPtr(T* lp)
	{
		p = lp;
	}
	~CMemClassPtr()
	{
		if (p) delete p;
	}

	operator T*() const
	{
		return (T*)p;
	}
	T& operator*() const
	{
		assert(p != nullptr);
		return *p;
	}
	T** operator&()
	{
		return &p;
	}

	T* operator=(T* lp)
	{
		if (p) delete p;
		return (p = lp);
	}

	bool operator!() const
	{
		return (p == NULL);
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}

	bool operator!=(T* pT) const
	{
		return p != pT;
	}

	T* operator->() const
	{
		ASSERT(p != NULL);
		return p;
	}
};


template <class T>
class CVxMallocPtr
{
private:
	T* p;
public:
	CVxMallocPtr()
	{
		p = NULL;
	}
	CVxMallocPtr(T* lp)
	{
		p = lp;
	}
	~CVxMallocPtr()
	{
		if (p) mx_free(p);
	}

	operator T*() const
	{
		return (T*)p;
	}
	T** operator&()
	{
		return &p;
	}

	T* operator=(T* lp)
	{
		if (p) mx_free(p);
		return (p = lp);
	}

	bool operator!() const
	{
		return (p == nullptr);
	}
	bool operator==(T* pT) const
	{
		return p == pT;
	}

	bool operator!=(T* pT) const
	{
		return p != pT;
	}

	T* operator->() const
	{
		assert(p != nullptr);
		return p;
	}
};



