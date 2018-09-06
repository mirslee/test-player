#ifndef __CMXSTRING_H__
#define __CMXSTRING_H__

class CMxCharArray {
public:
    CMxCharArray() {__data = nullptr;}
    vritual ~CMxCharArray() {delete [] __data;}
    
    char* data();
private:
    char* __data;
}

class CMxWCharArray {
public:
    CMxWCharArray() {__data = nullptr;}
    vritual !CMxWCharArray() {delete [] __data;}
    
    wchar_t* data();
private:
    wchar_t* __data;
}

class CMxString {
public:
public:
    CMxString();
    CMxString(char *sz, char* encode = "utf-8"); /*utf-8  or  gbk*/
    
    char* cString();
    wchar_t* wcString();
    
private:
    void __c2w();
    void __w2c();
private:
    char* __cdata;
    wchar_t* __wdata;
};

#endif //__CMXSTRING_H__
