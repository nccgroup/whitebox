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
#include "XGetOpt.h"

TCHAR *strInDir=NULL;
TCHAR *strProcessedDir=NULL;
TCHAR *strServer=NULL;
char strServerConv[1024];
TCHAR *strPort=NULL;
SOCKET ConnectSocket;

//
// Function : PrintBanner
// Role		:
// Notes	:
//
void PrintBanner(){

	fwprintf(stdout,L"WhiteBox Controller - Version v%d.%d\n",intVerMaj,intVerMin);
	fwprintf(stdout,L"(c) 2008, 2009 - Ollie Whitehouse\n");
	fwprintf(stdout,L"\n");
}
//
// Function : PrintHelp
// Role		:
// Notes	:
//
void PrintHelp(TCHAR *strExe)
{
	fwprintf (stdout,L"  [.Help.]\n");
	fwprintf (stdout,L"  Usage:\n");
	fwprintf (stdout,L"    %s [-s server] [-p port] [-i inputdirectory][-d processeddirectory]\n",strExe);
	fwprintf (stdout,L"  e.g.\n");
	fwprintf (stdout,L"    %s -s 192.168.0.1 -p 911 -i \\\\server\\input\\ \n",strExe);
	fwprintf (stdout,L"  Descriptions:\n");
	fwprintf (stdout,L"    [-s server]     - server\n");
	fwprintf (stdout,L"    [-p port]       - port to connect to on the server\n");
	fwprintf (stdout,L"    [-i directory]  - input directory (shared)\n");
	fwprintf (stdout,L"    [-d directory]  - all processed files\n");
	fwprintf (stdout,L"    [-h]            - help\n");
	fwprintf (stdout,L"\n");

	ExitProcess(1);
}


DWORD ServerConnect(){

	//----------------------
	// Create a SOCKET for connecting to server
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		fprintf(stderr,"[!] Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	//----------------------
	// The sockaddr_in structure specifies the address family,
	// IP address, and port of the server to be connected to.
	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr( strServerConv );
	clientService.sin_port = htons( _wtoi(strPort));

	//----------------------
	// Connect to server.
	if ( connect( ConnectSocket, (SOCKADDR*) &clientService, sizeof(clientService) ) == SOCKET_ERROR) {
		fprintf(stderr,"[!] Failed to connect.\n" );
		WSACleanup();
		return 1;
	}

	fprintf(stdout,"[i] Connected to server.\n");
	 
	return 0;
}

// Function : NetworkProcess
// Role     : Handles server communications
// Notes    :
//
DWORD NetworkProcess(TCHAR *strFile){

	char strTX[1024];
	char strRX[1024];
	DWORD dwSendRes=0;
	int dwRes=0;
	int intRecvBufLen=1024;

	if(ConnectSocket!=NULL){
		fprintf(stdout,"[i] Processing %S!\n",strFile);

		dwRes = recv(ConnectSocket, strRX, intRecvBufLen, 0);
		if(strstr(strRX,"OK")==0){
			fprintf(stderr,"[!] Didn't recieve OK from server!\n");
			return 1;
		}
		
		memset(strTX,0x00,1024);
		sprintf_s(strTX,1024,"%S\n",strFile);
		//fprintf(stdout,"[i] sending - %s", strTX);	
		dwSendRes = send( ConnectSocket, strTX, (int)strlen(strTX), 0 );
		
		// Receive until the peer shuts down the connection
		do {
			memset(strRX,0x00,1024);
			dwRes = recv(ConnectSocket, strRX, intRecvBufLen, 0);
			if (dwRes > 0) {
				//fprintf(stdout,"[i] bytes received: %d\n", dwRes);	
				//fprintf(stdout,"[i] recieved - %s\n", strRX);	
				if(strstr(strRX,"FAILED")>0){
					fprintf(stdout,"[!] %S WAS NOT processed\n", strFile);	
				} else if(strstr(strRX,"PROCESSED")>0){
					fprintf(stdout,"[i] %S was processed successfully\n", strFile);
				} else if(strstr(strRX,"ACTIVITY")>0){
					fprintf(stdout,"[i] %S showed some activity\n", strFile);
				} else {
					fprintf(stdout,"[i] ------------- acivity log ------------- \n%s[i] ----------- end activity log ----------\n", strRX);
				}
				shutdown(ConnectSocket, SD_SEND);
				closesocket(ConnectSocket);
				ConnectSocket=NULL;
				return 0;
			} else if (dwRes== 0) {
				//fprintf(stdout,"[i] Connection closing...\n");
				//shutdown(ConnectSocket, SD_SEND);
				closesocket(ConnectSocket);
				ConnectSocket=NULL;
				return 0;
			} else  {
				//fprintf(stderr,"[!] recv failed: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				ConnectSocket=NULL;
				return 1;
			}

		} while (dwRes> 0);

		closesocket(ConnectSocket);
		ConnectSocket=NULL;
	} else {
		fprintf(stderr,"[!] Client not connected could not process %S!\n",strFile);
		return 1;
	}

	return 0;
}

//
// Function	: Process
// Role		: Finds the files and establishes the server connection
// Notes	: 
//
DWORD Process(){

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	TCHAR strInDirFull[MAX_PATH];
	
	//----------------------
	// Initialize Winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != NO_ERROR) fprintf(stderr,"[!] Error at WSAStartup()\n");

	// Add the backslashes
	swprintf_s(strInDirFull,MAX_PATH,L"%s\\*",strInDir);


	// Loop through files which are there
	hFind = FindFirstFile(strInDirFull, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) 
	{
      fprintf(stderr,"[!] FindFirstFile failed!");
      return 1;
	} 
   
	// List all the files in the directory with some info about them.
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// fwprintf(stdout,L"Directory - %s\n", ffd.cFileName);
		} else {
			if(ServerConnect()!=0){
				while(ServerConnect()!=0){
					fwprintf(stdout,L"[i] Retrying connection...\n", ffd.cFileName);
					Sleep(5000);
				}
				fwprintf(stdout,L"[i] Processing file - %s\n", ffd.cFileName);
				NetworkProcess(ffd.cFileName);
			} else {
				fwprintf(stdout,L"[i] Processing file - %s\n", ffd.cFileName);
				NetworkProcess(ffd.cFileName);
			}
		}
	} while (FindNextFile(hFind, &ffd) != 0);
 
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) 
	{
        fprintf(stderr,"[!] FindFirstFile failed!");
		FindClose(hFind);
		return 1;
	} else {
		FindClose(hFind);
		WSACleanup();
		return 0;
	}
}

//
// Function	: _tmain
// Role		: Entry point to application
// Notes	: 
//
int _tmain(int argc, _TCHAR* argv[])
{
	int	chOpt;
	int	intOptHelp=0, intOptInDir=0, intOptProcessedDir=0, intOptServer=0, intOptPort=0;

	// Extract all the options
	while ((chOpt = getopt(argc, argv, _T("i:d:s:p:h"))) != EOF) 
	switch(chOpt)
	{
		case _T('i'): // Input directory
			intOptInDir=true;
			strInDir=optarg;
			break;
		case _T('d'): // Processed (dump) directory
			intOptProcessedDir=true;
			strProcessedDir=optarg;
			break;
		case _T('p'): // Port
			intOptPort=true;
			strPort=optarg;
			break;
		case _T('s'): // Server
			intOptServer=true;
			strServer=optarg;
			break;
		case _T('h'): // Help
			intOptHelp=1;
			break;
		default:
			fwprintf(stderr,L"[!] No handler - %c\n", chOpt);
			break;
	}

	PrintBanner();

	if(!intOptInDir || !intOptPort || !intOptServer || intOptHelp){
		PrintHelp(argv[0]);
	}

	if(_wtoi(strPort)>65535 || _wtoi(strPort) <0){
		fwprintf(stderr,L"[!] Invalid port number\n");
	}

	sprintf_s(strServerConv,1024,"%S",strServer);

	while(1){
		Process();
		Sleep(30000);
	}


	return 0;
}

