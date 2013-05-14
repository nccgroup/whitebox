/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/

// Globals
extern char strFileExcludeFilter[1024];
extern char strFileIncludeFilter[1024];
extern char strRegiExcludeFilter[1024];
extern char strRegiIncludeFilter[1024];
extern char strNetwExcludeFilter[1024];
extern TCHAR strConfInDir[MAX_PATH];
extern TCHAR strConfOutDir[MAX_PATH];
extern TCHAR strConfTempDir[MAX_PATH];
extern TCHAR strTCPPort[6];
extern bool bNetworkEngine;
extern bool bFilemonEngine;
extern bool bRegEngine;
extern bool bReportEnabled;

// Functions
DWORD LoadConfiguration(TCHAR *strFile);
