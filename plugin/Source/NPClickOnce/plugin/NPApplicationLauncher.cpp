#include "NPApplicationLauncher.h"
#include "NPClickOnce.h"

#include <vector>
#include <string>

NPApplicationLauncher::NPApplicationLauncher(NPP instance)
: NPScriptableObject<NPApplicationLauncher>(instance)
{
	launchIdentifier = GetStringIdentifier("launchClickOnce");
	versionString = GetStringIdentifier("version");
}

NPApplicationLauncher::~NPApplicationLauncher(void)
{
}


// If this logic ever needs to get significantly more complex
// it will probably make sense to create a map<NPIdentifier, method>
bool NPApplicationLauncher::HasMethod(NPIdentifier method)
{
	return (method == launchIdentifier);
}

bool NPApplicationLauncher::Invoke(NPIdentifier method,const NPVariant* args,uint32_t argc,NPVariant* result)
{
	if (method == launchIdentifier &&
		argc == 1 &&
		NPVARIANT_IS_STRING(args[0]))
	{
		NPString url = NPVARIANT_TO_STRING(args[0]);
		// ISSUE 7 - NPString might not be NULL terminated
		// copy the string to ensure null termination.
		url = NPStrDup(url);
		SafeLaunchClickOnceApp(_npp,url.UTF8Characters);
		NPFreeString(url);
		return true;
	}
	return false;
}

bool NPApplicationLauncher::HasProperty(NPIdentifier name)
{
	return name == versionString;
}

extern HMODULE g_dllModule;
namespace
{
	bool GetVersionOfModule(HMODULE ourModule,unsigned int& outMajor,unsigned int& outMinor,unsigned int& outRev,unsigned int& outBld)
	{
		std::wstring fileName;
		{
			if (ourModule == nullptr)
				return false;

			wchar_t fullPath[2048];
			fullPath[0] = 0;

			GetModuleFileName(ourModule,fullPath,sizeof(fullPath)/sizeof(fullPath[0]));
			fileName = fullPath;
		}

		const size_t verSize = GetFileVersionInfoSizeW(fileName.c_str(),nullptr);
		if (verSize < 1)
			return false;

		std::vector<unsigned char> fileVerBuf(verSize);
		if (!GetFileVersionInfoW(fileName.c_str(),0,fileVerBuf.size(),&fileVerBuf[0]))
			return false;

		VS_FIXEDFILEINFO* fileInfo = nullptr;
		size_t infoLen = 0;
		if (!VerQueryValue(&fileVerBuf[0],L"\\",(LPVOID*)&fileInfo,&infoLen))
			return false;
		if (!fileInfo || infoLen < 1)
			return false;

		outMajor	= HIWORD(fileInfo->dwFileVersionMS);
		outMinor	= LOWORD(fileInfo->dwFileVersionMS);
		outRev		= HIWORD(fileInfo->dwFileVersionLS);
		outBld		= LOWORD(fileInfo->dwFileVersionLS);

		return true;
	}
}

bool NPApplicationLauncher::GetProperty(NPIdentifier name,NPVariant* result)
{
	if (name == versionString && result != nullptr)
	{
		char versionBuf[MAX_PATH];

		unsigned int maj,min,rev,bld;
		if (GetVersionOfModule(g_dllModule,maj,min,rev,bld))
			sprintf_s(versionBuf,"%u.%u.%u",maj,min,rev);
		else
			sprintf_s(versionBuf,"unknown");	

		NPString value = NPStrDup(versionBuf,strlen(versionBuf));
		STRINGN_TO_NPVARIANT(value.UTF8Characters,value.UTF8Length,*result);
		return true;
	}
	return false;
}
