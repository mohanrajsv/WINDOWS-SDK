#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
using namespace std;
#include<iostream>
#include <strsafe.h>
void RefreshDirectory(LPTSTR);
void RefreshTree(LPTSTR);
void WatchDirectory(LPTSTR);

void create_folder()
{
	CreateDirectory(
		L"C:\\mohanraj_BACKUP",
		NULL
	);
}

/*
void delete_folder(LPCTSTR dir) // Fully qualified name of the directory being deleted, without trailing backslash
{
	SHFILEOPSTRUCT file_op = {
		NULL,
		FO_DELETE,
		dir,
		L"",
		FOF_NOCONFIRMATION |
		FOF_NOERRORUI |
		FOF_SILENT,
		false,
		0,
		L"" };
	SHFileOperation(&file_op);
}*/

void CopyDirTo(const wstring& source_folder, const wstring& target_folder)
{
	wstring new_sf = source_folder + L"\\*";
	WCHAR sf[MAX_PATH + 1];
	WCHAR tf[MAX_PATH + 1];
	wcscpy_s(sf, MAX_PATH, new_sf.c_str());
	wcscpy_s(tf, MAX_PATH, target_folder.c_str());
	sf[lstrlenW(sf) + 1] = 0;
	tf[lstrlenW(tf) + 1] = 0;
	SHFILEOPSTRUCTW s = { 0 };
	s.wFunc = FO_COPY;
	s.pTo = tf;
	s.pFrom = sf;
	s.fFlags = FOF_SILENT | FOF_NOCONFIRMMKDIR | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NO_UI;
	int res = SHFileOperationW(&s);
}

void _tmain(int argc, TCHAR* argv[])
{
	SC_HANDLE schSCManager, schService;
	LPCTSTR lpszBinaryPathName = L"E:\\mohanraj\\monitor\\Debug\\monitor.exe"; ///exe file
	LPCTSTR lpszDisplayName = L"newzohoV";			 // Service display name...
	LPCTSTR lpszServiceName = L"newzohoV"; 		// Registry Subkey
							
	schSCManager = OpenSCManager(
		NULL,									// local machine || NAME OF THE COMPUTER
		NULL,									// SERVICES_ACTIVE_DATABASE database is opened by default
		SC_MANAGER_ALL_ACCESS);					// full access rights

	schService = CreateServiceW(
		schSCManager,							// SCManager database
		lpszServiceName,						// name of service
		lpszDisplayName,						// service name to display
		SERVICE_ALL_ACCESS,						// desired access
		SERVICE_WIN32_OWN_PROCESS,				 // service type
		SERVICE_AUTO_START,						// start type
		SERVICE_ERROR_NORMAL,					// error control type
		lpszBinaryPathName,						// service's binary
		NULL,									// no load ordering group
		NULL,									// no tag identifier
		NULL,									// no dependencies,
		NULL,									// LocalSystem account
		NULL);
	if (schService == NULL)
		cout << "ERROR ON CREATING SERVICES \t error code: " << GetLastError() << endl;

	if (argc != 2)
	{
		_tprintf(TEXT("Usage: %s <dir>\n"), argv[0]);
		return;
	}

	WatchDirectory(argv[1]);

	////deleting the service
}

void WatchDirectory(LPTSTR lpDir)
{
	DWORD dwWaitStatus;
	HANDLE dwChangeHandles[2];
	TCHAR lpDrive[4];
	TCHAR lpFile[_MAX_FNAME];
	TCHAR lpExt[_MAX_EXT];
	_tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
	lpDrive[2] = (TCHAR)'\\';
	lpDrive[3] = (TCHAR)'\0';

	// Watch the directory for file creation and deletion. 
	dwChangeHandles[0] = FindFirstChangeNotification(
		lpDir,                         // directory to watch 
		FALSE,                         // do not watch subtree 
		FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 
	if (dwChangeHandles[0] == INVALID_HANDLE_VALUE)
	{
		printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError());
	}

	// Watch the subtree for directory creation and deletion. 
	dwChangeHandles[1] = FindFirstChangeNotification(
		lpDrive,                       // directory to watch 
		TRUE,                          // watch the subtree 
		FILE_NOTIFY_CHANGE_DIR_NAME);  // watch dir name changes 

	if (dwChangeHandles[1] == INVALID_HANDLE_VALUE)
	{
		printf("\n ERROR: FindFirstChangeNotification function failed.\n");
		ExitProcess(GetLastError());
	}

	CreateDirectory(
		L"C:\\temp",
		NULL
	);
	/////////////CREATING DIRECTORY
	create_folder();
	/////////////COPY
	CopyDirTo(lpDir, L"C:\\mohanraj_BACKUP");

	while (TRUE)
	{
		CopyDirTo(lpDir, L"C:\\mohanraj_BACKUP");
		
		printf("\nRUNNING...\n");
		dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles,
			FALSE, INFINITE);
		switch (dwWaitStatus)
		{
		case WAIT_OBJECT_0:
			
			//delete_folder(L"C:\\mohanraj_BACKUP");

			 MoveFileEx(L"C:\\mohanraj_BACKUP", L"C:\\temp", MOVEFILE_WRITE_THROUGH);
			create_folder();
			
			if (FindNextChangeNotification(dwChangeHandles[0]) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}
			break;

		case WAIT_OBJECT_0 + 1:
			MoveFileEx(L"C:\\mohanraj_BACKUP", L"C:\\temp", MOVEFILE_WRITE_THROUGH);
			create_folder();
			

			if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE)
			{
				printf("\n ERROR: FindNextChangeNotification function failed.\n");
				ExitProcess(GetLastError());
			}
			break;

		case WAIT_TIMEOUT:
			printf("\nNo changes in the timeout period.\n");
			break;

		default:
			printf("\n ERROR: Unhandled dwWaitStatus.\n");
			ExitProcess(GetLastError());
			break;
		}
	}
}
