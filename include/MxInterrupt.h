#ifndef MXINTERRUPT_H
#define MXINTERRUPT_H

#include "MxCommon.h"
#include "MxInterrupt.h"

# include "MxThread.h"
# ifndef _WIN32
#  include <sys/socket.h> /* socklen_t */
# else
typedef int     ssize_t;
#  include <ws2tcpip.h>
# endif

#include "MxAtomic.h"

#ifdef __cplusplus
using namespace std;
#endif

struct vlc_interrupt
{
    MxMutex lock;
    bool interrupted;
    atomic_bool killed;
    void (*callback)(void *);
    void *data;
};

struct pollfd;
struct iovec;
struct sockaddr;
struct msghdr;

MX_API int vlc_sem_wait_i11e(MxSem *);

MX_API int vlc_mwait_i11e(mtime_t);

static inline int vlc_msleep_i11e(mtime_t delay)
{
    return vlc_mwait_i11e(mdate() + delay);
}

MX_API int vlc_poll_i11e(struct pollfd *, unsigned, int);

MX_API ssize_t vlc_readv_i11e(int fd, struct iovec *, int);
MX_API ssize_t vlc_writev_i11e(int fd, const struct iovec *, int);
MX_API ssize_t vlc_read_i11e(int fd, void *, size_t);
MX_API ssize_t vlc_write_i11e(int fd, const void *, size_t);

MX_API ssize_t vlc_recvmsg_i11e(int fd, struct msghdr *, int flags);
MX_API ssize_t vlc_sendmsg_i11e(int fd, const struct msghdr *, int flags);

MX_API ssize_t vlc_recvfrom_i11e(int fd, void *, size_t, int flags,
                                  struct sockaddr *, socklen_t *);
MX_API ssize_t vlc_sendto_i11e(int fd, const void *, size_t, int flags,
                                const struct sockaddr *, socklen_t);

static inline ssize_t vlc_recv_i11e(int fd, void *buf, size_t len, int flags)
{
    return vlc_recvfrom_i11e(fd, buf, len, flags, NULL, NULL);
}

static inline
ssize_t vlc_send_i11e(int fd, const void *buf, size_t len, int flags)
{
    return vlc_sendto_i11e(fd, buf, len, flags, NULL, 0);
}

MX_API int vlc_accept_i11e(int fd, struct sockaddr *, socklen_t *, bool);

MX_API void vlc_interrupt_register(void (*cb)(void *), void *opaque);

MX_API int vlc_interrupt_unregister(void);

typedef struct vlc_interrupt vlc_interrupt_t;

MX_API vlc_interrupt_t *vlc_interrupt_create(void) MX_USED;

MX_API void vlc_interrupt_destroy(vlc_interrupt_t *);

MX_API vlc_interrupt_t *vlc_interrupt_set(vlc_interrupt_t *);

MX_API void vlc_interrupt_raise(vlc_interrupt_t *);

MX_API void vlc_interrupt_kill(vlc_interrupt_t *);

MX_API bool vlc_killed(void) MX_USED;

MX_API void vlc_interrupt_forward_start(vlc_interrupt_t *to,
                                         void *data[2]);
MX_API int vlc_interrupt_forward_stop(void *const data[2]);

#endif /* MXINTERRUPT_H */
