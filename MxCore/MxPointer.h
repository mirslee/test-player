#pragma once


template<class T> class CMxSharedPointer {
public:
	/*默认构造*/
    CMxSharedPointer() {
        pointer = nullptr;
    }

	/*带参数构造*/
	CMxSharedPointer(T* pointer) {
		this->pointer = pointer;
	}

	/*拷贝构造*/
	CMxSharedPointer(const CMxSharedPointer<T> & other) {
		if (other)
			other->addRef();
		pointer = other;
	}

	/*赋值构造*/
	CMxSharedPointer<T>& operator=(const CMxSharedPointer<T> & other) {
		if (other)
			other->addRef();
		if (pointer)
			pointer->unRef();
		this->pointer = pointer;
		return this->pointer;
	}

	~CMxSharedPointer() {
		return this->unRef();
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
		return &pointer;
	}

	/*    指针调用， pointer->     */
	T* operator->() const {
		return pointer;
		std::
	}

private:
	T* pointer;
};






