#include "NPClickOnce.h"
#include "NPApplicationLauncher.h"

// Exported functions
NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs)
{
	if (pFuncs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	pFuncs->version = (NP_VERSION_MAJOR <<8) | NP_VERSION_MINOR;
	pFuncs->newp = NPP_New;
	pFuncs->destroy = NPP_Destroy;
	pFuncs->setwindow = NPP_SetWindow;
	pFuncs->newstream = NPP_NewStream;
	pFuncs->destroystream = NPP_DestroyStream;
	pFuncs->asfile = NPP_StreamAsFile;
	pFuncs->writeready = NPP_WriteReady;
	pFuncs->write = NPP_Write;
	pFuncs->print = NPP_Print;
	pFuncs->javaClass = NULL;
	pFuncs->getvalue = NPP_GetValue;
	pFuncs->urlnotify = NPP_URLNotify;

	return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize(NPNetscapeFuncs* pFuncs)
{
	if (pFuncs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	// if the plugin's major ver level is lower than the Navigator's,
	// then they are incompatible, and should return an error
	if (HIBYTE(pFuncs->version) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	g_pPluginFuncs = pFuncs;

	return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown()
{
	return NPERR_NO_ERROR;
}

//
// Functions that the plugin must implement and return via NP_GetEntryPoints() to the browser
//

NPError NPP_New(NPMIMEType pluginType,NPP instance,uint16_t mode,int16_t argc,char* argn[],char* argv[],NPSavedData* saved)
{
	return NPERR_NO_ERROR;
}

NPError NPP_Destroy(NPP instance,NPSavedData** save)
{
	return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance,NPWindow* pNPWindow)
{
	if (pNPWindow->window != NULL && instance->pdata == NULL)
	{
		// only subclass once
		HWND mywindow = (HWND)pNPWindow->window;
		instance->pdata = (WNDPROC)SetWindowLongPtr(mywindow,GWL_WNDPROC,(LONG)PluginWndProc);
		// Store the original window proc in the userdata for the window
		SetWindowLongPtr(mywindow,GWLP_USERDATA,(LONG)instance->pdata);
	}

	return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance,NPMIMEType type,NPStream* stream,NPBool seekable,uint16_t* stype)
{
	return NPERR_NO_ERROR;
}

int32_t NPP_WriteReady(NPP instance,NPStream *stream)
{
	return 4096;
}

int32_t NPP_Write(NPP instance,NPStream *stream,int32_t offset,int32_t len,void *buffer)
{
	// The browser likes to check that we consumed all the data that it fed us so return the length or it will come back asking us to clean our plate.
	return len;
}

NPError NPP_DestroyStream(NPP instance,NPStream *stream,NPError reason)
{
	if (stream->notifyData != &NOTIFY_SKIP_GOBACK &&
		!(IsTokenValueInQueryString(stream->url,"cofc_goback=false")))
	{
		GoBack(instance);
	}

	// If the browser was not able to successfully download the file then I'm not launching it!
	if (reason == NPERR_NO_ERROR)
	{
		LaunchClickOnceApp(stream->url);
	}

	return NPERR_NO_ERROR;
}

void NPP_StreamAsFile(NPP instance,NPStream* stream,const char* fname)
{
}

void NPP_Print(NPP instance,NPPrint* printInfo)
{
	// Wow, either you're really fast or it's time for a new computer.
}

void NPP_URLNotify(NPP instance,const char* url,NPReason reason,void* notifyData)
{
}

NPError NPP_GetValue(NPP instance,NPPVariable variable,void* retValue)
{
	if (variable == NPPVpluginScriptableNPObject)
	{
		NPObject* plugin = NPApplicationLauncher::CreateInstance(instance);
		g_pPluginFuncs->retainobject(plugin);

		*(void**)retValue = plugin;
		return NPERR_NO_ERROR;
	}
	return NPERR_GENERIC_ERROR;
}

//
// Utility functions
//

void GoBack(NPP instance)
{
	NPIdentifier historyID = g_pPluginFuncs->getstringidentifier("history");
	NPIdentifier goFuncID = g_pPluginFuncs->getstringidentifier("go");
	NPObject* window;
	CHECK_NPERR(g_pPluginFuncs->getvalue(instance,NPNVWindowNPObject,&window));
	CHECK_NULL(window);

	CHECK_BOOL(g_pPluginFuncs->hasproperty(instance,window,historyID));
	NPVariant histv;
	CHECK_BOOL(g_pPluginFuncs->getproperty(instance,window,historyID,&histv));
	NPObject* history = NPVARIANT_TO_OBJECT(histv);
	CHECK_NULL(history);

	CHECK_BOOL(g_pPluginFuncs->hasmethod(instance,history,goFuncID))

	NPVariant negOne;
	INT32_TO_NPVARIANT(-1,negOne);

	NPVariant result;
	g_pPluginFuncs->invoke(instance,history,goFuncID,&negOne,1,&result);

Cleanup:
	return;
}

void LaunchClickOnceApp(const char* url)
{
	CHAR szSystem[MAX_PATH] ={0};
	GetSystemDirectoryA(szSystem,ARRAYSIZE(szSystem));

	CHAR szProcess[MAX_PATH] ={0};

	// The Firefox extension checks for the possible future shared location of PresentationHost.exe in
	// \windows\microsoft.net\framework[64]\wpf\presentationhost.exe
	// but lets be honest if it didn't happen for 4.0 it's not likely for 5.0

	GetSystemDirectoryA(szProcess,ARRAYSIZE(szProcess));
	strncat_s(szProcess,"\\PresentationHost.exe",ARRAYSIZE(szProcess));

	CHAR szArgs[2083] ={0}; // todo: allow unlimited url length?
	strncat_s(szArgs,szProcess,ARRAYSIZE(szArgs));
	strncat_s(szArgs," -LaunchApplication ",ARRAYSIZE(szArgs));
	strncat_s(szArgs,url,ARRAYSIZE(szArgs));

	STARTUPINFOA si ={sizeof(si)};
	PROCESS_INFORMATION pi ={0};

	int result = CreateProcessA(NULL,szArgs,NULL,NULL,FALSE,0,NULL,szSystem,&si,&pi);
}

// Use NPN_URLNotify to download the .application before launching it.
// This guaruntees that the browser did in fact have access to the URL
// before it delegated the download to ClickOnce (which will download the
// file again using IE and IE's policies.)
void SafeLaunchClickOnceApp(NPP instance,const NPUTF8* url)
{
	g_pPluginFuncs->geturlnotify(instance,url,NULL,(void*)&NOTIFY_SKIP_GOBACK);
}

LRESULT PluginWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_PAINT:
			// no-oping WM_PAINT eliminates the flicker that we get in Firefox
			// In Chrome there's stll a flicker probably because the HWND is in a separate process
			return 0;
		case WM_ERASEBKGND:
			return 0;
	}

	WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hwnd,GWLP_USERDATA);
	return CallWindowProc(oldWndProc,hwnd,uMsg,wParam,lParam);
}

// Implement plugin function wrappers to call through g_pPluginFuncs
NPObject* NPN_CreateObject(NPP instance,NPClass* npClass)
{
	return g_pPluginFuncs->createobject(instance,npClass);
}

NPIdentifier GetStringIdentifier(NPUTF8* name)
{
	return g_pPluginFuncs->getstringidentifier(name);
}

NPString NPStrDup(const NPUTF8* str,int len)
{
	NPString outStr;
	outStr.UTF8Length = 0;
	int out_len = len + 1; // for trailing null
	char* outBuffer = (char*)g_pPluginFuncs->memalloc(out_len+1);
	if (outBuffer)
	{
		strncpy_s(outBuffer,out_len+1,str,len);
		outStr.UTF8Length = len;
		outStr.UTF8Characters = outBuffer;
	}
	return outStr;
}

void NPFreeString(NPString str)
{
	if (str.UTF8Length > 0)
	{
		g_pPluginFuncs->memfree((void*)str.UTF8Characters);
		str.UTF8Characters = nullptr;
		str.UTF8Length = 0;
	}
}

bool IsTokenValueInQueryString(const char* url,const char* pTokenValue)
{
	bool retVal = false;
	CHECK_NULL(pTokenValue);

	const char* pQueryString = strchr(url,'?');
	CHECK_NULL(pQueryString);
	pQueryString++; // at worst this advances us to the '\0';
	size_t tokenLen = strlen(pTokenValue);

	// dup the string for local modification
	char dupQueryString[2083]; //INTERNET_MAX_URL_LENGTH;
	CHECK_ZERO(strncpy_s(dupQueryString,ARRAYSIZE(dupQueryString),pQueryString,strlen(pQueryString)));

	char* pContext = NULL;
	char* pCurToken=NULL;
	char* pstrtok = dupQueryString;
	while (pCurToken = strtok_s(pstrtok,"&;",&pContext))
	{
		pstrtok=NULL;
		int curTokenLen = strlen(pCurToken);
		if (curTokenLen == tokenLen &&
			_strnicmp(pCurToken,pTokenValue,tokenLen)==0)
		{
			retVal = true;
			break;
		}
	}

Cleanup:
	return retVal;
}
