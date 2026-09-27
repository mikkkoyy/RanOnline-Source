
// stdafx.h : standard include preamble for NativeItemRoundTrip.
//
// Mirrors EditorItem/stdafx.h so that the legacy RAN headers
// (GLItemMan.h -> GLItem.h -> ...) resolve in a plain console build.
// MFC first (BOOL, CString), then the DXUT/RAN preamble (d3d9, d3dx9,
// strsafe, SAFE_DELETE, V()).

#pragma once

#ifndef WINVER
#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0501
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _AFX_ALL_WARNINGS

#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>
#endif

#include "dxstdafx.h"

#include <algorithm>
#include <cctype>
#include <deque>
#include <fstream>
#include <set>
#include <map>
#include <list>
#include <queue>
#include <string>
#include <vector>
#include <iterator>
