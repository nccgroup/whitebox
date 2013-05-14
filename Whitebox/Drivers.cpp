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

// Globals 
HANDLE hSysReg = INVALID_HANDLE_VALUE;
HANDLE hSysFile = INVALID_HANDLE_VALUE;

// Taken from http://www.xfocus.net/articles/200210/456.html
typedef DWORD (CALLBACK* NTLOADDRIVER)(PVOID);
NTLOADDRIVER NtLoadDriver;
typedef DWORD (CALLBACK* RTLFREEANSISTRING)(PVOID);
RTLFREEANSISTRING RtlFreeAnsiString;
typedef DWORD (CALLBACK* RTLUNICODESTRINGTOANSISTRING)(PANSI_STRING, PUNICODE_STRING,DWORD);
RTLUNICODESTRINGTOANSISTRING RtlUnicodeStringToAnsiString;
typedef DWORD (CALLBACK* RTLINITANSISTRING)(PVOID, PVOID);
RTLINITANSISTRING RtlInitAnsiString;
typedef DWORD (CALLBACK* RTLANSISTRINGTOUNICODESTRING)(PVOID, PVOID,DWORD);
RTLANSISTRINGTOUNICODESTRING RtlAnsiStringToUnicodeString;
typedef DWORD (CALLBACK* RTLFREEUNICODESTRING)(PVOID);
RTLFREEUNICODESTRING RtlFreeUnicodeString;

//
// Function	: RegHandle
// Role		: Registers the driver and loads it
// Notes	: It doesn't unload them / delete them
//
int RegHandleDev(TCHAR * strExename, HANDLE *hDevice, TCHAR *strFile, char *strReg, TCHAR *strDeviceDir)
{
    TCHAR strSubkey[MAX_PATH];
	char strFoo[MAX_PATH];
    LSA_UNICODE_STRING buf1;
    LSA_UNICODE_STRING buf2;
    int intLen;
    HKEY hkResult;
    TCHAR Data[4];
    DWORD isok;
	HMODULE hNtdll = NULL;

	if(bDebug)wbprintf(0,"[debug] RegHandleDev()....\n");

	// Get some function pointers we need
	hNtdll = LoadLibrary( L"NTDLL.DLL" ); 
	NtLoadDriver = (NTLOADDRIVER) GetProcAddress(hNtdll,"NtLoadDriver");
	RtlAnsiStringToUnicodeString = (RTLANSISTRINGTOUNICODESTRING) GetProcAddress(hNtdll,"RtlAnsiStringToUnicodeString");
    RtlFreeUnicodeString = (RTLFREEUNICODESTRING) GetProcAddress(hNtdll,"RtlFreeUnicodeString");
	
	// TODO - error checking for the above here


	// Create the base key and set some fluff
	intLen = swprintf(strSubkey,MAX_PATH,L"System\\CurrentControlSet\\Services\\%s",strExename);
    strSubkey[intLen]=0;
    isok = RegCreateKey(HKEY_LOCAL_MACHINE,strSubkey,&hkResult);
	if(isok!=ERROR_SUCCESS){
        wbprintf(1,"[!] RegCreateKey failed - %d\n",GetLastError());
		return false;
	}

	Data[0]=1;
    Data[1]=0;
    Data[2]=0;
    Data[3]=0;
    isok=RegSetValueEx(hkResult,L"Type",0,REG_DWORD,(const unsigned char *)Data,4);
    isok=RegSetValueEx(hkResult,L"ErrorControl",0,REG_DWORD,(const unsigned char *)Data,4);
    isok=RegSetValueEx(hkResult,L"Start",0,REG_DWORD,(const unsigned char *)Data,4);
	
	//isok=RegSetValueEx(hkResult,L"Group",0,REG_SZ,"System Bus Extender",19);
	//isok=RegSetValueEx(hkResult,L"ImagePath",0,REG_SZ,"System32\\Drivers\\REGSYS701.SYS",30);


	// Now set the image path to the driver DLL
    intLen = wsprintf(strDeviceDir,L"System32\\Drivers\\%s",strFile);
    intLen = wsprintf(strSubkey,L"%s",strDeviceDir);
	if(bDebug)wbprintf(0,"[debug] Location - %S\n",strSubkey);
    strSubkey[intLen*2]=0;
    isok=RegSetValueExW(hkResult,L"ImagePath",0,1,(const BYTE *)strSubkey,intLen*2);
    
	// Now set the type
    intLen = wsprintf(strSubkey,L"System Bus Extender");
	if(bDebug)wbprintf(0,"[debug] Driver Type - %S\n",strSubkey);
    strSubkey[intLen*2]=0;
    isok=RegSetValueExW(hkResult,L"Group",0,1,(const BYTE *)strSubkey,intLen*2);
	
	//
	RegCloseKey(hkResult);

	// Now prepare the string to load it
	intLen = sprintf_s(strFoo,MAX_PATH,"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%s",strReg);
	if(bDebug)wbprintf(0,"[debug] %s\n",strFoo);
	if(bDebug)wbprintf(0,"[debug] %s\n",strReg);
    strFoo[intLen]=0;
    buf2.Buffer = (PVOID)strFoo;
    buf2.Length = intLen;
    RtlAnsiStringToUnicodeString(&buf1,&buf2,1);

	// Load the driver
	if(bVerbose && intVerboseLvl >= 0) wbprintf(0,"[i] Loading - %S\n",strExename);
    isok = NtLoadDriver(&buf1);
    RtlFreeUnicodeString(&buf1);
    
	intLen=swprintf_s(strSubkey,MAX_PATH,L"%s%s\\Enum","System\\CurrentControlSet\\Services\\",strExename);
    strSubkey[intLen]=0;

	// Open the device
    hSysFile = CreateFile(strSubkey,GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hSysFile==INVALID_HANDLE_VALUE)
    {
		intLen=swprintf_s(strSubkey,MAX_PATH,L"\\\\.\\Global\\%s",strExename);
		if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] Opening - %S\n",strSubkey);
        strSubkey[intLen]=0;
        *hDevice = CreateFile(strSubkey,GENERIC_READ|GENERIC_WRITE,NULL,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(*hDevice ==INVALID_HANDLE_VALUE){
             wbprintf(1,"[!] Failed to open device - %d\n",GetLastError());
			return false;
		}
    }

	return true;
}

//
// Function	: LoadFilemonDriver
// Role		: Loads the Sysinternals FILEMON device driver
// Notes	: It doesn't unload them / delete them
//
DWORD LoadFilemonDriver()
{
	static	TCHAR strDriverPath[MAX_PATH];
	TCHAR   strPath[MAX_PATH];
	DWORD	dwVersionNumber=0,dwFoo=0;
	TCHAR	strSystemRoot[MAX_PATH];
	TCHAR	strDeviceDir[MAX_PATH];

	// Copy the driver to the correct location
	GetCurrentDirectory( sizeof strPath, strPath );
	swprintf( strPath+lstrlen(strPath),MAX_PATH, _T("\\%s"), SYSF_FILE );
	if( !GetEnvironmentVariable( L"SYSTEMROOT", strSystemRoot, sizeof(strSystemRoot))) {
		return 1;
	}


	swprintf( strDriverPath,MAX_PATH, _T("%s\\system32\\drivers\\%s"), strSystemRoot, SYSF_FILE );
	if( !CopyFile( strPath, strDriverPath, FALSE )) {
		wbprintf(1,"[!] Couldn't copy filemon driver to system root!\n");
		return 1;
	} 


	// Setup and load the driver
	if(RegHandleDev(L"FILEMON701",&hSysFile,SYSF_FILE,"FILEON701",strDeviceDir)==false){
		wbprintf(1,"[!] Loading of filemon driver failed...\n");
		return 1;
	}

	// Print out the version number of the driver
	if (!DeviceIoControl(	hSysFile, IOCTL_FILEMON_VERSION, 
		NULL, 0, &dwVersionNumber, sizeof(DWORD), &dwFoo, NULL )){
		wbprintf(1,"[!] Couldn't get filemon driver version!\n");
		return 1;
	} else {
		wbprintf(0,"[i] Filemon Driver Version: %d\n", dwVersionNumber);
		hgSysFile=hSysFile;
	}
	return 0;
}

//
// Function	: LoadRegmonDriver
// Role		: Loads the Sysinternals REGMON device driver
// Notes	: It doesn't unload them / delete them
//
DWORD LoadRegmonDriver()
{
	static	TCHAR strDriverPath[MAX_PATH];
	TCHAR   strPath[MAX_PATH];
	DWORD	dwVersionNumber=0,dwFoo=0;
	TCHAR	strSystemRoot[MAX_PATH];
	TCHAR	strDeviceDir[MAX_PATH];
	
	// Copy the driver to the correct location
	GetCurrentDirectory( sizeof strPath, strPath );
	swprintf( strPath+lstrlen(strPath),MAX_PATH, _T("\\%s"), SYSR_FILE );
	if( !GetEnvironmentVariable( L"SYSTEMROOT", strSystemRoot, sizeof(strSystemRoot))) {
		return 1;
	}


	swprintf( strDriverPath,MAX_PATH, _T("%s\\system32\\drivers\\%s"), strSystemRoot, SYSR_FILE );
	if( !CopyFile( strPath, strDriverPath, FALSE )) {
		wbprintf(1,"[!] Couldn't copy regmon driver to system root!\n");
		return 1;
	} 


	// Setup and load the driver
	if(RegHandleDev(L"REGMON701",&hSysReg,SYSR_FILE,"REGMON701",strDeviceDir)==false){
		wbprintf(1,"[!] Loading of regmon driver failed...\n");
		return 1;
	}

	// Print out the version number of the driver
	if (!DeviceIoControl(	hSysReg, IOCTL_REGMON_VERSION, 
		NULL, 0, &dwVersionNumber, sizeof(DWORD), &dwFoo, NULL )){
		wbprintf(1,"[!] Couldn't get regmon driver version!\n");
		return 1;
	} else {
		wbprintf(0,"[i] Regmon Driver Version: %d\n", dwVersionNumber);
		hgSysReg=hSysReg;
	}

	return 0;
}

//
// Function	: LoadDrivers
// Role		: Loads the Sysinternals FILEMON and REGMON device drivers
// Notes	: It doesn't unload them / delete them
//
DWORD LoadDrivers(){

	if(LoadFilemonDriver() == 0 && LoadRegmonDriver() == 0){
		return 0;
	} else {
		return 1;
		wbprintf(1,"[!] One of more of the device drivers failed to load!\n");
	}
}