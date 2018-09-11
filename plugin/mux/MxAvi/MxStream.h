#ifndef __MXSTREAM_H__
#define __MXSTREAM_H__

typedef struct input_thread_t input_thread_t;
struct stream_t
{
    
    /* Module properties for stream filter */
    //module_t    *p_module;
    
    char        *psz_name;
    char        *psz_url; /**< Full URL or MRL (can be NULL) */
    const char  *psz_location; /**< Location (URL with the scheme stripped) */
    char        *psz_filepath; /**< Local file path (if applicable) */
    bool         b_preparsing; /**< True if this access is used to preparse */
    
    /* Stream source for stream filter */
    stream_t *p_source;
    
    /**
     * Read data.
     *
     * Callback to read data from the stream into a caller-supplied buffer.
     *
     * This may be NULL if the stream is actually a directory rather than a
     * byte stream, or if \ref stream_t.pf_block is non-NULL.
     *
     * \param buf buffer to read data into
     * \param len buffer length (in bytes)
     *
     * \retval -1 no data available yet
     * \retval 0 end of stream (incl. fatal error)
     * \retval positive number of bytes read (no more than len)
     */
    ssize_t     (*pf_read)(stream_t *, void *buf, size_t len);
    
    /**
     * Read data block.
     *
     * Callback to read a block of data. The data is read into a block of
     * memory allocated by the stream. For some streams, data can be read more
     * efficiently in block of a certain size, and/or using a custom allocator
     * for buffers. In such case, this callback should be provided instead of
     * \ref stream_t.pf_read; otherwise, this should be NULL.
     *
     * \param eof storage space for end-of-stream flag [OUT]
     * (*eof is always false when invoking pf_block(); pf_block() should set
     *  *eof to true if it detects the end of the stream)
     *
     * \return a data block,
     * NULL if no data available yet, on error and at end-of-stream
     */
    block_t    *(*pf_block)(stream_t *, bool *eof);
    
    /**
     * Read directory.
     *
     * Callback to fill an item node from a directory
     * (see doc/browsing.txt for details).
     *
     * NULL if the stream is not a directory.
     */
    int         (*pf_readdir)(stream_t *, input_item_node_t *);
    
    /**
     * Seek.
     *
     * Callback to set the stream pointer (in bytes from start).
     *
     * May be NULL if seeking is not supported.
     */
    int         (*pf_seek)(stream_t *, uint64_t);
    
    /**
     * Stream control.
     *
     * Cannot be NULL.
     *
     * \see stream_query_e
     */
    int         (*pf_control)(stream_t *, int i_query, va_list);
    
    /**
     * Private data pointer
     */
    void *p_sys;
    
    /* Weak link to parent input */
    input_thread_t *p_input;
};

enum stream_query_e
{
    /* capabilities */
    STREAM_CAN_SEEK,            /**< arg1= bool *   res=cannot fail*/
    STREAM_CAN_FASTSEEK,        /**< arg1= bool *   res=cannot fail*/
    STREAM_CAN_PAUSE,           /**< arg1= bool *   res=cannot fail*/
    STREAM_CAN_CONTROL_PACE,    /**< arg1= bool *   res=cannot fail*/
    /* */
    STREAM_GET_SIZE=6,          /**< arg1= uint64_t *     res=can fail */
    STREAM_IS_DIRECTORY,        /**< res=can fail */
    
    /* */
    STREAM_GET_PTS_DELAY = 0x101,/**< arg1= int64_t* res=cannot fail */
    STREAM_GET_TITLE_INFO, /**< arg1=input_title_t*** arg2=int* res=can fail */
    STREAM_GET_TITLE,       /**< arg1=unsigned * res=can fail */
    STREAM_GET_SEEKPOINT,   /**< arg1=unsigned * res=can fail */
    STREAM_GET_META,        /**< arg1= vlc_meta_t *       res=can fail */
    STREAM_GET_CONTENT_TYPE,    /**< arg1= char **         res=can fail */
    STREAM_GET_SIGNAL,      /**< arg1=double *pf_quality, arg2=double *pf_strength   res=can fail */
    STREAM_GET_TAGS,        /**< arg1=const block_t ** res=can fail */
    
    STREAM_SET_PAUSE_STATE = 0x200, /**< arg1= bool        res=can fail */
    STREAM_SET_TITLE,       /**< arg1= int          res=can fail */
    STREAM_SET_SEEKPOINT,   /**< arg1= int          res=can fail */
    
    /* XXX only data read through vlc_stream_Read/Block will be recorded */
    STREAM_SET_RECORD_STATE,     /**< arg1=bool, arg2=const char *psz_ext (if arg1 is true)  res=can fail */
    
    STREAM_SET_PRIVATE_ID_STATE = 0x1000, /* arg1= int i_private_data, bool b_selected    res=can fail */
    STREAM_SET_PRIVATE_ID_CA,             /* arg1= int i_program_number, uint16_t i_vpid, uint16_t i_apid1, uint16_t i_apid2, uint16_t i_apid3, uint8_t i_length, uint8_t *p_data */
    STREAM_GET_PRIVATE_ID_STATE,          /* arg1=int i_private_data arg2=bool *          res=can fail */
};

#endif //__MXSTREAM_H__
