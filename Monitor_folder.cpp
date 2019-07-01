#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
using namespace std;
#include<iostream>
#include <strsafe.h>

void backup_folder()
{
	CreateDirectory(
		L"C:\\mohanraj_BACKUP",
		NULL
	);
}


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
	int res = SHFileOperation(&file_op);
	if (res == 0)
		cout << "deleted done";
	else {
		cout << "cant deleted " << GetLastError() << endl;
		wcout << "dir : " << dir << endl;
	}
}
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

void backup(LPCWSTR path)
{
	/*
	FileName member of FILE_NOTIFY_INFORMATION has only one WCHAR according to definition. Most likely, this field will have more characters.
	So the expected size of one item is (sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(WCHAR)).
	Prepare buffer for 256 items.
	*/
	backup_folder();
	CopyDirTo(L"C:\\mohanrajnew", L"C:\\mohanraj_BACKUP");

	TCHAR buf[256 * (sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH * sizeof(WCHAR))] = { 0 };

	DWORD bytesReturned = 0;
	BOOL result = FALSE;
	FILE_NOTIFY_INFORMATION* fni = NULL;

	HANDLE hDir = CreateFile(path,
		FILE_LIST_DIRECTORY | STANDARD_RIGHTS_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL);

	if (!hDir || hDir == INVALID_HANDLE_VALUE)
	{
		wprintf(L"CreateFile failed\n");
		return;
	}

	while (1)
	{
		result = ReadDirectoryChangesW(hDir,
			buf,
			sizeof(buf) / sizeof(*buf),
			TRUE, /* monitor the entire subtree */
			FILE_NOTIFY_CHANGE_FILE_NAME |
			FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_SIZE |
			FILE_NOTIFY_CHANGE_LAST_WRITE |
			FILE_NOTIFY_CHANGE_LAST_ACCESS |
			FILE_NOTIFY_CHANGE_CREATION |
			FILE_NOTIFY_CHANGE_SECURITY,
			&bytesReturned,
			NULL,
			NULL);

		if (result && bytesReturned)
		{
			wchar_t filename[MAX_PATH];
			wchar_t destname[MAX_PATH];
			wchar_t destname2[MAX_PATH];
			wchar_t srcname[MAX_PATH];
			wchar_t tmpname[MAX_PATH];
			wchar_t action[256];
			TCHAR lpDrive[4];
			TCHAR lpFile[_MAX_FNAME];
			TCHAR lpDir[_MAX_DIR];
			DWORD res;


			for (fni = (FILE_NOTIFY_INFORMATION*)buf; fni; )
			{
				TCHAR* lpExt;
				lpExt = new TCHAR[_MAX_EXT];
				switch (fni->Action)
				{
				case FILE_ACTION_ADDED:
					wcscpy_s(action, sizeof(action) / sizeof(*action), L"File added:");
					wcsncpy_s(destname, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					destname[fni->FileNameLength / 2] = 0;

					wcscpy_s(destname2, MAX_PATH, L"C:\\mohanraj_BACKUP\\");
					wcscpy_s(srcname, MAX_PATH, L"C:\\mohanrajnew\\");

					wcscat_s(srcname, fni->FileName);
					wcscat_s(destname2, destname);

					_tsplitpath_s(srcname, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
					wprintf(L"\n	EXTENSTION %s\n", lpExt);

					if (wcslen(lpExt) > 0)
						printf("\n");
					else
					{

						if (wcslen(lpExt) == 0)
						{
							CreateDirectory(
								destname2,
								NULL
							);
						}
						else
							cout << "\nERROR WHILE adding file\n";
					}
					break;

				case FILE_ACTION_REMOVED:
					wcscpy_s(action, sizeof(action) / sizeof(*action), L"File removed:");
					wcsncpy_s(destname, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					destname[fni->FileNameLength / 2] = 0;

					wcscpy_s(destname2, MAX_PATH, L"C:\\mohanraj_BACKUP\\");
					wcscpy_s(tmpname, MAX_PATH, L"C:\\");
					wcscpy_s(srcname, MAX_PATH, L"C:\\mohanrajnew\\");

					wcscat_s(srcname, fni->FileName);
					wcscat_s(destname2, fni->FileName);
					free(lpExt);
					lpExt = new TCHAR[_MAX_EXT];
					_tsplitpath_s(destname2, lpDrive, 4, lpDir, _MAX_DIR, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
					wcscat_s(tmpname, lpDir);
					wprintf(L"\n	EXTENSTION %s\n", lpExt);

					if (*lpExt == L'.txt' || *lpExt == L'.rtf' || *lpExt == L'.zip' || *lpExt == L'.jpeg' || *lpExt == L'.ini')
					{
						res = DeleteFileW(destname2);
						if (res)
							cout << "deleted";
						else
							cout << "error";
					}
					else
					{
						cout << "elsepart";
						delete_folder((LPCTSTR)tmpname);
					}
					break;

				case FILE_ACTION_MODIFIED:
					wcscpy_s(action, sizeof(action) / sizeof(*action), L"File modified:");
					wcsncpy_s(destname, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					destname[fni->FileNameLength / 2] = 0;

					wcscpy_s(destname2, MAX_PATH, L"C:\\mohanraj_BACKUP\\");
					wcscpy_s(srcname, MAX_PATH, L"C:\\mohanrajnew\\");

					wcscat_s(srcname, fni->FileName);
					wcscat_s(destname2, destname);

					_tsplitpath_s(srcname, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
					wprintf(L"\n	EXTENSTION %s\n", lpExt);

					if (wcslen(lpExt) > 0 && CopyFile(srcname, destname2, FALSE))
						printf("File copied.\n");
					else
					{
						if (wcslen(lpExt) == 0)
						{
							CreateDirectory(
								destname2,
								NULL
							);
						}
					}
					break;

				case FILE_ACTION_RENAMED_OLD_NAME:
					wcscpy_s(action, sizeof(action) / sizeof(*action), L"File renamed, was:");
					wcsncpy_s(destname, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					destname[fni->FileNameLength / 2] = 0;
					wcscpy_s(destname2, MAX_PATH, L"C:\\mohanraj_BACKUP\\");
					wcscpy_s(srcname, MAX_PATH, L"C:\\mohanrajnew\\");
					wcscat_s(srcname, fni->FileName);
					wcscat_s(destname2, destname);
					_tsplitpath_s(srcname, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
					wprintf(L"\n	EXTENSTION %s\n", lpExt);
					if (wcslen(lpExt) > 0)
					{
						if (DeleteFileW(destname2))
							printf("");
					}
					else if (wcslen(lpExt) == 0)
						RemoveDirectory(destname2);

					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					wcscpy_s(action, sizeof(action) / sizeof(*action), L"File renamed, now is:");
					wcsncpy_s(destname, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					destname[fni->FileNameLength / 2] = 0;
					wcscpy_s(destname2, MAX_PATH, L"C:\\mohanraj_BACKUP\\");
					wcscpy_s(srcname, MAX_PATH, L"C:\\mohanrajnew\\");
					wcscat_s(srcname, fni->FileName);
					wcscat_s(destname2, destname);
					_tsplitpath_s(srcname, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

					if (wcslen(lpExt) > 0 && CopyFile(srcname, destname2, FALSE))
					{
						printf("File copied.\n");
					}
					else
					{
						if (wcslen(lpExt) == 0)
							CreateDirectory(
								destname2,
								NULL
							);
						else
							cout << "\nERROR WHILE RENAMING\n";
					}
					break;

				default:
					swprintf_s(action, sizeof(action) / sizeof(*action), L"Unkonwn action: %ld. File name is:", fni->Action);
				}
				if (fni->FileNameLength)
				{
					wcsncpy_s(filename, MAX_PATH, fni->FileName, fni->FileNameLength / 2);
					filename[fni->FileNameLength / 2] = 0;
					wprintf(L"%s '%s'\n", action, filename);
				}
				else
					wprintf(L"%s <EMPTY>\n", action);

				if (fni->NextEntryOffset)
				{
					char* p = (char*)fni;
					fni = (FILE_NOTIFY_INFORMATION*)(p + fni->NextEntryOffset);
				}
				else
					fni = NULL;
				free(lpExt);
			}
		}
		else
			wprintf(L"ReadDirectoryChangesW failed\n");
	}
	CloseHandle(hDir);
}

int main()
{
	backup(L"C:\\mohanrajnew");
	return 0;
}
