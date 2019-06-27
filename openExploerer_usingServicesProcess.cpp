#include <windows.h>
#include <iostream>
using namespace std;
int main(void)
{
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };							//Specifies the window station, desktop, standard handles, and appearance of the main window
	SC_HANDLE schSCManager, schService;
	LPCTSTR lpszBinaryPathName = L"E:\mohanraj\ConsoleApplication3\Debug\ConsoleApplication3.exe"; ///exe file
	LPCTSTR lpszDisplayName = L"hello";			 // Service display name...
	LPCTSTR lpszServiceName = L"hello"; 		// Registry Subkey

												// Open a handle to the SC Manager database...
	schSCManager = OpenSCManager(
		NULL,									// local machine || NAME OF THE COMPUTER
		NULL,									// SERVICES_ACTIVE_DATABASE database is opened by default
		SC_MANAGER_ALL_ACCESS);					// full access rights

	if (NULL == schSCManager)
		cout << "ERROR ON SCM ACCESS\t error code: %d.\n" << GetLastError() << endl;
	else
		cout << "SCManager is successfully accessed!\n" << endl;

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
		NULL);									// no password

	if (schService == NULL)
	{
		cout << "ERROR ON CREATING SERVICES \t error code: " << GetLastError() << endl;
		return FALSE;
	}
	else
	{
		cout<<"Successfully created a service  \n"<<lpszServiceName<<endl;
		if (!CreateProcess(L"C:\\Windows\\System32\\explorer.exe", NULL, NULL, NULL, NULL, 0, NULL, NULL, &si, &pi))
			cout << "\nError on opening a file..! " << GetLastError() << endl;
		else
			cout << "Success opened a file\n";
		if (CloseServiceHandle(schService) == 0)
			cout << "Error on closing service handle\t Error code: " << GetLastError() << endl;
		else
			cout << "\nSERVICE HANDLE IS CLOSED!\n" << endl;
		
	}
	return 0;
}


/*
 CreateProcess(
  LPCSTR                lpApplicationName,
  LPSTR                 lpCommandLine,
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL                  bInheritHandles,
  DWORD                 dwCreationFlags,
  LPVOID                lpEnvironment,
  LPCSTR                lpCurrentDirectory,
  LPSTARTUPINFOA        lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation
);

typedef struct _PROCESS_INFORMATION {
  HANDLE hProcess;
  HANDLE hThread;
  DWORD  dwProcessId;
  DWORD  dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;

*/
