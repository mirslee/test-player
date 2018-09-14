#include "stdafx.h"
#include "MxFileSystem.h"
#include "MxConfig.h"

#include <assert.h>

#include <stdio.h>
#include <limits.h> /* NAME_MAX */
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef HAVE_LSTAT
# define lstat(a, b) stat(a, b)
#endif
#include <dirent.h>
#include <sys/socket.h>
#ifndef O_TMPFILE
# define O_TMPFILE 0
#endif

#if !defined(HAVE_ACCEPT4) || !defined HAVE_MKOSTEMP
static inline void mxCloexec(int fd)
{
    fcntl(fd, F_SETFD, FD_CLOEXEC | fcntl(fd, F_GETFD));
}
#endif

int mxOpen (const char *filename, int flags, ...)
{
    unsigned int mode = 0;
    va_list ap;
    
    va_start (ap, flags);
    if (flags & (O_CREAT|O_TMPFILE))
        mode = va_arg (ap, unsigned int);
    va_end (ap);
    
#ifdef O_CLOEXEC
    return open(filename, flags | O_CLOEXEC, mode);
#else
    int fd = open(filename, flags, mode);
    if (fd != -1)
        vlc_cloexec(fd);
    return -1;
#endif
}

int mxOpenat (int dir, const char *filename, int flags, ...)
{
    unsigned int mode = 0;
    va_list ap;
    
    va_start (ap, flags);
    if (flags & (O_CREAT|O_TMPFILE))
        mode = va_arg (ap, unsigned int);
    va_end (ap);
    
#ifdef HAVE_OPENAT
    return openat(dir, filename, flags | O_CLOEXEC, mode);
#else
    MX_UNUSED (dir);
    MX_UNUSED (filename);
    MX_UNUSED (mode);
    errno = ENOSYS;
    return -1;
#endif
}

int mxMkstemp (char * templ)
{
#if defined (HAVE_MKOSTEMP) && defined (O_CLOEXEC)
    return mkostemp(templ, O_CLOEXEC);
#else
    int fd = mkstemp(templ);
    if (fd != -1)
        mxCloexec(fd);
    return fd;
#endif
}

int mxMemfd (void)
{
    int fd;
#if O_TMPFILE
    fd = vlc_open ("/tmp", O_RDWR|O_TMPFILE, S_IRUSR|S_IWUSR);
    if (fd != -1)
        return fd;
    /* ENOENT means either /tmp is missing (!) or the kernel does not support
     * O_TMPFILE. EISDIR means /tmp exists but the kernel does not support
     * O_TMPFILE. EOPNOTSUPP means the kernel supports O_TMPFILE but the /tmp
     * filesystem does not. Do not fallback on other errors. */
    if (errno != ENOENT && errno != EISDIR && errno != EOPNOTSUPP)
        return -1;
#endif
    
    char bufpath[] = "/tmp/" PACKAGE_NAME "XXXXXX";
    
    fd = mxMkstemp (bufpath);
    if (fd != -1)
        unlink (bufpath);
    return fd;
}

int mxClose (int fd)
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

int mxMkdir (const char *dirname, mode_t mode)
{
    return mkdir (dirname, mode);
}

DIR *mxOpendir (const char *dirname)
{
    return opendir (dirname);
}

const char *mxReaddir(DIR *dir)
{
    struct dirent *ent = readdir (dir);
    return (ent != NULL) ? ent->d_name : NULL;
}

int mxStat (const char *filename, struct stat *buf)
{
    return stat (filename, buf);
}

int mxLstat (const char *filename, struct stat *buf)
{
    return lstat (filename, buf);
}

int mxUnlink (const char *filename)
{
    return unlink (filename);
}

int mxRename (const char *oldpath, const char *newpath)
{
    return rename (oldpath, newpath);
}

char *mxGetcwd (void)
{
    long path_max = pathconf (".", _PC_PATH_MAX);
    size_t size = (path_max == -1 || path_max > 4096) ? 4096 : path_max;
    
    for (;; size *= 2)
    {
        char *buf = (char*)malloc (size);
        if (unlikely(buf == NULL))
            break;
        
        if (getcwd (buf, size) != NULL)
            return buf;
        free (buf);
        
        if (errno != ERANGE)
            break;
    }
    return NULL;
}

int mxDup (int oldfd)
{
#ifdef F_DUPFD_CLOEXEC
    return fcntl (oldfd, F_DUPFD_CLOEXEC, 0);
#else
    int newfd = dup (oldfd);
    if (newfd != -1)
        vlc_cloexec(oldfd);
    return newfd;
#endif
}

int mxPipe (int fds[2])
{
#ifdef HAVE_PIPE2
    return pipe2(fds, O_CLOEXEC);
#else
    int ret = pipe(fds);
    if (ret == 0)
    {
        mxCloexec(fds[0]);
        mxCloexec(fds[1]);
    }
    return ret;
#endif
}

ssize_t mxWrite(int fd, const void *buf, size_t len)
{
    struct iovec iov = { .iov_base = (void *)buf, .iov_len = len };
    
    return mxWritev(fd, &iov, 1);
}

ssize_t mxWritev(int fd, const struct iovec *iov, int count)
{
    sigset_t set, oset;
    
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &set, &oset);
    
    ssize_t val = writev(fd, iov, count);
    if (val < 0 && errno == EPIPE)
    {
#if (_POSIX_REALTIME_SIGNALS > 0)
        siginfo_t info;
        struct timespec ts = { 0, 0 };
        
        while (sigtimedwait(&set, &info, &ts) >= 0 || errno != EAGAIN);
#else
        for (;;)
        {
            sigset_t s;
            int num;
            
            sigpending(&s);
            if (!sigismember(&s, SIGPIPE))
                break;
            
            sigwait(&set, &num);
            assert(num == SIGPIPE);
        }
#endif
    }
    
    if (!sigismember(&oset, SIGPIPE)) /* Restore the signal mask if changed */
        pthread_sigmask(SIG_SETMASK, &oset, NULL);
    return val;
}

//#include <vlc_network.h>

#ifndef HAVE_ACCEPT4
static void mxSocketSetup(int fd, bool nonblock)
{
    mxCloexec(fd);
    
    if (nonblock)
        fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    
#ifdef SO_NOSIGPIPE
    int a = (int){1};
    setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, /*&(int){ 1 }*/&a, sizeof (int));
#endif
}
#endif

int mxSocket (int pf, int type, int proto, bool nonblock)
{
#ifdef SOCK_CLOEXEC
    if (nonblock)
        type |= SOCK_NONBLOCK;
    
    int fd = socket(pf, type | SOCK_CLOEXEC, proto);
# ifdef SO_NOSIGPIPE
    if (fd != -1)
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, sizeof (int));
# endif
#else
    int fd = socket (pf, type, proto);
    if (fd != -1)
        mxSocketSetup(fd, nonblock);
#endif
    return fd;
}

int mxSocketpair(int pf, int type, int proto, int fds[2], bool nonblock)
{
#ifdef SOCK_CLOEXEC
    if (nonblock)
        type |= SOCK_NONBLOCK;
    
    int ret = socketpair(pf, type | SOCK_CLOEXEC, proto, fds);
# ifdef SO_NOSIGPIPE
    if (ret == 0)
    {
        const int val = 1;
        
        setsockopt(fds[0], SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof (val));
        setsockopt(fds[1], SOL_SOCKET, SO_NOSIGPIPE, &val, sizeof (val));
    }
# endif
#else
    int ret = socketpair(pf, type, proto, fds);
    if (ret == 0)
    {
        mxSocketSetup(fds[0], nonblock);
        mxSocketSetup(fds[1], nonblock);
    }
#endif
    return ret;
}

int mxAccept (int lfd, struct sockaddr *addr, socklen_t *alen, bool nonblock)
{
#ifdef HAVE_ACCEPT4
    int flags = SOCK_CLOEXEC;
    if (nonblock)
        flags |= SOCK_NONBLOCK;
    
    int fd = accept4(lfd, addr, alen, flags);
# ifdef SO_NOSIGPIPE
    if (fd != -1)
        setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &(int){ 1 }, sizeof (int));
# endif
#else
    int fd = accept(lfd, addr, alen);
    if (fd != -1)
        mxSocketSetup(fd, nonblock);
#endif
    return fd;
}
