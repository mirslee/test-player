
#ifndef CMXFILEACCESS_H
#define CMXFILEACCESS_H

#include "MxTypes.h"
#include <stdarg.h>

class CMxStream {
    
public:
    char        *psz_name;
    char        *psz_url; /**< Full URL or MRL (can be NULL) */
    const char  *psz_location; /**< Location (URL with the scheme stripped) */
    char        *psz_filepath; /**< Local file path (if applicable) */
    bool         b_preparsing; /**< True if this access is used to preparse */
    
    int read(CMxStream* pStream, void* pData, int len);
    int seek(CMxStream* pStream, uint64 len);
    void *p_sys;
    
    /*
     module_t    *p_module;
     stream_t *p_source;
     block_t    *(*pf_block)(stream_t *, bool *eof);
    int         (*pf_readdir)(stream_t *, input_item_node_t *);
    int         (*pf_control)(stream_t *, int i_query, va_list);
    input_thread_t *p_input;*/
};

class CMxFileAccess {
    
public:
    int open(CMxStream *pStream);
    int read(CMxStream* pStream, void* pBuffer, int len);
    int seek(CMxStream* pStream, uint64 pos);
    int noSeek(CMxStream* pStream, uint64 pos);
    int control(CMxStream* pStream,int i_query, va_list args);
};

#endif /* CMXFILEACCESS_H */
