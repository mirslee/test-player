
#ifndef __MXMEMORY_H__
#define __MXMEMORY_H__


MXCORE_API void* mx_pool_alloc(int size);
MXCORE_API void  mx_pool_free(void * ptr);

MXCORE_API void* mx_calloc(size_t _NumOfElements, size_t _SizeOfElements, int _Alignment = 16);
MXCORE_API void* mx_malloc(size_t _Size, int _Alignment = 16);
MXCORE_API void* mx_mallocz(size_t _Size, int _Alignment = 16);
MXCORE_API void* mx_realloc(void * _Memory, size_t _Size, int _Alignment = 16);
MXCORE_API void  mx_free(void * memblock);
MXCORE_API void  mx_freep(void ** memblock);
MXCORE_API void  mx_memcheck(void * memblock);

MXCORE_API void* mxAlloc(int size);
MXCORE_API void  mxFree(void *ptr);


#endif//__MXMEMORY_H__
