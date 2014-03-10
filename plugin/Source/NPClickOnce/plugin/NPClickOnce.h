#pragma once

#include "WinApi.h"
#include <npapi/npapi.h>
#include <npapi/npfunctions.h>

// Exported functions called by the browser to initialize the plugin
extern "C"
{
	NPError OSCALL NP_GetEntryPoints(NPPluginFuncs* pFuncs);
	NPError OSCALL NP_Initialize(NPNetscapeFuncs* bFuncs);
	NPError OSCALL NP_Shutdown();
}

static NPNetscapeFuncs* g_pPluginFuncs;

// Functions that the plugin must implement and return via NP_GetEntryPoints() to the browser

NPError NPP_New(NPMIMEType pluginType,NPP instance,uint16_t mode,int16_t argc,char* argn[],char* argv[],NPSavedData* saved);
NPError NPP_Destroy(NPP instance,NPSavedData** save);
NPError NPP_SetWindow(NPP instance,NPWindow* pNPWindow);
NPError NPP_NewStream(NPP instance,NPMIMEType type,NPStream* stream,NPBool seekable,uint16_t* stype);
NPError NPP_GetValue(NPP instance,NPPVariable variable,void* retValue);
int32_t NPP_WriteReady(NPP instance,NPStream *stream);
int32_t NPP_Write(NPP instance,NPStream *stream,int32_t offset,int32_t len,void *buffer);
NPError NPP_DestroyStream(NPP instance,NPStream *stream,NPError reason);
void NPP_StreamAsFile(NPP instance,NPStream* stream,const char* fname);
void NPP_Print(NPP instance,NPPrint* printInfo);
void NPP_URLNotify(NPP instance,const char* url,NPReason reason,void* notifyData);

// Utility functions

void GoBack(NPP);
void LaunchClickOnceApp(const char* url);
LRESULT PluginWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

NPIdentifier GetStringIdentifier(NPUTF8* name);
NPString NPStrDup(const NPUTF8* str,int len);
inline NPString NPStrDup(NPString str) { return NPStrDup(str.UTF8Characters,str.UTF8Length); }
void NPFreeString(NPString str);
const int NOTIFY_SKIP_GOBACK = 42;
void SafeLaunchClickOnceApp(NPP instance,const NPUTF8* url);

bool IsTokenValueInQueryString(const char* url,const char* pTokenValue);

#define CHECK_NULL(x) do { if ((x) == NULL) goto Cleanup; } while(0);
#define CHECK_BOOL(x) do { if ((x) == false) goto Cleanup; } while(0);
#define CHECK_NPERR(x) do { if ((x) != NPERR_NO_ERROR) goto Cleanup; } while(0);
#define CHECK_ZERO(x) do { if ((x) != 0) goto Cleanup; } while(0);
