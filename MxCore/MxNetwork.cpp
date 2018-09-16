#include "stdafx.h"
#include "MxNetwork.h"
#include "MxCommon.h"
#include "MxConfig.h"
#include <fcntl.h>

#ifndef _WIN32
int mxAccept (int lfd, struct sockaddr *addr, socklen_t *alen, bool nonblock)
{
    do
    {
        int fd = accept (lfd, addr, alen);
        if (fd != -1)
        {
            fcntl (fd, F_SETFD, FD_CLOEXEC);
            if (nonblock)
                fcntl (fd, F_SETFL, fcntl (fd, F_GETFL, 0) | O_NONBLOCK);
            return fd;
        }
    }
    while (errno == EINTR);
    
    return -1;
}
#endif