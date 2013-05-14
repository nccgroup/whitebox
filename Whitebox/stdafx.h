/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <memory.h>


// Defines
#define intVerMaj 0
#define intVerMin 1

// Globals (actually defined in Whitebox.cpp)
extern bool bDebug;
extern bool bVerbose;
extern unsigned int intVerboseLvl;
extern int intNetDevice;
extern bool bLog;
extern TCHAR *strLogFile;
extern FILE *fileLog;
extern TCHAR *strConfFile;
extern FILE *fileConf;
extern TCHAR *strInDir;
extern TCHAR *strOutDir;
extern TCHAR *strTempDir;
