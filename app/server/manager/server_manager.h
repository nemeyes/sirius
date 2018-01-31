#pragma warning( disable: 4049 )
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif

#ifndef __SERVER_MANAGER_H__
#define __SERVER_MANAGER_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef __SERVERMANAGERLIB_LIBRARY_DEFINED__
#define __SERVERMANAGERLIB_LIBRARY_DEFINED__

EXTERN_C const IID LIBID_SERVERMANAGERLIB;
#endif

#ifdef __cplusplus
}
#endif
#endif


