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
#include "Engine.h"            // Our foo
#include "Utils.h"             // General Windows/Logging foo
#include "Filemonitor.h"       // Sysinternal's FileMonitor driver foo
#include "Registrymonitor.h"   // Sysinternal's RegMon driver foo
#include "Networkmonitor.h"    // WinPCap foo
#include "Configuration.h"     // XML foo
#include "Drivers.h"           // Driver foo

// Globals 
SOCKET socketClient=INVALID_SOCKET;
HANDLE hgSysFile = INVALID_HANDLE_VALUE;
HANDLE hgSysReg = INVALID_HANDLE_VALUE;
HANDLE hExitEvent=NULL; 
char   *strActivity=NULL;

//
// Function	: ProcessFile
// Role		: Process the file we're told to by the Job Server
// Notes	: This is designed to support one concurrent TCP connection
//
DWORD WINAPI ProcessFile(char *strFile, int intLen){

	DWORD dwCount=0;
	BOOL  bFound=false;
	TCHAR strFrom[MAX_PATH];
	TCHAR strToTemp[MAX_PATH];
	TCHAR strToOut[MAX_PATH];

	if(intLen<0) return 1;


	// Ensure it's NULL terminated (and remove the newline character)
	for(dwCount=0;dwCount<(DWORD)intLen;dwCount++){
		if(strFile[dwCount] == '\n'){
			bFound=true;
			strFile[dwCount]= '\x00';
			break;
		}
	}

	if(!bFound) return 1;

	memset(strActivity,0x00,ACTIVITY_SIZE);

	if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] ProcessFile is processing %s.. \n",strFile);

	swprintf_s(strFrom,MAX_PATH,L"%s\\%S",strInDir,strFile);
	swprintf_s(strToTemp,MAX_PATH,L"%s\\%S",strTempDir,strFile);
	swprintf_s(strToOut,MAX_PATH,L"%s\\%S",strOutDir,strFile);

	if((bVerbose && intVerboseLvl >= 10) || bDebug) wbprintf(0,"[i] Copying from %S to %S.. \n",strFrom,strToTemp);

	CopyFile(strFrom,strToTemp,false);

	SHELLEXECUTEINFO shellexeInfo;
	shellexeInfo.cbSize=sizeof(SHELLEXECUTEINFO);
	shellexeInfo.lpDirectory=L"C:\\"; // this should probably be configurable
	shellexeInfo.lpVerb=L"open";
	shellexeInfo.lpFile=strToTemp;
	shellexeInfo.nShow=SW_SHOWNORMAL;
	shellexeInfo.lpIDList=0;
	shellexeInfo.lpParameters=NULL;
	shellexeInfo.hInstApp=0;
	shellexeInfo.fMask=SEE_MASK_NOCLOSEPROCESS |SEE_MASK_NOASYNC;

	if(ShellExecuteEx(&shellexeInfo)==false){
		wbprintf(1,"[!] Could not launch application\n");
		return 1;
	}

	// Sleep some 20 seconds
	Sleep(10000);

	if(shellexeInfo.hProcess!=NULL) TerminateProcess(shellexeInfo.hProcess,0);
	else {
		wbprintf(1,"[!] Could not terminate process %08x\n",shellexeInfo.hProcess);
		CloseHandle(shellexeInfo.hProcess);
		return 1;
	}

	// Did we detect the file as malicous?
	if(strlen(strActivity)>1){
		CopyFile(strFrom,strToOut,false);
		DeleteFile(strToTemp);
		wbprintf(0,"[investigate] %s had malicous activity\n",strFile);
		if(bReportEnabled)WriteReport(strFile);
		return 2;
	} else {
		DeleteFile(strToTemp);
	}

	return 0;
}

//
// Function	: JobServer
// Role		: Wait to be told to do a job
// Notes	: This is designed to support one concurrent TCP connection
//
DWORD WINAPI JobServerThread(LPVOID lpvParam){

	WSADATA wsaData;
    SOCKET socketListen = INVALID_SOCKET;
    struct addrinfo *Result = NULL, hints;
    char strRecvBuf[DEFAULT_BUFLEN];
    int dwRes=0, dwSendRes=0;
    int intRecvBufLen = DEFAULT_BUFLEN;
    
	if(bVerbose && intVerboseLvl > 0) wbprintf(0,"[i] JobServer starting... \n");

    // Initialize Winsock
    dwRes= WSAStartup(MAKEWORD(2,2), &wsaData);
    if (dwRes!= 0) {
        wbprintf(1,"[!] WSAStartup failed: %d\n", dwRes);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char strASCIITCPPort[6];
	sprintf_s(strASCIITCPPort,6,"%S",strTCPPort);
	if(wcslen(strTCPPort)>0) dwRes = getaddrinfo(NULL, strASCIITCPPort, &hints, &Result);
    else dwRes = getaddrinfo(NULL, DEFAULT_PORT, &hints, &Result);

    if ( dwRes != 0 ) {
        wbprintf(1,"[!] getaddrinfo failed: %d\n", dwRes);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    socketListen = socket(Result->ai_family, Result->ai_socktype, Result->ai_protocol);
    if (socketListen == INVALID_SOCKET) {
        wbprintf(1,"[!] socket failed: %ld\n", WSAGetLastError());
        freeaddrinfo(Result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    dwRes = bind( socketListen, Result->ai_addr, (int)Result->ai_addrlen);
    if (dwRes == SOCKET_ERROR) {
        wbprintf(1,"[!] bind failed: %d\n", WSAGetLastError());
        freeaddrinfo(Result);
        closesocket(socketListen);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(Result);

    dwRes = listen(socketListen, SOMAXCONN);
    if (dwRes == SOCKET_ERROR) {
        wbprintf(1,"[!] listen failed: %d\n", WSAGetLastError());
        closesocket(socketListen);
        WSACleanup();
        return 1;
    }

	if(wcslen(strTCPPort)>0) wbprintf(0,"[*] Job server active (%d/TCP)\n",atoi(strASCIITCPPort));
	else wbprintf(0,"[*] Job server active (%d/TCP)\n",atoi(DEFAULT_PORT));

	while(1){
		// Accept a client socket
		socketClient = accept(socketListen, NULL, NULL);
		if (socketClient == INVALID_SOCKET) {
			wbprintf(1,"[!] accept failed: %d\n", WSAGetLastError());
			closesocket(socketListen);
			WSACleanup();
			return 1;
		}


		dwSendRes = send( socketClient, "OK\n", (int)strlen("OK\n"), 0 );
		if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);

		// Receive until the peer shuts down the connection
		do {

			dwRes = recv(socketClient, strRecvBuf, intRecvBufLen, 0);
			if (dwRes > 0) {
				if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes received: %d\n", dwRes);	


				// Shutdown command
				if(strstr(strRecvBuf,"SHUTDOWN")>0) {
					SetEvent(hExitEvent);
				} else if (strstr(strRecvBuf,"REBOOT")>0){
					 ExitWindowsEx(EWX_REBOOT | EWX_FORCEIFHUNG, SHTDN_REASON_MAJOR_SOFTWARE | SHTDN_REASON_MINOR_SECURITY);
				} else {
					// This is blocking
					DWORD dwProcRet=0;
					dwProcRet=ProcessFile(strRecvBuf,dwRes);
					if(dwProcRet==1){
						dwSendRes = send( socketClient, "FAILED\n", (int)strlen("FAILED\n"), 0 );
						if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);
					} else if(dwProcRet==2) {
						if(strlen(strActivity)>1) {
							dwSendRes = send( socketClient, strActivity, (int)strlen(strActivity), 0 );
							if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);
						}
						dwSendRes = send( socketClient, "ACTIVITY\n", (int)strlen("ACTIVITY\n"), 0 );
						if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);
					} else {
						dwSendRes = send( socketClient, "PROCESSED\n", (int)strlen("PROCESSED\n"), 0 );
						if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);
					}

					/*
					// This just echos
					dwSendRes = send( socketClient, strRecvBuf, dwRes, 0 );
					if (dwSendRes == SOCKET_ERROR) {
						wbprintf(1,"[!] send failed: %d\n", WSAGetLastError());
						closesocket(socketClient);
						WSACleanup();
						return 1;
					}
					if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] bytes sent: %d\n", dwSendRes);
					*/
				}
			} else if (dwRes== 0) {
				wbprintf(0,"[i] Connection closing...\n");
			} else  {
				//wbprintf(1,"[!] recv failed: %d\n", WSAGetLastError());
				closesocket(socketClient);
				//WSACleanup();
				//return 1;
			}

		} while (dwRes> 0);

		// shutdown the connection since we're done
		dwRes = shutdown(socketClient, SD_SEND);
		if (dwRes == SOCKET_ERROR) {
			//wbprintf(1,"[!] shutdown failed: %d\n", WSAGetLastError());
			closesocket(socketClient);
			//WSACleanup();
			//return 1;
		}

		// cleanup
		closesocket(socketClient);
	}

	// No longer need server socket
    closesocket(socketListen);


	WSACleanup();

	if(bVerbose && intVerboseLvl > 0) wbprintf(0,"[i] JobServer exiting... \n");

	return 0;
}

//
// Function	: WhiteboxEngine
// Role		: Main engine entry point
// Notes	: 
//
int WhiteboxEngine(){

	HANDLE hThreadFile=NULL;
	HANDLE hThreadReg=NULL;
	HANDLE hThreadNet=NULL;
	HANDLE hThreadJob=NULL;
	
	strActivity=(char *)malloc(ACTIVITY_SIZE);
	memset(strActivity,0x00,ACTIVITY_SIZE);

	if(strActivity==NULL) {
		wbprintf(1,"[!] Failed to allocate the required memory...\n");
		return 1;
	}

	hExitEvent = CreateEvent(NULL,TRUE,FALSE,L"Whitebox.Exit");

	if(bVerbose && intVerboseLvl > 0) wbprintf(0,"[i] Whitebox Engine %s...\n","starting");

	// Load the configuration
	if(strConfFile!=NULL){
		if(bDebug) wbprintf(0,"[debug] Loading configuration\n");
		if(LoadConfiguration(strConfFile)!=0){
			wbprintf(1,"[!] Configuration failed!\n");
			return 1;
		}
	}

	// Load the kernel drivers
	if(bDebug) wbprintf(0,"[debug] Loading drivers\n");
	if(LoadDrivers()!=0){
		wbprintf(1,"[!] Loading drivers failed!\n");
		return 1;
	}

	// Load and configure the network monitor thread
	if(bNetworkEngine) hThreadNet = CreateThread(NULL,0,NetworkMonitorThread,NULL,0,NULL);
	else wbprintf(0,"[i] Network monitoring engine disabled\n");

	// Load and configure the file monitor thread
	if(bFilemonEngine) hThreadFile = CreateThread(NULL,0,FileMonitorThread,NULL,0,NULL);
	else wbprintf(0,"[i] File monitoring engine disabled\n");

	// Load and configure the registry monitor thread
	if(bRegEngine) hThreadReg = CreateThread(NULL,0,RegMonitorThread,NULL,0,NULL);
	else wbprintf(0,"[i] Registry monitoring engine disabled\n");

	// Start the job server thread
	hThreadJob=CreateThread(NULL,0,JobServerThread,NULL,0,NULL);

	/*
	// Now we go into a deep sleep until signaled
	while(1){
		Sleep(10000);
	}
	*/

	WaitForSingleObject(hExitEvent,INFINITE);

	wbprintf(0,"[i] Whitebox Engine received exit event, shutdown started...\n");

	// Kill the threads
	if(bNetworkEngine && hThreadNet !=NULL) TerminateThread(hThreadNet,0);
	if(bFilemonEngine && hThreadFile !=NULL) TerminateThread(hThreadFile,0);
	if(bRegEngine  && hThreadReg !=NULL) TerminateThread(hThreadReg,0);
	if(hThreadJob!=NULL) TerminateThread(hThreadJob,0);


	if(bVerbose && intVerboseLvl > 0) wbprintf(0,"[i] Whitebox Engine %s...\n","exiting");

	return 0;
}