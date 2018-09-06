#ifndef __CMXSTRING_H__
#define __CMXSTRING_H__

class CMxCharArray {
public:
    CMxCharArray(char* str) {__data = str;}
	virtual ~CMxCharArray() {delete [] __data;}
    
    char* data();
private:
    char* __data;
};

class CMxWCharArray {
public:
    CMxWCharArray(wchar_t* wstr) {__data = wstr;}
    virtual ~CMxWCharArray() {delete [] __data;}
    
    wchar_t* data();
private:
    wchar_t* __data;
};

class CMxString {
public:
public:
    CMxString();
    CMxString(const char *sz, const char* encode); /*utf-8  or  gbk*/
	CMxString(const char *sz);
    
	CMxCharArray cStr();
	CMxWCharArray wcStr();
private:
    char* __pData;
};

#endif //__CMXSTRING_H__
