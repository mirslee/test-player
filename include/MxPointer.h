#pragma once


template<class T> class CMxObjectPointer {
public:
	/*Ĭ�Ϲ���*/
    CMxObjectPointer() {
        this->pointer = nullptr;
    }

	/*����������*/
	CMxObjectPointer(T* pointer) {
		this->pointer = pointer;
	}

	/*��������*/
	CMxObjectPointer(const CMxObjectPointer<T> & other) {
		if (other)
			other->addRef();
		pointer = other;
	}

	/*����ָ�븳ֵ����ָͨ�룬*/
	T* operator=(T* pointer) {
		if (pointer)
			pointer->addRef();
		if (this->pointer)
			pointer->unRef();
		this->pointer = pointer;
		return this->pointer;
	}

	/*��ֵ����*/
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

	

	/*ָ���Ƿ�Ϊ��*/
	bool operator!() const {
		return pointer == nullptr;
	}

	/*�жϹ���ָ�����ָͨ���Ƿ����*/
	bool operator==(T* pointer) const {
		return this->pointer == pointer;
	}

	/*    ǿ��ת��        */
	operator T*() const {
		return (T*)pointer;
	}

	/*    ȡָ���ַ��&pointer     */
	T** operator&() {
		return &this->pointer;
	}

	/*    ָ����ã� pointer->     */
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





