
#ifndef __MXERROR_H__
#define __MXERROR_H__


#define _FACLS  0x008
#ifdef _WIN32
#define MAKE_RESULT( code )        MAKE_HRESULT( 1, _FACLS, code )
#else
#define MAKE_RESULT( code )     1
#endif

#define MX_NOERROR						0
#define MX_OK							0
#define MX_SUCCESS                      0
#define MX_FAILED					    -1
#define MX_EGENERIC                     -1

typedef long MxResult;
#define MX_S_OK                            0
#define MX_E_FAIL                        0x80004005
#define MX_E_INVALIDARG                    0x80070057
#define MX_E_OUTOFMEMORY                MAKE_RESULT( 0x0003 )            //    ƒ⁄¥Ê≤ª◊„
#define NOERROR 0

#define Mx_Failed(hr) ((hr)<0)


#endif //__MXERROR_H__
