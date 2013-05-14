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
#include "engine.h"

//
// Function	: SetPrivilege
// Role		: Enables a particular windows priv for the process
// Notes	: 
//
BOOL SetPrivilege(HANDLE hProcess, TCHAR *strPriv)
{
	LUID luid ;
	TOKEN_PRIVILEGES privs ;
	HANDLE hToken = NULL ;
	DWORD dwBufLen = 0 ;
	char buf[1024] ;
	
	ZeroMemory(&luid,sizeof(luid));
	
	if(!LookupPrivilegeValue(NULL,strPriv,&luid))
		return false ;
	
	privs.PrivilegeCount = 1 ;
	privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
	memcpy( &privs.Privileges[0].Luid, &luid, sizeof(privs.Privileges[0].Luid ));
	
	if(!OpenProcessToken(hProcess,TOKEN_ALL_ACCESS,&hToken))
		return false;
	
	if(!AdjustTokenPrivileges(hToken,FALSE,&privs,
		sizeof(buf),(PTOKEN_PRIVILEGES)buf,&dwBufLen))
		return false;

	CloseHandle(hProcess);
	CloseHandle(hToken);
	
	return true;
}

//
// Function	: OpenLog
// Role		: Opens the log file
// Notes	: 
//
BOOL OpenLog(TCHAR *strFile){

	DWORD dwRet=0;

	dwRet=_wfopen_s(&fileLog,strFile,L"w+");

	if(dwRet==0){
		return true;
	} else {
		return false;
	}
}

//
// Function	: CloseLog
// Role		: Closes the log file
// Notes	: 
//
BOOL CloseLog(){

	DWORD dwRet=0;

	if(fileLog!=NULL){	
		if(fclose(fileLog)!=0){
			return false;
		} else {
			return true;
		}
	} else {
		return false;
	}
}


//
// Function	: wbprintf
// Role		: Own printf function
// Notes	: 
//
void wbprintf(int intType, char *strFmt, ...){

	va_list vArgs;
	va_start(vArgs,strFmt);

	switch(intType){
		case 0: // stdout
			vfprintf(stdout,strFmt,vArgs);			
			break;
		case 1: // stderr
			vfprintf(stderr,strFmt,vArgs);
			break;
		default:
			vfprintf(stderr,strFmt,vArgs);
			break;
	}

	if(bLog && fileLog!=NULL ) vfprintf(fileLog,strFmt,vArgs);
}

//
// Function	: addactivity
// Role		: Adds activity to our 10MB heap buffer
// Notes	: 
//
void addactivity(char *strFmt, ...){

	char strTemp[2048];
	memset(strTemp,0x00,2048);
	va_list vArgs;
	va_start(vArgs,strFmt);
	vsprintf_s(strTemp,2048,strFmt,vArgs);
	if(bDebug)wbprintf(0,"[debug] %s %d",strTemp,strlen(strActivity));
	//wbprintf(0,"[debug] %d",strlen(strActivity));
	if(strlen(strActivity)==0){
		strcpy_s(strActivity,ACTIVITY_SIZE,strTemp);
	} else {
		strcat_s(strActivity,ACTIVITY_SIZE,strTemp);
	}
}

//
// Function	: write report
// Role		: Write out the strActivity to a report
// Notes	: 
//
void WriteReport(char *strFile){
	FILE *fileReport=NULL;
	char strReportFile[MAX_PATH];

	sprintf_s(strReportFile,MAX_PATH,"%S\\%s.report",strOutDir,strFile);

	fopen_s(&fileReport,strReportFile,"w+");

	if(fileReport!=NULL) fprintf(fileReport,"%s",strActivity);
	else wbprintf(1,"[!] Couldn't write report for %s\n",strFile);

	fflush(fileReport);
	fclose(fileReport);
}

#define HEX2CHAR(x) (((x) < 0x0a) ? (x) + '0' : ((x)-0xa) + 'A')

//
// Function	: PrintHex
// Role		: Prints out pretty style the data
// Notes	: 
//
void PrintHex(DWORD dwOffset, LPBYTE pBytes, DWORD dwLen,DWORD dwPrettyOffset)
{
    TCHAR *szOutStr;
    DWORD dwStartOffset, i, j;
	
	szOutStr = (TCHAR *)LocalAlloc(LMEM_FIXED,dwLen*6);
	
    for (dwStartOffset = 0; dwStartOffset < dwLen; dwStartOffset += 16) {
		wsprintf (szOutStr, TEXT(" "));
        i = (DWORD)_tcslen(szOutStr);

        // Print them out in HEX
        for (j=0; j < 16; j++) {
            if ((dwStartOffset + j) < dwLen) {
                szOutStr[i++] = HEX2CHAR(pBytes[dwStartOffset+j]>>4);
                szOutStr[i++] = HEX2CHAR(pBytes[dwStartOffset+j]&0x0F);
            } else {
                szOutStr[i++] = TEXT(' ');
                szOutStr[i++] = TEXT(' ');
            }
            szOutStr[i++] = TEXT(' ');
            if (7 == j) {
                szOutStr[i++] = TEXT('-');
                szOutStr[i++] = TEXT(' ');
            }

        }

        // A little space
        for (j=0; j < 5; j++) {
            szOutStr[i++] = TEXT(' ');
        }
        szOutStr[i++] = TEXT(':');

        for (j=0; j < 16; j++) {
            if ((dwStartOffset + j) < dwLen) {
                if ((pBytes[dwStartOffset+j] < ' ') || (pBytes[dwStartOffset+j] >= 0x7f)) {
                    szOutStr[i++] = TEXT('.');
                } else {
                    szOutStr[i++] = pBytes[dwStartOffset+j];
                }
            } else {
                szOutStr[i++] = TEXT(' ');
            }
        }
        szOutStr[i++] = TEXT(':');
        szOutStr[i++] = TEXT('\r');
        szOutStr[i++] = TEXT('\n');
        szOutStr[i++] = TEXT('\0');
        
		// Some output
		fwprintf(stdout, TEXT("%s"),szOutStr);
    }

	LocalFree(szOutStr);

}

//
// Function	: RemoveTabs
// Role		: Takes the buffer and replaces the tabs
// Notes	: 
//
void ReplaceTabs(PENTRY	ptr, DWORD dwLen){
	
	DWORD dwCount=0;

	for(dwCount=0;dwCount<dwLen;dwCount++){
		if(ptr->text[dwCount]=='\t') ptr->text[dwCount]=',';

	}

	return;
}