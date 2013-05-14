/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/


// Defines
#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "999"
#define ACTIVITY_SIZE 10240000

// Global
extern SOCKET socketClient;
extern HANDLE hgSysFile;
extern HANDLE hgSysReg;
extern char *strActivity;

// Externall called functions
int WhiteboxEngine();