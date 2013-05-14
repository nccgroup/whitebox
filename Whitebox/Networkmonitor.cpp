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
#include "Utils.h"
#include "Networkmonitor.h"
#include "Configuration.h"
#include <pcap.h>

//
// Function	: PrintNetworkDeviceList
// Role		: Prints the availible interfaces to winpcap so the user can choose
// Notes	: 
//
void PrintNetworkDeviceList(){

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;
	char errbuf[PCAP_ERRBUF_SIZE];

	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"[!] Error in pcap_findalldevs: %s\n", errbuf);
		ExitProcess(1);
	}

	fprintf(stdout,"[i] Network device list (please note the number to the left)..\n");

    /* Print the list */
    for(d=alldevs; d; d=d->next)
    {
        printf("%d. %s\n", ++i, d->name);
        if (d->description)
            printf("\t (%s)\n", d->description);
        else
            printf("\t (No description available)\n");
    }

	ExitProcess(0);
}

//
// Function	: PacketHandler
// Role		: Callback function invoked by libpcap for every incoming packet 
// Notes	: 
//
void PacketHandler(u_char *fileDump, const struct pcap_pkthdr *header, const u_char *pkt_data)
{


	wbprintf(0,"[network] traffic\n");
	addactivity("%s\n\x00\x00\x00","[network] traffic");
	
	// If we're logging to our .pcap
	if(*fileDump!=NULL){
		pcap_dump(fileDump, header, pkt_data);
	}
}

//
// Function	: NetworkMonitorThread
// Role		: Sniffer thread entry point
// Notes	: 
//
DWORD WINAPI NetworkMonitorThread(LPVOID lpvParam){

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fcode;
	bpf_u_int32 NetMask;
	pcap_dumper_t *fileDump;

	/*
	int res;
	const u_char *pkt_data;
	time_t local_tv_sec;
	char timestr[16];
	struct tm *ltime;
	struct pcap_pkthdr *header;
	*/	
    
	/* Retrieve the device list */
	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		wbprintf(1,"[!] Error in pcap_findalldevs: %s\n", errbuf);
		return 1;
	}
	
	for(d=alldevs; d; d=d->next) i++;

    if(i==0)
    {
        wbprintf(1,"[!] No interfaces found! Make sure WinPcap is installed.\n");
        return 1;
    }
    
	if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] Using interface %d\n",intNetDevice);

    if(intNetDevice < 1 || intNetDevice > i)
    {
        wbprintf(1,"[!] Network interface number out of range!\n");
		// Free the device list
        pcap_freealldevs(alldevs);
        return 1;
    }
	
    // Jump to the selected adapter 
    for(d=alldevs, i=0; i< intNetDevice-1 ;d=d->next, i++);
    
	// Open the adapter 
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		wbprintf(1,"[!] Unable to open the adapter. %s is not supported by WinPcap\n", d->name);
		//  Free the device list
		pcap_freealldevs(alldevs);
		return 1;
	}

	if(d->addresses != NULL)
		// Retrieve the mask of the first address of the interface 
		NetMask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		// If the interface is without addresses we suppose to be in a C class network 
		NetMask=0xffffff; 

	if(bVerbose && intVerboseLvl>=5) wbprintf(0,"[i] Netmask being used is %08X\n",NetMask);

	char strFilter[1024];
	memset(strFilter,0x00,1024);

	sprintf_s(strFilter,1024,"%s",strNetwExcludeFilter);
	if(bVerbose && intVerboseLvl>=5) wbprintf(0,"[i] Network filter being used is '%s'\n",strFilter);

	// compile the filter
	if(pcap_compile(adhandle, &fcode, strFilter, 1, NetMask) < 0)
	{
		wbprintf(1,"[!] Error compiling pcap filter: wrong syntax\n");
		pcap_close(adhandle);
		return 1;
	}

	// set the filter
	if(pcap_setfilter(adhandle, &fcode)<0)
	{
		wbprintf(1,"[!] Error setting the pcap filter\n");
		pcap_close(adhandle);
		return 1;
	}
    
    if(bVerbose && intVerboseLvl >= 5) wbprintf(0,"[i] sniffer listening on %s...\n", d->description);
	
	wbprintf(0,"[*] Sniffer active\n");

    // At this point, we don't need any more the device list. Free it
    pcap_freealldevs(alldevs);
	
	// Open PCAP File
	if(bReportEnabled){
		char strPCAPLog[MAX_PATH];
		sprintf_s(strPCAPLog,MAX_PATH,"%S\\network.report.pcap",strOutDir);
		fileDump = pcap_dump_open(adhandle, strPCAPLog);
		if(fileDump==NULL){
			wbprintf(1,"[!] Error opening PCAP dump file %s!\n",strPCAPLog);
			pcap_close(adhandle);
			return 1;
		}
	}

	pcap_loop(adhandle, 0, PacketHandler, (unsigned char *)fileDump);

	while(1){
		Sleep(500);
	}
	
   pcap_close(adhandle);  
   return 0;
}