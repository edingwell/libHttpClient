// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#pragma warning(disable: 4503) // C4503: decorated name length exceeded, name was truncated  
#pragma warning(disable: 4242) 

#ifdef _WIN32
// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>

// Windows
#if HC_UWP_API
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#define NOMINMAX

#include <windows.h>
//#include <winapifamily.h>
#else
#define __STDC_LIMIT_MACROS
#include <stdint.h>

//#include <boost/uuid/uuid.hpp>
#endif

// STL includes
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <codecvt>
#include <iomanip>

#if HC_UWP_API
#include <collection.h>
#endif

#ifndef _WIN32
#define UNREFERENCED_PARAMETER(args)
#endif

#ifndef max
#define max(x,y) std::max(x,y)
#endif

#ifndef _Printf_format_string_
#define _Printf_format_string_ 
#endif

#ifndef _Post_invalid_
#define _Post_invalid_ 
#endif

#ifndef _In_opt_
#define _In_opt_ 
#endif

#ifndef _In_z_
#define _In_z_ 
#endif

#ifndef _In_opt_z_
#define _In_opt_z_ 
#endif

#ifndef _In_reads_bytes_
#define _In_reads_bytes_(size) 
#endif

#ifndef _In_reads_
#define _In_reads_(size) 
#endif

#ifndef _In_reads_bytes_opt_
#define _In_reads_bytes_opt_(size) 
#endif

#ifndef _Outptr_result_bytebuffer_maybenull_
#define _Outptr_result_bytebuffer_maybenull_(size)
#endif

#ifndef __cdecl
#define __cdecl 
#endif

#ifndef _Out_
#define _Out_ 
#endif

#ifndef _Out_opt_
#define _Out_opt_ 
#endif


#ifndef _Outptr_
#define _Outptr_ 
#endif


// TODO: Add check for android
#ifndef CONST
#define CONST const
#endif

typedef unsigned char BYTE; 
typedef char CHAR;
typedef wchar_t WCHAR;

typedef CONST CHAR *PCSTR;
typedef BYTE *PBYTE;
typedef CONST WCHAR *PCWSTR;

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) sizeof(x) / sizeof(x[0])
#endif

#if _MSC_VER <= 1800
typedef std::chrono::system_clock chrono_clock_t;
#else
typedef std::chrono::steady_clock chrono_clock_t;
#endif

#define NAMESPACE_XBOX_HTTP_CLIENT_BEGIN                     namespace xbox { namespace httpclient {
#define NAMESPACE_XBOX_HTTP_CLIENT_END                       }}
#define NAMESPACE_XBOX_HTTP_CLIENT_LOG_BEGIN                 namespace xbox { namespace httpclient { namespace log {
#define NAMESPACE_XBOX_HTTP_CLIENT_LOG_END                   }}}
#define NAMESPACE_XBOX_HTTP_CLIENT_DETAIL_BEGIN              namespace xbox { namespace httpclient { namespace detail {
#define NAMESPACE_XBOX_HTTP_CLIENT_DETAIL_END                }}}
#define NAMESPACE_XBOX_HTTP_CLIENT_TEST_BEGIN                namespace xbox { namespace httpclienttest {
#define NAMESPACE_XBOX_HTTP_CLIENT_TEST_END                  }}

#if !HC_UNITTEST_API
#define ENABLE_LOGS 1
#define ENABLE_ASSERTS 1
#endif

#ifdef ENABLE_ASSERTS
#define HC_ASSERT(x) assert(x);
#else
#define HC_ASSERT(x)
#endif

typedef int32_t function_context;
#include <httpClient/httpClient.h>
#include "../Global/mem.h"
#include "Win/utils_win.h"
#include "utils.h"
#include "../Task/task_impl.h"
#include "../Global/global.h"
#include "trace_internal.h"

HC_DECLARE_TRACE_AREA(HTTPCLIENT);
HC_DECLARE_TRACE_AREA(WEBSOCKET);

#define CATCH_RETURN() CATCH_RETURN_IMPL(__FILE__, __LINE__)

#define CATCH_RETURN_IMPL(file, line) \
    catch (std::bad_alloc const& e) { return ::xbox::httpclient::detail::StdBadAllocToResult(e, file, line); } \
    catch (std::exception const& e) { return ::xbox::httpclient::detail::StdExceptionToResult(e, file, line); } \
    catch (...) { return ::xbox::httpclient::detail::UnknownExceptionToResult(file, line); }

#define CATCH_RETURN_WITH(errCode) CATCH_RETURN_IMPL_WITH(__FILE__, __LINE__, errCode)

#define CATCH_RETURN_IMPL_WITH(file, line, errCode) \
    catch (std::bad_alloc const& e) { ::xbox::httpclient::detail::StdBadAllocToResult(e, file, line); return errCode; } \
    catch (std::exception const& e) { ::xbox::httpclient::detail::StdExceptionToResult(e, file, line); return errCode; } \
    catch (...) { ::xbox::httpclient::detail::UnknownExceptionToResult(file, line); return errCode; }

#define RETURN_IF_PERFORM_CALLED(call) if (call->performCalled) return HC_E_PERFORMALREADYCALLED;
#define RETURN_IF_WEBSOCKET_CONNECT_CALLED(socket) if (socket->connectCalled) return HC_E_CONNECTALREADYCALLED;

NAMESPACE_XBOX_HTTP_CLIENT_DETAIL_BEGIN

HC_RESULT StdBadAllocToResult(std::bad_alloc const& e, _In_z_ char const* file, uint32_t line);
HC_RESULT StdExceptionToResult(std::exception const& e, _In_z_ char const* file, uint32_t line);
HC_RESULT UnknownExceptionToResult(_In_z_ char const* file, uint32_t line);

NAMESPACE_XBOX_HTTP_CLIENT_DETAIL_END
