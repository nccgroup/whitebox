/*
Whitebox Proto-type

Released as open source by NCC Group Plc - http://www.nccgroup.com/

Developed by Ollie Whitehouse, ollie dot whitehouse at nccgroup dot com

http://www.github.com/nccgroup/whitebox

Released under AGPL see LICENSE for more information

(c) 2008 - 2013 Ollie Whitehouse
(c) 2013 NCC Group Plc
*/

#include "stdafx.h"			// Standard pre compiled header
#include "XGetOpt.h"		// Command line processing
#include "Utils.h"			// General windows foo
#include "Engine.h"			// Whitebox engine
#include "Networkmonitor.h" // Network monitoring engine

bool bDebug=false;
bool bVerbose=false;
bool bLog=false;
unsigned int intVerboseLvl=0;
TCHAR *strLogFile=NULL;
FILE *fileLog=NULL;
TCHAR *strConfFile=NULL;
FILE *fileConf=NULL;
TCHAR *strInDir=NULL;
TCHAR *strOutDir=NULL;
TCHAR *strTempDir=NULL;
int intNetDevice=0;


//
// Function : PrintBanner
// Role		:
// Notes	:
//
void PrintBanner(){

	fwprintf(stdout,L"WhiteBox - Version v%d.%d\n",intVerMaj,intVerMin);
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
	fwprintf (stdout,L"    %s [-v level] [-d] [-l logfile]\n",strExe);
	fwprintf (stdout,L"  e.g.\n");
	fwprintf (stdout,L"    %s -v 10 -d -l C:\\Log.txt\\ -i c:\\ -o c:\\out\\ -n 1\n",strExe);
	fwprintf (stdout,L"  Descriptions:\n");
	fwprintf (stdout,L"    [-v level]     - verbose mode and level (0 to 10)\n");
	fwprintf (stdout,L"    [-d]           - debug mode\n");
	fwprintf (stdout,L"    [-p]           - print the network interfaces and exit\n");
	fwprintf (stdout,L"    [-l logfile]   - log file\n");
	/*
	fwprintf (stdout,L"    [-i dir]       - input dir to pick up files from\n");
	fwprintf (stdout,L"    [-o dir]       - output dir to write suspicious files to\n");
	fwprintf (stdout,L"    [-t dir]       - local temp dir to write files to before loading\n");
	*/
	fwprintf (stdout,L"    [-c file]      - XML configuration file\n");
	fwprintf (stdout,L"    [-n interface] - network interface to sniff on\n");
	/*
	fwprintf (stdout,L"    [-f string]    - PCAP filter string\n");
	*/
	fwprintf (stdout,L"\n");

	ExitProcess(1);
}

//
// Function	: _tmain
// Role		: Entry point to application
// Notes	: 
//
int _tmain(int argc, _TCHAR* argv[])
{
	
	int	chOpt;
	int	intOptHelp=0, intOptPrintNetDevs=0, intOptInDir=0, intOptOutDir=0, intOptNet=0;
	int	intOptTempDir=0, intOptConfig=0;

	// Extract all the options
	while ((chOpt = getopt(argc, argv, _T("c:f:n:i:o:t:l:v:dhp"))) != EOF) 
	switch(chOpt)
	{
		case _T('f'): // PCAP filter
			break;
		case _T('d'): // Debugging Mode
			bDebug=true;
			break;
		case _T('v'): // Verbose Mode
			bVerbose=true;
			intVerboseLvl=_wtol(optarg);
			break;
		case _T('l'): // Log File
			bLog=true;
			strLogFile=optarg;
			break;
		case _T('i'): // In directory
			intOptInDir=1;
			strInDir=optarg;
			break;
		case _T('o'): // Out directory
			intOptOutDir=1;
			strOutDir=optarg;
			break;
		case _T('n'): // Network device to sniff
			intOptNet=1;
			intNetDevice=_wtol(optarg);
			break;
		case _T('t'): // Temp directory
			intOptTempDir=1;
			strTempDir=optarg;
			break;
		case _T('c'): // Config files
			intOptConfig=1;
			strConfFile=optarg;
			break;
		case _T('h'): // Help
			intOptHelp=1;
			break;
		case _T('p'): // Print network devices
			intOptPrintNetDevs=1;
			break;
		default:
			fwprintf(stderr,L"[!] No handler - %c\n", chOpt);
			break;
	}

	// Print the banner
	PrintBanner();

	// Just print and exit
	if(intOptPrintNetDevs) PrintNetworkDeviceList();

	// Input validation
	if((!intOptInDir || !intOptOutDir || !intOptTempDir || !intOptNet) && !intOptConfig) {
		if(!intOptInDir) fprintf(stderr,"[!] Need to specify the input directory OR configuration file\n");
		if(!intOptOutDir) fprintf(stderr,"[!] Need to specify the output directory OR configuration file\n");
		if(!intOptTempDir) fprintf(stderr,"[!] Need to specify the temporary directory OR configuration file\n");
		if(!intOptNet) fprintf(stderr,"[!] Need to specify the network interface to use OR configuration file\n");
		fprintf(stderr,"\n");
		PrintHelp(argv[0]);
	}

	// Print help if required
	if(intOptHelp) PrintHelp(argv[0]);

	

	// Print some flag status
	if(!bDebug) fwprintf(stdout,L"[i] Debugging: Off\n"); 
	else fwprintf(stdout,L"[i] Debugging: On\n");
	if(!bVerbose) fwprintf(stdout,L"[i] Verbose: Off\n"); 
	else fwprintf(stdout,L"[i] Verbose: On (Level %u)\n",intVerboseLvl);
	if(intOptInDir)   fwprintf(stdout,L"[i] Input Directory : %s\n",strInDir);
    if(intOptOutDir)  fwprintf(stdout,L"[i] Output Directory: %s\n",strOutDir);
	if(intOptTempDir) fwprintf(stdout,L"[i] Temporary Directory: %s\n",strTempDir);

	// Get the required privs
	if(SetPrivilege(GetCurrentProcess(),L"SeLoadDriverPrivilege")){
		fwprintf(stdout,L"[*] Privilege obtained - SeLoadDriverPrivilege\n");
	} else{
		fwprintf(stderr,L"[!] Failed to obtain privilege - SeLoadDriverPrivilege\n");
		return 1;
	}

	// Get some Windows privs - needs to be run as administrator / 
	// be granted these as required
	if(SetPrivilege(GetCurrentProcess(),L"SeDebugPrivilege")){
		fwprintf(stdout,L"[*] Privilege obtained - SeDebugPrivilege\n");
	} else{
		fwprintf(stderr,L"[!] Failed to obtain privilege - SeDebugPrivilege\n");
		return 1;
	}

	// Open the log file if required
	if(bVerbose && intVerboseLvl >= 5) fwprintf(stdout,L"[i] Log file chosen %s..\n",strLogFile);
    if(bLog && strLogFile!=NULL){
		if(OpenLog(strLogFile)){
			fwprintf(stdout,L"[*] Log file opened\n");
		} else {
			fwprintf(stderr,L"[!] Failed to open log file '%s' - !\n",strLogFile);
			return 1;
		}
	}


	// Call the main engine (vrrrrm!!)
	if(WhiteboxEngine()!=0){
		wbprintf(1,"[!] Engine suffered an unrecoverable error!\n");
		return 1;
	} else {
		wbprintf(0,"[!] Engine exited cleanly..\n");
	}

	//Close the log file if required
	if(bLog && fileLog!=NULL){
		if(CloseLog()){
			fwprintf(stdout,L"[*] Log file closed\n");
		} else {
			fwprintf(stderr,L"[!] Failed to close log file '%s' - !\n",strLogFile);
		}
	}

		
	return 0;
}

