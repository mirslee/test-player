#ifndef MXNETWORK_H
#define MXNETWORK_H

#include <sys/types.h>
#include <unistd.h>

#if defined( _WIN32 )
#   define _NO_OLDNAMES 1
#   include <winsock2.h>
#   include <ws2tcpip.h>
#   define net_errno (WSAGetLastError())
#   define net_Close(fd) ((void)closesocket((SOCKET)fd))
#   ifndef IPV6_V6ONLY
#       define IPV6_V6ONLY 27
#   endif
#else
#   include <sys/socket.h>
#   include <netinet/in.h>
#   include <netdb.h>
#   define net_errno errno
#   define net_Close(fd) ((void)vlc_close(fd))
#endif

#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

#include "MxCommon.h"

MX_API int mxAccept(int lfd, struct sockaddr *addr, socklen_t *alen, bool nonblock) MX_USED;

#endif /* MXNETWORK_H */
