#pragma once


template<class T> class CMxSharedPointer {
public:
	/*Ĭ�Ϲ���*/
    CMxSharedPointer() {
        pointer = nullptr;
    }

	/*����������*/
	CMxSharedPointer(T* pointer) {
		this->pointer = pointer;
	}

	/*��������*/
	CMxSharedPointer(const CMxSharedPointer<T> & other) {
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
	T* operator=(const CMxSharedPointer<T> & other) {
		if (other)
			other->addRef();
		if (pointer)
			pointer->unRef();
		this->pointer = pointer;
		return this->pointer;
	}

	~CMxSharedPointer() {
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
		return &pointer;
	}

	/*    ָ����ã� pointer->     */
	T* operator->() const {
		return pointer;
	}

private:
	T* pointer;
};






