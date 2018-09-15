#include "CMxFileAccess.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>
#include <dirent.h>
#include <sys/stat.h>
#include "MxError.h"
#include "MxCommon.h"
#include "MxFileSystem.h"
#include "MxInterrupt.h"

#ifndef HAVE_STRUCT_POLLFD
enum
{
    POLLERR=0x1,
    POLLHUP=0x2,
    POLLNVAL=0x4,
    POLLWRNORM=0x10,
    POLLWRBAND=0x20,
    POLLRDNORM=0x100,
    POLLRDBAND=0x200,
    POLLPRI=0x400,
};
#define POLLIN  (POLLRDNORM|POLLRDBAND)
#define POLLOUT (POLLWRNORM|POLLWRBAND)

struct pollfd
{
    int fd;
    unsigned events;
    unsigned revents;
};
#endif

#define msg_Info(p_this, ...)
#define msg_Err(p_this, ...)
#define msg_Warn(p_this, ...)
#define msg_Dbg(p_this, ...)



#define LC_MESSAGES_MASK 0
typedef void *locale_t;

int vlc_close (int fd)
{
    int ret;
#ifdef POSIX_CLOSE_RESTART
    ret = posix_close(fd, 0);
#else
    ret = close(fd);
    /* POSIX.2008 (and earlier) does not specify if the file descriptor is
     * closed on failure. Assume it is as on Linux and most other common OSes.
     * Also emulate the correct error code as per newer POSIX versions. */
    if (unlikely(ret != 0) && unlikely(errno == EINTR))
        errno = EINPROGRESS;
#endif
    assert(ret == 0 || errno != EBADF); /* something is corrupt? */
    return ret;
}

#if !defined (_WIN32) && !defined (__OS2__)
static bool IsRemote (int fd)
{
#if defined (HAVE_FSTATVFS) && defined (MNT_LOCAL)
    struct statvfs stf;
    
    if (fstatvfs (fd, &stf))
        return false;
    /* fstatvfs() is in POSIX, but MNT_LOCAL is not */
    return !(stf.f_flag & MNT_LOCAL);
    
#elif defined (HAVE_LINUX_MAGIC_H)
    struct statfs stf;
    
    if (fstatfs (fd, &stf))
        return false;
    
    switch ((unsigned long)stf.f_type)
    {
        case AFS_SUPER_MAGIC:
        case CODA_SUPER_MAGIC:
        case NCP_SUPER_MAGIC:
        case NFS_SUPER_MAGIC:
        case SMB_SUPER_MAGIC:
        case 0xFF534D42 /*CIFS_MAGIC_NUMBER*/:
            return true;
    }
    return false;
    
#else
    (void)fd;
    return false;
    
#endif
}
# define IsRemote(fd,path) IsRemote(fd)

#else /* _WIN32 || __OS2__ */
static bool IsRemote (const char *path)
{
# if !defined(__OS2__) && !VLC_WINSTORE_APP
    wchar_t *wpath = ToWide (path);
    bool is_remote = (wpath != NULL && PathIsNetworkPathW (wpath));
    free (wpath);
    return is_remote;
# else
    return (! strncmp(path, "\\\\", 2));
# endif
}
# define IsRemote(fd,path) IsRemote(path)
#endif

#ifndef HAVE_POSIX_FADVISE
# define posix_fadvise(fd, off, len, adv)
#endif

struct access_sys_t
{
    int fd;
    bool b_pace_control;
};

struct access_sys_t2
{
    char *base_uri;
    DIR *dir;
};

void DirClose(CMxStream *obj)
{
    CMxStream *access = (CMxStream *)obj;
    access_sys_t2 *sys = (access_sys_t2*)access->p_sys;
    
    free(sys->base_uri);
    closedir(sys->dir);
}

static inline locale_t newlocale(int mask, const char * locale, locale_t base)
{
    (void)mask; (void)locale; (void)base;
    return NULL;
}
static inline void freelocale(locale_t loc)
{
    (void)loc;
}

static const char *vlc_strerror_l(int errnum, const char *lname)
{
    int saved_errno = errno;
    locale_t loc = newlocale(LC_MESSAGES_MASK, lname, (locale_t)0);
    
    if (unlikely(loc == (locale_t)0))
    {
        if (errno == ENOENT) /* fallback to POSIX locale */
            loc = newlocale(LC_MESSAGES_MASK, "C", (locale_t)0);
        
        if (unlikely(loc == (locale_t)0))
        {
            assert(errno != EINVAL && errno != ENOENT);
            errno = saved_errno;
            return "Error message unavailable";
        }
        errno = saved_errno;
    }
    
    //meiyou shixian
    //const char *buf = strerror_l(errnum, loc);
    freelocale(loc);
    return /*buf*/"";
}

/**
 * Formats an error message in the current locale.
 * @param errnum error number (as in errno.h)
 * @return A string pointer, valid until the next call to a function of the
 * strerror() family in the same thread. This function cannot fail.
 */
const char *vlc_strerror(int errnum)
{
    /* We cannot simply use strerror() here, since it is not thread-safe. */
    return vlc_strerror_l(errnum, "");
}

/**
 * Formats an error message in the POSIX/C locale (i.e. American English).
 * @param errnum error number (as in errno.h)
 * @return A string pointer, valid until the next call to a function of the
 * strerror() family in the same thread. This function cannot fail.
 */
const char *vlc_strerror_c(int errnum)
{
    return vlc_strerror_l(errnum, "C");
}

char *vlc_uri_decode (char *str)
{
    char *in = str, *out = str;
    if (in == NULL)
        return NULL;
    
    char c;
    while ((c = *(in++)) != '\0')
    {
        if (c == '%')
        {
            char hex[3];
            
            if (!(hex[0] = *(in++)) || !(hex[1] = *(in++)))
                return NULL;
            hex[2] = '\0';
            *(out++) = strtoul (hex, NULL, 0x10);
        }
        else
            *(out++) = c;
    }
    *out = '\0';
    return str;
}

char *vlc_uri_decode_duplicate (const char *str)
{
    char *buf = strdup (str);
    if (vlc_uri_decode (buf) == NULL)
    {
        free(buf);
        buf = NULL;
    }
    return buf;
}

int CMxFileAccess::open(CMxStream *pStream)
{
    CMxStream *p_access = pStream;
    
    /* Open file */
    int fd = -1;
    
    if (!strcasecmp (p_access->psz_name, "fd"))
    {
        char *end;
        int oldfd = strtol (p_access->psz_location, &end, 10);
        
        if (*end == '\0')
            fd = mxDup (oldfd);
        else if (*end == '/' && end > p_access->psz_location)
        {
            char *name = vlc_uri_decode_duplicate (end - 1);
            if (name != NULL)
            {
                name[0] = '.';
                fd = mxOpenat (oldfd, name, O_RDONLY | O_NONBLOCK);
                free (name);
            }
        }
    }
    else
    {
        if (unlikely(p_access->psz_filepath == NULL))
            return MX_EGENERIC;
        fd = mxOpen (p_access->psz_filepath, O_RDONLY | O_NONBLOCK);
    }
    
    if (fd == -1)
    {
        msg_Err (p_access, "cannot open file %s (%s)",
                 p_access->psz_filepath ? p_access->psz_filepath
                 : p_access->psz_location,
                 vlc_strerror_c(errno));
        return MX_EGENERIC;
    }
    
    struct stat st;
    if (fstat (fd, &st))
    {
        msg_Err (p_access, "read error: %s", vlc_strerror_c(errno));
        //goto error;
        mxClose (fd);
        return MX_EGENERIC;
    }
    
#if O_NONBLOCK
    /* Force blocking mode back */
    fcntl (fd, F_SETFL, fcntl (fd, F_GETFL) & ~O_NONBLOCK);
#endif
    
    /* Directories can be opened and read from, but only readdir() knows
     * how to parse the data. The directory plugin will do it. */
    if (S_ISDIR (st.st_mode))
    {
#ifdef HAVE_FDOPENDIR
        DIR *p_dir = fdopendir(fd);
        if (!p_dir) {
            msg_Err (p_access, "fdopendir error: %s", vlc_strerror_c(errno));
            goto error;
        }
        return DirInit (p_access, p_dir);
#else
        msg_Dbg (p_access, "ignoring directory");
        //goto error;
        mxClose (fd);
        return MX_EGENERIC;
#endif
    }
    
    //access_sys_t *p_sys = vlc_obj_malloc(p_this, sizeof (*p_sys));
    access_sys_t *p_sys = (access_sys_t*)malloc(sizeof (*p_sys));
    if (unlikely(p_sys == NULL))
        goto error;
    //p_access->pf_read = Read;
    //p_access->pf_block = NULL;
    //p_access->pf_control = FileControl;
    p_access->p_sys = p_sys;
    p_sys->fd = fd;
    
    if (S_ISREG (st.st_mode) || S_ISBLK (st.st_mode))
    {
        //p_access->pf_seek = FileSeek;
        p_sys->b_pace_control = true;
        
        /* Demuxers will need the beginning of the file for probing. */
        posix_fadvise (fd, 0, 4096, POSIX_FADV_WILLNEED);
        /* In most cases, we only read the file once. */
        posix_fadvise (fd, 0, 0, POSIX_FADV_NOREUSE);
#ifdef F_NOCACHE
        fcntl (fd, F_NOCACHE, 0);
#endif
#ifdef F_RDAHEAD
        if (IsRemote(fd, p_access->psz_filepath))
            fcntl (fd, F_RDAHEAD, 0);
        else
            fcntl (fd, F_RDAHEAD, 1);
#endif
    }
    else
    {
        //p_access->pf_seek = NoSeek;
        p_sys->b_pace_control = strcasecmp (p_access->psz_name, "stream");
    }
    
    return MX_SUCCESS;
    
error:
    mxClose (fd);
    return MX_EGENERIC;
}

/*****************************************************************************
 * FileClose: close the target
 *****************************************************************************/
void FileClose (CMxStream * p_this)
{
    CMxStream     *p_access = (CMxStream*)p_this;
    
    /*if (p_access->pf_read == NULL)
    {
        DirClose (p_this);
        return;
    }*/
    
    access_sys_t *p_sys = (access_sys_t*)p_access->p_sys;
    
    vlc_close (p_sys->fd);
}

#include <sys/uio.h>
ssize_t vlc_readv_i11e(int fd, struct iovec *iov, int count)
{
    struct pollfd ufd;
    
    ufd.fd = fd;
    ufd.events = POLLIN;
    
    if (vlc_poll_i11e(&ufd, 1, -1) < 0)
        return -1;
    return readv(fd, iov, count);
}

ssize_t vlc_read_i11e(int fd, void *buf, size_t count)
{
    struct iovec iov = { .iov_base = buf, .iov_len = count };
    return vlc_readv_i11e(fd, &iov, 1);
}

int CMxFileAccess::read(CMxStream* pStream, void* pBuffer, int len)
{
    access_sys_t *p_sys = (access_sys_t*)pStream->p_sys;
    int fd = p_sys->fd;
    
    int val = vlc_read_i11e (fd, pBuffer, len);
    if (val < 0)
    {
        switch (errno)
        {
            case EINTR:
            case EAGAIN:
                return -1;
        }
        
        msg_Err (p_access, "read error: %s", vlc_strerror_c(errno));
        val = 0;
    }
    
    return val;
}

/*****************************************************************************
 * Seek: seek to a specific location in a file
 *****************************************************************************/
int CMxFileAccess::seek(CMxStream* pStream, uint64 pos)
{
    access_sys_t *sys = (access_sys_t*)pStream->p_sys;
    
    if (lseek(sys->fd, pos, SEEK_SET) == (off_t)-1)
        return MX_EGENERIC;
    return MX_SUCCESS;
}

int CMxFileAccess::noSeek(CMxStream* pStream, uint64 pos)
{
    /* vlc_assert_unreachable(); ?? */
    (void) pStream; (void) pos;
    return MX_EGENERIC;
}

/*****************************************************************************
 * Control:
 *****************************************************************************/
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

/*static inline int64_t var_InheritInteger( CMxStream *obj, const char *name )
{
    vlc_value_t val;
    
    if( var_Inherit( obj, name, VLC_VAR_INTEGER, &val ) )
        val.i_int = 0;
    return val.i_int;
}*/

int control(CMxStream* pStream,int i_query, va_list args)
{
    access_sys_t *p_sys = (access_sys_t*)pStream->p_sys;
    bool    *pb_bool;
    int64_t *pi_64;
    
    switch( i_query )
    {
        case STREAM_CAN_SEEK:
        case STREAM_CAN_FASTSEEK:
            pb_bool = va_arg( args, bool * );
            //*pb_bool = (p_access->pf_seek != NoSeek);
            break;
            
        case STREAM_CAN_PAUSE:
        case STREAM_CAN_CONTROL_PACE:
            pb_bool = va_arg( args, bool * );
            *pb_bool = p_sys->b_pace_control;
            break;
            
        case STREAM_GET_SIZE:
        {
            struct stat st;
            
            if (fstat (p_sys->fd, &st) || !S_ISREG(st.st_mode))
                return MX_EGENERIC;
            *va_arg( args, uint64_t * ) = st.st_size;
            break;
        }
            
        case STREAM_GET_PTS_DELAY:
            pi_64 = va_arg( args, int64_t * );
            /*if (IsRemote (p_sys->fd, p_access->psz_filepath))
                *pi_64 = var_InheritInteger (pStream, "network-caching");
            else
                *pi_64 = var_InheritInteger (pStream, "file-caching");
            *pi_64 *= 1000;*/
            break;
            
        case STREAM_SET_PAUSE_STATE:
            /* Nothing to do */
            break;
            
        default:
            return MX_EGENERIC;
            
    }
    return MX_SUCCESS;
}
