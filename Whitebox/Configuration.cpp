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
#include "utils.h"
#include <atlbase.h>
#include "xmllite.h"
#include <strsafe.h>
#include "Configuration.h"

// Config engine state machine
bool bWhiteBox=false;
bool bFilemonitor=false;
bool bRegistrymonitor=false;
bool bNetworkmonitor=false;
bool bEngine=false;
bool bInclude=false;
bool bExclude=false;
bool bTCPPort=false;
bool bInDir=false;
bool bOutDir=false;
bool bTempDir=false;
bool bInterface=false;
bool bEnabled=false;
bool bReport=false;


// Settings
bool bNetworkEngine=true;
bool bFilemonEngine=true;
bool bRegEngine=true;
bool bReportEnabled=true;
char strFileExcludeFilter[1024];
char strFileIncludeFilter[1024];
char strRegiExcludeFilter[1024];
char strRegiIncludeFilter[1024];
char strNetwExcludeFilter[1024];
TCHAR strConfInDir[MAX_PATH];
TCHAR strConfOutDir[MAX_PATH];
TCHAR strConfTempDir[MAX_PATH];
TCHAR strTCPPort[6];

DWORD LoadConfiguration(TCHAR *strFile){
	HRESULT hr;
    CComPtr<IStream> pFileStream;
    CComPtr<IXmlReader> pReader;
    XmlNodeType nodeType;
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    const WCHAR* pwszValue;
    UINT cwchPrefix;

    //Open read-only input stream
    if (FAILED(hr = SHCreateStreamOnFile(strFile, STGM_READ, &pFileStream)))
    {
        wprintf(L"[!] Error creating file reader, error is %08.8lx", hr);
        return 1;
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"[!] Error creating xml reader, error is %08.8lx", hr);
        return 1;
    }

    if (FAILED(hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit)))
    {
        wprintf(L"[!] Error setting XmlReaderProperty_DtdProcessing, error is %08.8lx", hr);
        return 1;
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"[!] Error setting input for reader, error is %08.8lx", hr);
        return 1;
    }

    //read until there are no more nodes
    while (S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch (nodeType)
        {
			// We don't use this
			case XmlNodeType_XmlDeclaration:
				break;

			// This is the start of an element
			case XmlNodeType_Element:
				if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
				{
					wprintf(L"[!] Error getting prefix, error is %08.8lx", hr);
					return 1;
				}
				if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
				{
					wprintf(L"[!] Error getting local name, error is %08.8lx", hr);
					return 1;
				}
	            
				if(bDebug) wprintf(L"[debug] Element: %s\n", pwszLocalName);

				if(wcsstr(pwszLocalName,L"whitebox")>0) bWhiteBox=true;
				else if (wcsstr(pwszLocalName,L"filemonitor")>0) bFilemonitor=true;
				else if (wcsstr(pwszLocalName,L"includefilter")>0) bInclude=true;
				else if (wcsstr(pwszLocalName,L"excludefilter")>0) bExclude=true;
				else if (wcsstr(pwszLocalName,L"registrymonitor")>0) bRegistrymonitor=true;
				else if (wcsstr(pwszLocalName,L"networkmonitor")>0) bNetworkmonitor=true;
				else if (wcsstr(pwszLocalName,L"excludeips")>0) bExclude=true;
				else if (wcsstr(pwszLocalName,L"engine")>0) bEngine=true;
				else if (wcsstr(pwszLocalName,L"tcpport")>0) bTCPPort=true;
				else if (wcsstr(pwszLocalName,L"indir")>0) bInDir=true;
				else if (wcsstr(pwszLocalName,L"outdir")>0) bOutDir=true;
				else if (wcsstr(pwszLocalName,L"tempdir")>0) bTempDir=true;
				else if (wcsstr(pwszLocalName,L"interface")>0) bInterface=true;
				else if (wcsstr(pwszLocalName,L"enabled")>0) bEnabled=true;
				else if (wcsstr(pwszLocalName,L"report")>0) bReport=true;
			
				break;

			// This is the end of an element
			case XmlNodeType_EndElement:
				if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
				{
					wprintf(L"[!] Error getting prefix, error is %08.8lx", hr);
					return 1;
				}
				if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
				{
					wprintf(L"[!] Error getting local name, error is %08.8lx", hr);
					return 1;
				}
			
				if(bDebug) wprintf(L"[debug] End Element: %s\n", pwszLocalName);

				if(wcsstr(pwszLocalName,L"whitebox")>0) bWhiteBox=false;
				else if (wcsstr(pwszLocalName,L"filemonitor")>0) bFilemonitor=false;
				else if (wcsstr(pwszLocalName,L"includefilter")>0) bInclude=false;
				else if (wcsstr(pwszLocalName,L"excludefilter")>0) bExclude=false;
				else if (wcsstr(pwszLocalName,L"registrymonitor")>0) bRegistrymonitor=false;
				else if (wcsstr(pwszLocalName,L"networkmonitor")>0) bNetworkmonitor=false;
				else if (wcsstr(pwszLocalName,L"excludeips")>0) bExclude=false;
				else if (wcsstr(pwszLocalName,L"engine")>0) bEngine=false;
				else if (wcsstr(pwszLocalName,L"tcpport")>0) bTCPPort=false;
				else if (wcsstr(pwszLocalName,L"indir")>0) bInDir=false;
				else if (wcsstr(pwszLocalName,L"outdir")>0) bOutDir=false;
				else if (wcsstr(pwszLocalName,L"tempdir")>0) bTempDir=false;
				else if (wcsstr(pwszLocalName,L"interface")>0) bInterface=false;
				else if (wcsstr(pwszLocalName,L"enabled")>0) bEnabled=false;
				else if (wcsstr(pwszLocalName,L"report")>0) bReport=false;

				break;

			// This is the contents of an element
			case XmlNodeType_Text:
			case XmlNodeType_Whitespace:
				if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
				{
					wprintf(L"[!] Error getting value, error is %08.8lx", hr);
					return 1;
				}

				if(wcsstr(pwszValue,L"\n")==0){
					if(bDebug)wprintf(L"[debug] Text: %s\n", pwszValue);
					
					if(bWhiteBox && bFilemonitor && bInclude) sprintf_s(strFileIncludeFilter,1024,"%S",pwszValue);
					else if(bWhiteBox && bFilemonitor && bExclude) sprintf_s(strFileExcludeFilter,1024,"%S",pwszValue);
					else if(bWhiteBox && bRegistrymonitor && bInclude) sprintf_s(strRegiIncludeFilter,1024,"%S",pwszValue);	
					else if(bWhiteBox && bRegistrymonitor && bExclude) sprintf_s(strRegiExcludeFilter,1024,"%S",pwszValue);	
					else if(bWhiteBox && bNetworkmonitor && bExclude) sprintf_s(strNetwExcludeFilter,1024,"%S",pwszValue);
					else if(bWhiteBox && bNetworkmonitor && bInterface) intNetDevice=_wtoi(pwszValue);
					else if(bWhiteBox && bFilemonitor && bEnabled) {
						if(wcsstr(pwszValue,L"false")>0) bFilemonEngine=false;
					}
					else if(bWhiteBox && bRegistrymonitor && bEnabled) {
						if(wcsstr(pwszValue,L"false")>0) bRegEngine=false;
					}
					else if(bWhiteBox && bNetworkmonitor && bEnabled) {
						if(wcsstr(pwszValue,L"false")>0) bNetworkEngine=false;
					}
					else if(bWhiteBox && bEngine && bTCPPort) wcscpy_s(strTCPPort,6,pwszValue);
					else if(bWhiteBox && bEngine && bInDir) wcscpy_s(strConfInDir,MAX_PATH,pwszValue);
					else if(bWhiteBox && bEngine && bOutDir) wcscpy_s(strConfOutDir,MAX_PATH,pwszValue);
					else if(bWhiteBox && bEngine && bTempDir) wcscpy_s(strConfTempDir,MAX_PATH,pwszValue);
					else if(bWhiteBox && bEngine && bReport){
						if(wcsstr(pwszValue,L"false")>0) bReportEnabled=false;
					}
					else {
						return 1;
					}
				}
				break;

			// We don't use this
			case XmlNodeType_CDATA:
				if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
				{
					wprintf(L"[!] Error getting value, error is %08.8lx", hr);
					return 1;
				}
				//wprintf(L"CDATA: %s\n", pwszValue);
				break;

			// We don't use this
			case XmlNodeType_ProcessingInstruction:
				if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
				{
					wprintf(L"[!] Error getting name, error is %08.8lx", hr);
					return 1;
				}
				if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
				{
					wprintf(L"[!] Error getting value, error is %08.8lx", hr);
					return 1;
				}
				//wprintf(L"Processing Instruction name:%S value:%S\n", pwszLocalName, pwszValue);
				break;

			// We don't use this
			case XmlNodeType_Comment:
				if (FAILED(hr = pReader->GetValue(&pwszValue, NULL)))
				{
					wprintf(L"[!] Error getting value, error is %08.8lx", hr);
					return 1;
				}
				//wprintf(L"Comment: %s\n", pwszValue);
				break;
			
				// We don't use this
			case XmlNodeType_DocumentType:
				//wprintf(L"DOCTYPE is not printed\n");
				break;
		}
    }

	if(bVerbose && intVerboseLvl>=5){
		wbprintf(0,"[i] Configuration loaded from file\n");
		wbprintf(0,"[i] Filemon Include    : %s\n",strFileIncludeFilter);
		wbprintf(0,"[i] Filemon Exclude    : %s\n",strFileExcludeFilter);
		wbprintf(0,"[i] Regmon Include     : %s\n",strRegiIncludeFilter);
		wbprintf(0,"[i] Regmon Exclude     : %s\n",strRegiExcludeFilter);
		wbprintf(0,"[i] Network IP Exclude : %s\n",strNetwExcludeFilter);
		wbprintf(0,"[i] Input Directory    : %S\n",strConfInDir);
		wbprintf(0,"[i] Output Directory   : %S\n",strConfOutDir);
		wbprintf(0,"[i] Temp Directory     : %S\n",strConfTempDir);
		wbprintf(0,"[i] Job Mgr TCP Port   : %S\n",strTCPPort);
	}

	// Now update any command supplied paramaters
	if(strInDir!=NULL && wcslen(strConfInDir)>0) {
		wbprintf(0,"[i] Input directory %S overridden by config file with %S\n",strInDir,strConfInDir);
		strInDir=strConfInDir;
	} else if (wcslen(strConfInDir)>0) {
		strInDir=strConfInDir;
		fwprintf(stdout,L"[i] Input Directory : %s\n",strInDir);
	} else {
		if(strInDir==NULL) {
			fwprintf(stderr,L"[!] Need to specify input directory (config or command line)!\n");
			return 1;
		}
		fwprintf(stdout,L"[i] Input Directory : %s\n",strInDir);
	}

	if(strOutDir!=NULL && wcslen(strConfOutDir)>0) {
		wbprintf(0,"[i] Output directory %S overridden by config file with %S\n",strOutDir,strConfOutDir);
		strOutDir=strConfOutDir;
	} else if(wcslen(strConfOutDir)>0) {
		strOutDir=strConfOutDir;
		fwprintf(stdout,L"[i] Output Directory: %s\n",strOutDir);
	} else {
		if(strOutDir==NULL) {
			fwprintf(stderr,L"[!] Need to specify output directory (config or command line)!\n");
			return 1;
		}
		fwprintf(stdout,L"[i] Output Directory: %s\n",strOutDir);
	}

	if(strTempDir!=NULL && wcslen(strConfTempDir)>0) {
		wbprintf(0,"[i] Temp directory %S overridden by config file with %S\n",strTempDir,strConfTempDir);
		strTempDir=strConfTempDir;
	} else if(wcslen(strConfTempDir)>0) {
		strTempDir=strConfTempDir;
		fwprintf(stdout,L"[i] Temporary Directory: %s\n",strTempDir);
	} else {
		if(strTempDir==NULL) {
			fwprintf(stderr,L"[!] Need to specify temporary directory (config or command line)!\n");
			return 1;
		}
		fwprintf(stdout,L"[i] Temporary Directory: %s\n",strTempDir);
	}

	if(intNetDevice==0){
		fwprintf(stderr,L"[!] Need to specify network interface (config or command line)!\n");
		return 1;
	}


	/*
	if(wcslen(strFileExcludeFilter)>0) 
	if(wcslen(strRegiIncludeFilter)>0) 
	if(wcslen(strRegiExcludeFilter)>0) 
	if(wcslen(strNetwExcludeFilter)>0) 
	if(wcslen(strTCPPort)>0) 
	*/

	// Configuration validation

    return 0;

}