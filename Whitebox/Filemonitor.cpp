/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/

#include "stdafx.h"
#include "drivers.h"
#include "utils.h"
#include "engine.h"
#include "configuration.h"

char Stats[LOGBUFSIZE];

//
// Function	: PrintOutput
// Role		: Takes the buffer we recieve from the driver and prints it out nicley
// Notes	: 
//
void PrintOutput(DWORD dwLen){

	PENTRY	ptr;

	for ( ptr = (PENTRY)Stats; (char *)ptr < min(Stats+dwLen,Stats + sizeof (Stats)); )  {
	 	// Add to list
		size_t len = strlen(ptr->text);

		ReplaceTabs(ptr,dwLen);
		if(ptr->text[0]!=','){
			if(strstr(ptr->text,"WRITE")>0) wbprintf(0,"[filemon] %s \n",ptr->text);
			if(strstr(ptr->text,"WRITE")>0) addactivity("[filemon] %s \n",ptr->text);
		}

		len += 4; len &= 0xFFFFFFFC; // +1 for null-terminator +3 for 32bit alignment
		ptr = (PENTRY)(ptr->text + len);
	}

}

//
// Function	: FileMonitorThread
// Role		: Communicates with the Sysinternal Filemon driver to minitor F/S activity
// Notes	: 
//
DWORD WINAPI FileMonitorThread(LPVOID lpvParam){

	FFILTER	FilterDefinition;
	DWORD dwFoo=0;
	DWORD dwFLen=0;
	DWORD	CurDriveSet;

	if(hgSysFile==INVALID_HANDLE_VALUE || hgSysFile==NULL) wbprintf(1,"[!] Handle to kernel driver is invalid!\n");
	
	// Set which drives to monitor
	DWORD dwMaxDriveSet=0;
	dwMaxDriveSet = GetLogicalDrives();
	CurDriveSet=dwMaxDriveSet;
	if ( ! DeviceIoControl(	hgSysFile, IOCTL_FILEMON_SETDRIVES,
							&CurDriveSet, sizeof CurDriveSet,
							&CurDriveSet, sizeof CurDriveSet,
							&dwFoo, NULL ) ){
		wbprintf(1,"[!] Couldn't set Filemon drives!\n");
		ExitProcess(1);
	} else {
		if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] Filemon drives to monitor set - %d\n",dwFoo);
	}


	if (!DeviceIoControl(hgSysFile, IOCTL_FILEMON_STOPFILTER,NULL,0,NULL,0,&dwFoo,NULL)){
		wbprintf(1,"[!] Couldn't get Filemon to stop filtering!");
		ExitProcess(1);
	}

	// Set the filters
	memset(&FilterDefinition,0x00,sizeof(FFILTER));
	if(strlen(strFileExcludeFilter)>0) {
		if(bVerbose && intVerboseLvl>=5){
			wbprintf(0,"[i] Filemon Exclude    : %s\n",strFileExcludeFilter);
		}
		strcpy_s(FilterDefinition.excludefilter,1024,strFileExcludeFilter);
	} else strcpy_s(FilterDefinition.excludefilter,1024,"WHITEBOX.EXE");
	
	if(strlen(strFileIncludeFilter)>0) {
		if(bVerbose && intVerboseLvl>=5){
			wbprintf(0,"[i] Filemon Include    : %s\n",strFileIncludeFilter);
		}
		strcpy_s(FilterDefinition.includefilter,1024,strFileIncludeFilter);
	} else strcpy_s(FilterDefinition.includefilter,1024,"*");

	FilterDefinition.logsucess=TRUE;
	FilterDefinition.logerrors=FALSE;
	FilterDefinition.logreads=FALSE;
	FilterDefinition.logwrites=TRUE;
	FilterDefinition.logopens=FALSE;

	if (!DeviceIoControl(hgSysFile, IOCTL_FILEMON_SETFILTER,
			(PVOID) &FilterDefinition, sizeof(FFILTER), 
			NULL, 0, &dwFoo, NULL ) )	{
		wbprintf(1,"[!] Couldn't set Filemon filters\n");
	} else {
		if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] Filemon filters set - %d - %d\n",dwFoo,sizeof(FilterDefinition));
		else if(bVerbose && intVerboseLvl >= 1) wbprintf(0,"[i] Filemon filters set\n");
	}


	// Tell driver to start filtering
	if (!DeviceIoControl(hgSysFile, IOCTL_FILEMON_STARTFILTER,NULL,0,NULL,0,&dwFoo,NULL)){
		wbprintf(1,"[!] Couldn't get Filemon to start filtering!");
		ExitProcess(1);
	} else {
		if(bVerbose && intVerboseLvl >= 1) wbprintf(0,"[i] Filemon now filtering - %d \n",dwFoo);
	}


	wbprintf(0,"[*] File activity monitor active\n");

	
	if(bDebug){
		DWORD dwVersionNumber=0;	
		// Print out the version number of the driver
		if (!DeviceIoControl(hgSysFile, IOCTL_FILEMON_VERSION,NULL,0,&dwVersionNumber,sizeof(DWORD),&dwFoo,NULL))
		{
			wbprintf(1,"[!] Couldn't get filemon driver version!\n");
			return 1;
		} else {
			wbprintf(0,"[i] Filemon Driver Version: %d\n", dwVersionNumber);
		}
	}

	// Now loop
	while(1){
		// Have driver fill stats buffer
		if(bDebug) wbprintf(0,"[debug] Checking filemon buffers\n");

		if (!DeviceIoControl(hgSysFile,IOCTL_FILEMON_GETSTATS,NULL,0,&Stats,sizeof Stats,&dwFLen,NULL)) 
		{
			ExitProcess(1);
		} else {
			if(dwFLen>0){
				if(bDebug) wbprintf(0,"[debug] Filemon got data - %d\n",dwFLen);
				if(bDebug) PrintHex(0,(LPBYTE)Stats,dwFLen,0);
				PrintOutput(dwFLen);
			} else {
				if(bDebug) wbprintf(0,"[debug] No data\n");
			}
		}

		// Have a little nap
		Sleep(500);
	}

	wbprintf(0,"[*] File activity monitor exiting\n");

	return 0;
}