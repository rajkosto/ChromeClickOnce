#include "NPApplicationLauncher.h"
#include "NPClickOnce.h"

#include <vector>
#include <string>

NPApplicationLauncher::NPApplicationLauncher(NPP instance)
: NPScriptableObject<NPApplicationLauncher>(instance)
{
	launchIdentifier = GetStringIdentifier("launchClickOnce");
	versionString = GetStringIdentifier("version");
	clrVersionString = GetStringIdentifier("clrVersion");
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
	if (method == launchIdentifier && argc == 1 && NPVARIANT_IS_STRING(args[0]))
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
	if (name == versionString)
		return true;
	else if (name == clrVersionString)
		return true;
	else
		return false;
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
	else if (name == clrVersionString && result != nullptr)
	{
		HKEY openedKey = nullptr;
		const auto ok = RegOpenKeyExW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\5.0\\User Agent\\Post Platform",
						0,KEY_READ,&openedKey);

		if (ok != ERROR_SUCCESS)
		{ 
			//return null if failed to open key
			NULL_TO_NPVARIANT(*result);
			return true;
		}

		std::string outputVersions;
		{
			std::wstring compareWith = L".NET";
			std::vector<std::wstring> dotNetVersions;
			std::vector<wchar_t> subKeyName(32767); //max length of a key name		
			for (DWORD i=0;; i++)
			{
				DWORD nameLength = static_cast<DWORD>(subKeyName.size());
				DWORD valueType = REG_NONE;
				const auto succ = RegEnumValueW(openedKey,i,&subKeyName[0],&nameLength,nullptr,&valueType,nullptr,nullptr);
				if (succ == ERROR_NO_MORE_ITEMS)
					break;
				else if (succ != ERROR_SUCCESS)
				{
					//report err with null return val
					RegCloseKey(openedKey); openedKey = nullptr;
					NULL_TO_NPVARIANT(*result);
					return true;
				}

				if (valueType != REG_SZ) //skip non strings
					continue;

				auto keyName = std::wstring(&subKeyName[0],nameLength);
				if (keyName.length() < compareWith.length())
					continue;
				if (keyName.substr(0,compareWith.length()) != compareWith)
					continue;

				dotNetVersions.push_back(std::move(keyName));
			}

			RegCloseKey(openedKey); openedKey = nullptr;

			outputVersions = "(";
			for (size_t i=0; i<dotNetVersions.size(); i++)
			{
				const auto& currNet = dotNetVersions[i];

				const size_t utf8Len = WideCharToMultiByte(CP_UTF8,0,currNet.c_str(),currNet.length(),
					(char*)&subKeyName[0],subKeyName.size()*2,nullptr,nullptr);

				outputVersions += std::string((const char*)&subKeyName[0],utf8Len);
				if (i != dotNetVersions.size()-1)
					outputVersions += "; ";
				else
					outputVersions += ")";
			}
		}		

		NPString value = NPStrDup(outputVersions.c_str(),outputVersions.length());
		STRINGN_TO_NPVARIANT(value.UTF8Characters,value.UTF8Length,*result);
		return true;
	}
	return false;
}
