/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/

#include "drivers.h"

BOOL SetPrivilege(HANDLE hProcess, TCHAR *strPriv);
BOOL OpenLog(TCHAR *strFile);
BOOL CloseLog();
void addactivity(char *strFmt, ...);
void wbprintf(int intType, char *strFmt, ...);
void PrintHex(DWORD dwOffset, LPBYTE pBytes, DWORD dwLen,DWORD dwPrettyOffset);
void ReplaceTabs(PENTRY	ptr, DWORD dwLen);
void WriteReport(char *strFile);