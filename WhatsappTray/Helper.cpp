/*
*
* WhatsappTray
* Copyright (C) 1998-2018 Sebastian Amann
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#include "stdafx.h"
#include "Helper.h"

#include "Logger.h"

#include <vector>
#include <sstream>
#include <Shlobj.h>
#include <psapi.h>

/* For .lnk resolver */
#include "shobjidl.h"
#include "shlguid.h"
#include "strsafe.h"

#undef MODULE_NAME
#define MODULE_NAME "Helper::"

/*
* @brief Get the Path to the exe-file of the application.
*/
std::string Helper::GetApplicationFilePath()
{
	char szPathToExe[MAX_PATH];
	GetModuleFileNameA(NULL, szPathToExe, MAX_PATH);

	return szPathToExe;
}

/*
* @brief Get the Path to the directory where the exe-file of the application is.
*/
std::string Helper::GetApplicationDirectory()
{
	char szPathToExe[MAX_PATH];
	GetModuleFileNameA(NULL, szPathToExe, MAX_PATH);

	std::string applicationFilePath = GetApplicationFilePath();
	std::size_t found = applicationFilePath.find_last_of('\\');
	if (found == std::string::npos) {
		OutputDebugString(TEXT("ERROR: GetApplicationDirectory() - Could not find \\ in the application-path.\n"));
		return "";
	}

	return applicationFilePath.substr(0, found) + "\\";
}

/**
* Untested!
*/
std::wstring Helper::ToWString(const std::string& inputString)
{
	const char* inputC_String = inputString.c_str();
	size_t size = inputString.length() + 1;
	wchar_t* outputStringBuffer = new wchar_t[size];

	size_t outSize;
	errno_t convertResult = mbstowcs_s(&outSize, outputStringBuffer, size, inputC_String, size - 1);

	if (convertResult != NULL) {
		Logger::Error("Error in mbsrtowcs(): %d", errno);
		return L"";
	}

	std::wstring outputString = outputStringBuffer;
	delete[] outputStringBuffer;

	return outputString;
}

/**
* Convert a std::wstring to a std::string.
*/
std::string Helper::ToString(const std::wstring& inputString)
{
	const wchar_t* inputC_String = inputString.c_str();
	size_t size = inputString.length() + 1;
	char* outputStringBuffer = new char[size];

	size_t outSize;
	errno_t convertResult = wcstombs_s(&outSize, outputStringBuffer, size, inputC_String, size - 1);

	if (convertResult != NULL) {
		Logger::Error("Error in mbsrtowcs(): %d", errno);
		return "";
	}

	std::string outputString = outputStringBuffer;
	delete[] outputStringBuffer;

	return outputString;
}

std::wstring Helper::Utf8ToWide(const std::string& inputString)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, inputString.c_str(), (int)inputString.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, inputString.c_str(), (int)inputString.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

std::string Helper::WideToUtf8(const std::wstring& inputString)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, inputString.c_str(), (int)inputString.size(), NULL, 0, NULL, NULL);
	std::string strTo(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, inputString.c_str(), (int)inputString.size(), &strTo[0], size_needed, NULL, NULL);
	return strTo;
}

bool Helper::Replace(std::string& str, const std::string& oldValue, const std::string& newValue) {
	size_t start_pos = str.find(oldValue);
	if (start_pos == std::string::npos) {
		return false;
	}

	str.replace(start_pos, oldValue.length(), newValue);

	return true;
}

HICON Helper::GetWindowIcon(HWND hwnd)
{
	HICON icon;
	if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_SMALL, 0)) return icon;
	if (icon = (HICON)SendMessage(hwnd, WM_GETICON, ICON_BIG, 0)) return icon;
	if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM)) return icon;
	if (icon = (HICON)GetClassLongPtr(hwnd, GCLP_HICON)) return icon;
	return LoadIcon(NULL, IDI_WINLOGO);
}

// From: https://stackoverflow.com/questions/7028304/error-lnk2019-when-using-getfileversioninfosize
std::string Helper::GetProductAndVersion()
{
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0) {
		Logger::Error("GetModuleFileName failed with error %d", GetLastError());
		return "";
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0) {
		Logger::Error("GetFileVersionInfoSize failed with error %d", GetLastError());
		return "";
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0])) {
		Logger::Error("GetFileVersionInfo failed with error %d", GetLastError());
		return "";
	}

	// get the version strings
	LPVOID pvProductVersion = NULL;
	unsigned int iProductVersionLen = 0;

	UINT uiVerLen = 0;
	VS_FIXEDFILEINFO*   pFixedInfo = 0;     // pointer to fixed file info structure
	// get the fixed file info (language-independent) 
	if (VerQueryValue(&data[0], TEXT("\\"), (void**)&pFixedInfo, (UINT *)&uiVerLen) == 0) {
		Logger::Error("Can't obtain ProductVersion from resources");
		return "";
	}

	std::stringstream stringStream;
	stringStream << HIWORD(pFixedInfo->dwProductVersionMS) << "." << LOWORD(pFixedInfo->dwProductVersionMS) << "." << HIWORD(pFixedInfo->dwProductVersionLS) << "." << LOWORD(pFixedInfo->dwProductVersionLS);

	return stringStream.str();
}

std::string Helper::GetStartMenuProgramsDirectory()
{
	wchar_t* startMenuProgramsBuffer;
	if (SHGetKnownFolderPath(FOLDERID_Programs, 0, NULL, &startMenuProgramsBuffer) != S_OK) {
		MessageBoxA(NULL, "'Start Menu\\Programs' folder not found", "WhatsappTray", MB_OK);
		return NULL;
	}
	std::string startMenuPrograms = Helper::ToString(startMenuProgramsBuffer);
	CoTaskMemFree(startMenuProgramsBuffer);
	return startMenuPrograms;
}

std::string Helper::GetWindowsAppDataDirectory()
{
	char appDataDirectory[MAX_PATH] = { 0 };
	if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appDataDirectory) != S_OK) {
		Logger::Fatal(MODULE_NAME "Init() - Could not get the AppData-directory!");
		MessageBoxA(NULL, MODULE_NAME "Init() - Fatal: Could not get the AppData-directory!", "WhatsappTray", MB_OK | MB_ICONINFORMATION);
		return "";
	}
	return appDataDirectory;
}

std::string Helper::GetCurrentUserDirectory()
{
	char currentUserDirectory[MAX_PATH] = { 0 };
	if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, currentUserDirectory) != S_OK) {
		Logger::Fatal(MODULE_NAME "Init() - Could not get the CurrentUser-directory!");
		MessageBoxA(NULL, MODULE_NAME "Init() - Fatal: Could not get the CurrentUser-directory!", "WhatsappTray", MB_OK | MB_ICONINFORMATION);
		return "";
	}
	return currentUserDirectory;
}

/**
 * @brief Gets the path to C:\Users\<JohnDoe>\AppData\Roaming
*/
std::string Helper::GetCurrentUserAppData()
{
	char currentUserDirectory[MAX_PATH] = { 0 };
	if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, currentUserDirectory) != S_OK) {
		Logger::Fatal(MODULE_NAME "Init() - Could not get the CurrentUser-directory!");
		MessageBoxA(NULL, MODULE_NAME "Init() - Fatal: Could not get the CurrentUser-directory!", "WhatsappTray", MB_OK | MB_ICONINFORMATION);
		return "";
	}
	return currentUserDirectory;
}

std::string Helper::GetFilenameFromPath(std::string path)
{
	std::string filename(MAX_PATH, 0);
	std::string extension(MAX_PATH, 0);
	_splitpath_s(path.c_str(), NULL, NULL, NULL, NULL, &filename[0], filename.length(), &extension[0], extension.length());

	return filename;
}

std::wstring Helper::GetFilenameFromPath(std::wstring path)
{
	std::wstring filename(MAX_PATH, 0);
	std::wstring extension(MAX_PATH, 0);
	_wsplitpath_s(path.c_str(), NULL, NULL, NULL, NULL, &filename[0], filename.length(), &extension[0], extension.length());

	return filename;
}

/**
 * @brief Uses the Shell's IShellLink and IPersistFile interfaces to retrieve the path and description from an existing shortcut.
 * 
 * Returns the result of calling the member functions of the interfaces. 
 * WARNING: It is assumed that CoInitialize()
 *
 * @param  hwnd A handle to the parent window. The Shell uses this window to 
 *                display a dialog box if it needs to prompt the user for more 
 *                information while resolving the link.
 * @param  lpszLinkFile Address of a buffer that contains the path of the link,
 *                including the file name.
 * 
 * @note From: https://docs.microsoft.com/en-ca/windows/win32/shell/links?redirectedfrom=MSDN (ResolveIt)
 */
std::string Helper::ResolveLnk(HWND hwnd, LPCSTR lpszLinkFile)
{
	HRESULT hres;
	IShellLink* psl;
	TCHAR szGotPath[MAX_PATH];
	//TCHAR szDescription[MAX_PATH];
	WIN32_FIND_DATA wfd;
	std::string lnkPath = "";

	//*lpszPath = 0; // Assume failure 

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize has already been called. 
	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf;

		// Get a pointer to the IPersistFile interface. 
		hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);

		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode. 
			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH);

			// Add code here to check return value from MultiByteWideChar 
			// for success.

			// Load the shortcut. 
			hres = ppf->Load(wsz, STGM_READ);

			if (SUCCEEDED(hres))
			{
				// Resolve the link. 
				hres = psl->Resolve(hwnd, 0);

				if (SUCCEEDED(hres))
				{
					// Get the path to the link target. 
					hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);

					if (SUCCEEDED(hres))
					{
						//hres = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
						lnkPath = std::string(szGotPath);

						// Get the description of the target. 
						//hres = psl->GetDescription(szDescription, MAX_PATH);

						//if (SUCCEEDED(hres))
						//{
						//	
						//}
					}
				}
			}

			// Release the pointer to the IPersistFile interface. 
			ppf->Release();
		}

		// Release the pointer to the IShellLink interface. 
		psl->Release();
	}

	return lnkPath;
}

/**
 * @brief Get the path to the executable for the ProcessID
 *
 * @param processId The ProcessID from which the path to the executable should be fetched
 * @return The path to the executable from the ProcessID
*/
std::string Helper::GetFilepathFromProcessID(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
	if (processHandle == NULL) {
		Logger::Error(MODULE_NAME "::GetFilepathFromProcessID() - Failed to open process.");
		return "";
	}

	wchar_t filepath[MAX_PATH];
	if (GetModuleFileNameExW(processHandle, NULL, filepath, MAX_PATH) == 0) {
		CloseHandle(processHandle);
		Logger::Error(MODULE_NAME "::GetFilepathFromProcessID() - Failed to get module filepath.");
		return "";
	}
	CloseHandle(processHandle);

	return Helper::WideToUtf8(filepath);
}