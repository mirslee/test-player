
#ifndef __MXCONFIG_H__
#define __MXCONFIG_H__

#ifdef WIN32
#define INIT_PTHREAD(t) memset(&t,0,sizeof(t))
#define PTHREAD_IS_VALID(t) (t.p!=nullptr)
#else	
#define INIT_PTHREAD(t) t = nullptr;
#define PTHREAD_IS_VALID(t) (t!=nullptr)
#endif

#endif//__MXCONFIG_H__