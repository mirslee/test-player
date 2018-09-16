#include "stdafx.h"
#include "MxFileSystem.h"

#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

FILE *mxFopen (const char *filename, const char *mode)
{
    int rwflags = 0, oflags = 0;
    
    for (const char *ptr = mode; *ptr; ptr++)
    {
        switch (*ptr)
        {
            case 'r':
                rwflags = O_RDONLY;
                break;
                
            case 'a':
                rwflags = O_WRONLY;
                oflags |= O_CREAT | O_APPEND;
                break;
                
            case 'w':
                rwflags = O_WRONLY;
                oflags |= O_CREAT | O_TRUNC;
                break;
                
            case 'x':
                oflags |= O_EXCL;
                break;
                
            case '+':
                rwflags = O_RDWR;
                break;
                
#ifdef O_BINARY
            case 'b':
                oflags = (oflags & ~O_TEXT) | O_BINARY;
                break;
                
            case 't':
                oflags = (oflags & ~O_BINARY) | O_TEXT;
                break;
#endif
        }
    }
    
    int fd = mxOpen (filename, rwflags | oflags, 0666);
    if (fd == -1)
        return NULL;
    
    FILE *stream = fdopen (fd, mode);
    if (stream == NULL)
        mxClose (fd);
    
    return stream;
}


static int dummy_select( const char *str )
{
    (void)str;
    return 1;
}

int mxLoaddir( DIR *dir, char ***namelist,
                int (*select)( const char * ),
                int (*compar)( const char **, const char ** ) )
{
    assert (dir);
    
    if (select == NULL)
        select = dummy_select;
    
    char **tab = NULL;
    unsigned num = 0;
    
    rewinddir (dir);
    
    for (unsigned size = 0;;)
    {
        errno = 0;
        const char *entry = mxReaddir (dir);
        if (entry == NULL)
        {
            if (errno)
                goto error;
            break;
        }
        
        if (!select (entry))
            continue;
        
        if (num >= size)
        {
            size = size ? (2 * size) : 16;
            char **newtab = (char **)realloc (tab, sizeof (*tab) * (size));
            
            if (unlikely(newtab == NULL))
                goto error;
            tab = newtab;
        }
        
        tab[num] = strdup(entry);
        if (likely(tab[num] != NULL))
            num++;
    }
    
    if (compar != NULL && num > 0)
        qsort (tab, num, sizeof (*tab),
               (int (*)( const void *, const void *))compar);
    *namelist = tab;
    return num;
    
error:
    for (unsigned i = 0; i < num; i++)
        free (tab[i]);
    free (tab);
    return -1;
}

int mxScandir( const char *dirname, char ***namelist,
                int (*select)( const char * ),
                int (*compar)( const char **, const char ** ) )
{
    DIR *dir = mxOpendir (dirname);
    int val = -1;
    
    if (dir != NULL)
    {
        val = mxLoaddir (dir, namelist, select, compar);
        closedir (dir);
    }
    return val;
}

//#if defined (_WIN32) || defined (__OS2__)
//# include <vlc_rand.h>
//
//int mxMkstemp( char *template )
//{
//    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
//    static const int i_digits = sizeof(digits)/sizeof(*digits) - 1;
//    
//    /* */
//    assert( template );
//    
//    /* Check template validity */
//    const size_t i_length = strlen( template );
//    char *psz_rand = &template[i_length-6];
//    
//    if( i_length < 6 || strcmp( psz_rand, "XXXXXX" ) )
//    {
//        errno = EINVAL;
//        return -1;
//    }
//    
//    /* */
//    for( int i = 0; i < 256; i++ )
//    {
//        /* Create a pseudo random file name */
//        uint8_t pi_rand[6];
//        
//        vlc_rand_bytes( pi_rand, sizeof(pi_rand) );
//        for( int j = 0; j < 6; j++ )
//            psz_rand[j] = digits[pi_rand[j] % i_digits];
//        
//        /* */
//        int fd = mxOpen( template, O_CREAT | O_EXCL | O_RDWR, 0600 );
//        if( fd >= 0 )
//            return fd;
//        if( errno != EEXIST )
//            return -1;
//    }
//    
//    errno = EEXIST;
//    return -1;
//}
//#endif
