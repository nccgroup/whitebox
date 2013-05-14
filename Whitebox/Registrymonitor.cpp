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

char RStats[LOGBUFSIZE];

//
// Function	: PrintOutput
// Role		: Takes the buffer we recieve from the driver and prints it out nicley
// Notes	: 
//
void PrintOutputReg(DWORD dwLen){

	PENTRY	ptr;

	for ( ptr = (PENTRY)RStats; (char *)ptr < min(RStats+dwLen,RStats + sizeof (RStats)); )  {
	 	// Add to list
		size_t len = strlen(ptr->text);

		ReplaceTabs(ptr,dwLen);
		wbprintf(0,"[regmon] %s \n",ptr->text);
		addactivity("[regmon] %s \n",ptr->text);

		len += 4; len &= 0xFFFFFFFC; // +1 for null-terminator +3 for 32bit alignment
		ptr = (PENTRY)(ptr->text + len);
	}

}

//
// Function	: RegMonitorThread
// Role		: Communicates with the Sysinternal Regmon driver to minitor reg activity
// Notes	: 
//
DWORD WINAPI RegMonitorThread(LPVOID lpvParam){

	RFILTER	FilterDefinition;
	DWORD dwFoo=0;
	DWORD dwRLen=0;
	DWORD dwCrapOla=0;


	// Have driver turn on hooks
	if (!DeviceIoControl(hgSysReg, IOCTL_REGMON_HOOK,
		&dwCrapOla, 4, NULL, 0, &dwFoo, NULL ) )	{
		wbprintf(1,"[!] Couldn't get RegMon driver to turn on hooks!\n");
		CloseHandle(hgSysReg);
		ExitProcess(1);
	} else {
		if(bVerbose && intVerboseLvl >= 1) wbprintf(0,"[i] RegMon has turned on hooks\n");
	}

	// Set the filters
	memset(&FilterDefinition,0x00,sizeof(FilterDefinition));
	
	if(strlen(strRegiExcludeFilter)>0) {
		if(bVerbose && intVerboseLvl>=5){
			wbprintf(0,"[i] Regmon Exclude     : %s\n",strRegiExcludeFilter);
		}
		strcpy_s(FilterDefinition.excludefilter,1024,strRegiExcludeFilter);
	} else strcpy_s(FilterDefinition.excludefilter,1024,"WHITEBOX.EXE");
	
	if(strlen(strRegiIncludeFilter)>0) {
		if(bVerbose && intVerboseLvl>=5){
			wbprintf(0,"[i] Regmon Include     : %s\n",strRegiIncludeFilter);
		}
		strcpy_s(FilterDefinition.includefilter,1024,strRegiIncludeFilter);
	} else strcpy_s(FilterDefinition.includefilter,1024,"*");

	FilterDefinition.logaux=FALSE;
	FilterDefinition.logerror=FALSE;
	FilterDefinition.logreads=FALSE;
	FilterDefinition.logsuccess=TRUE;
	FilterDefinition.logwrites=TRUE;
	if (!DeviceIoControl(hgSysReg,IOCTL_REGMON_SETFILTER,
			(PVOID) &FilterDefinition, sizeof(FilterDefinition), 
			NULL, 0, &dwFoo, NULL ) )	{
			wbprintf(1,"[!] Couldn't set Regmon filters\n");
			ExitProcess(1);
	} else {
		if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] Regmon filters set - %d - %d\n",dwFoo,sizeof(FilterDefinition));
		else if(bVerbose && intVerboseLvl >= 1) wbprintf(0,"[i] Regmon filters set\n");
	}

	wbprintf(0,"[*] Registry activity monitor active\n");

	// Now loop
	while(1){
		if(bDebug) wbprintf(0,"[debug] Checking regmon buffers\n");

		// Have driver fill stats buffer
		if (!DeviceIoControl(hgSysReg,IOCTL_REGMON_GETSTATS,NULL,0,&RStats,sizeof RStats,&dwRLen, NULL)) 
		{
			ExitProcess(1);
		} else {
			if(dwRLen>0){
				if(bDebug) wbprintf(0,"[debug] Regmon got data - %d\n",dwRLen);
				if(bDebug) PrintHex(0,(LPBYTE)RStats,dwRLen,0);
				PrintOutputReg(dwRLen);
			}
		}

		// Have a little nap
		Sleep(500);
	}

	wbprintf(0,"[*] Registry activity monitor exiting\n");
}