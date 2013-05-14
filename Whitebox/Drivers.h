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

// Defines
#define	SYSF_FILE			_T("FILEM701.SYS")
#define	SYSF_NAME			_T("FILEMON701")
#define	SYSR_FILE			_T("REGSYS701.SYS")
#define	SYSR_NAME			_T("REGMON701")

// Driver commands
#define IOCTL_REGMON_HOOK      (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x00, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_REGMON_UNHOOK    (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x01, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_REGMON_ZEROSTATS (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x02, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_REGMON_GETSTATS  (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x03, METHOD_NEITHER, FILE_WRITE_ACCESS )
#define IOCTL_REGMON_SETFILTER (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x04, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_REGMON_VERSION   (ULONG) CTL_CODE( FILE_DEVICE_REGMON, 0x05, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOCTL_FILEMON_SETDRIVES   (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x00, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_ZEROSTATS   (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x01, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_GETSTATS    (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x02, METHOD_NEITHER, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_UNLOADQUERY (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x03, METHOD_NEITHER, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_STOPFILTER  (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x04, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_STARTFILTER (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x05, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_SETFILTER   (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x06, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_VERSION     (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x07, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_HOOKSPECIAL (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x08, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define IOCTL_FILEMON_UNHOOKSPECIAL (ULONG) CTL_CODE( FILE_DEVICE_FILEMON, 0x09, METHOD_BUFFERED, FILE_ANY_ACCESS)

// Memory
#define PAGE_SIZE 0x1000  // 4K
#define LOGBUFSIZE  	(PAGE_SIZE*16 - 3*sizeof(ULONG))

// File device
#define FILE_DEVICE_REGMON  0x00008305
#define FILE_DEVICE_FILEMON	0x00008300

// Maxmimum filter lengths
#define MAXFILTERLEN   1024

// Taken from IOCTLCMD.h from RegSrc.zip
// Filter definition
typedef struct {
        CHAR   includefilter[MAXFILTERLEN];
        CHAR    excludefilter[MAXFILTERLEN];
        BOOLEAN  logsuccess;
        BOOLEAN  logerror;
        BOOLEAN  logreads;
        BOOLEAN  logwrites;
		BOOLEAN  logaux;
} RFILTER, *PRFILTER;

// Taken from IOCTLCMD.h from FileSrc.zip
// Filter definition
// Update: This is the *new* structure for the newer ver
//        of the driver - 3 new documented options / 2 undocumented
typedef struct {
        CHAR   includefilter[MAXFILTERLEN];
        CHAR    excludefilter[MAXFILTERLEN];
		BOOLEAN  logsucess;
		BOOLEAN  logerrors;
		BOOLEAN  logreads;
		BOOLEAN  logwrites;
		BOOLEAN  logopens;
		BOOLEAN  logundoc1;
		BOOLEAN  logundoc2;
} FFILTER, *PFFILTER;

// Taken from IOCTLCMD.h from FileSrc.zip
// format of a data entry
#pragma pack(1)
typedef struct {
	ULONG	          seq;
    LARGE_INTEGER     perftime;
    LARGE_INTEGER     datetime;
	char	          text[0];
} ENTRY, *PENTRY;
#pragma pack()

// Taken from
// http://www.xfocus.net/articles/200210/456.html
typedef struct _LSA_UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PVOID  Buffer;
} LSA_UNICODE_STRING, *PLSA_UNICODE_STRING; 

typedef LSA_UNICODE_STRING UNICODE_STRING, *PUNICODE_STRING;

typedef struct _ANSI_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PCHAR Buffer;
}ANSI_STRING,*PANSI_STRING;


// Functions
DWORD LoadRegmonDriver();
DWORD LoadFilemonDriver();
DWORD LoadDrivers();