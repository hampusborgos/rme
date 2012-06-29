//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/definitions.h $
// $Id: definitions.h 321 2010-03-09 17:20:26Z admin $

#ifndef RME_DEFINITIONS_H_
#define RME_DEFINITIONS_H_

// Version info
// xxyyzzt (major, minor, subversion)
#define __RME_VERSION_MAJOR__      3
#define __RME_VERSION_MINOR__      0
#define __RME_SUBVERSION__         0

#define __LIVE_NET_VERSION__       5

#define MAKE_VERSION_ID(major, minor, subversion) \
	((major)      * 10000000 + \
     (minor)      * 100000 + \
     (subversion) * 1000)


#define __RME_VERSION_ID__ MAKE_VERSION_ID(\
	__RME_VERSION_MAJOR__, \
    __RME_VERSION_MINOR__, \
    __RME_SUBVERSION__)

#ifdef __EXPERIMENTAL__
#   define __RME_VERSION__ std::string(i2s(__RME_VERSION_MAJOR__) + "." + i2s(__RME_VERSION_MINOR__) + + " BETA")
#   define __W_RME_VERSION__ (wxString() << __RME_VERSION_MAJOR__ << wxT(".") << __RME_VERSION_MINOR__ << wxT(" BETA"))
#else
#   define __RME_VERSION__ std::string(i2s(__RME_VERSION_MAJOR__) + "." + i2s(__RME_VERSION_MINOR__))
#   define __W_RME_VERSION__ (wxString() << __RME_VERSION_MAJOR__ << wxT(".") << __RME_VERSION_MINOR__)
#endif
// OS

#ifdef __VISUALC__
#pragma warning(disable:4996) // Stupid MSVC complaining 'bout "unsafe" functions
#pragma warning(disable:4800) // Bool conversion warning
#pragma warning(disable:4100) // Unused parameter warning ( I like to name unused stuff... )
#pragma warning(disable:4706) // Assignment within conditional expression
#endif

#ifndef FORCEINLINE
#   ifdef __VISUALC__
#       define FORCEINLINE __forceinline
#   else
#       define FORCEINLINE inline
#   endif
#endif

// Debug mode?
#if defined __DEBUG__ || defined _DEBUG || defined __DEBUG_MODE__
#	undef __DEBUG_MODE__
#	define __DEBUG_MODE__ 1
#	undef _DEBUG
#	define _DEBUG 1
#	undef __DEBUG__
#	define __DEBUG__ 1
#else
#   ifndef __RELEASE__
#       define __RELEASE__ 1
#   endif
#	ifndef NDEBUG
#		define NDEBUG 1
#	endif
#endif

#ifdef __RELEASE__
#   ifdef __VISUALC__
#       define _SCL_SECURE 0
#       define _SECURE_SCL 0 // They can't even decide on a coherent define interface!
#       define _HAS_ITERATOR_DEBUGGING 0
#   endif
#endif

#ifndef _DONT_USE_UPDATER_
#   if defined __WINDOWS__ && !defined _USE_UPDATER_
#       define _USE_UPDATER_
#    endif
#endif

#ifndef _DONT_USE_PROCESS_COM
#   if defined __WINDOWS__ && !defined _USE_PROCESS_COM
#       define _USE_PROCESS_COM
#   endif
#endif

// Mathematical constants
#define PI 3.14159265358979323846264338327950288419716939937510
#define DEG2RAD (PI/180.0)
#define RAD2DEG (180.0/DEG)

// The height of the map (there should be more checks for this...)
#define MAP_HEIGHT 16

// wxString conversions
#define nstr(str) std::string((const char*)(str.mb_str(wxConvUTF8)))
#define wxstr(str) wxString((str).c_str(), wxConvUTF8)

#ifdef __WINDOWS__
#ifndef _STDINT_H
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
#endif
typedef unsigned int uint;
#else
typedef unsigned int uint;
#endif

#endif
