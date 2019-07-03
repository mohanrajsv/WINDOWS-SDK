#include <windows.h>
#include <iostream>
#define MAX_NAME 512
using namespace std;

SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
HANDLE hServiceEvent = NULL;

LPCTSTR lpszDisplayName = L"aftrnoon";			 // Service display name...
LPCTSTR szSvcName = L"aftrnoon"; 		// Registry Subkey
LPCTSTR lpszBinaryPathName = L"C:\\Users\\Administrator\\source\\repos\\serviceADemo\\Debug\\serviceADemo.exe"; ///exe file


//////////////////token function

BOOL SearchTokenGroupsForSID(VOID)
{
	DWORD i, dwSize = 0, dwResult = 0;
	HANDLE hToken;
	PTOKEN_GROUPS pGroupInfo;
	SID_NAME_USE SidType;
	char lpName[MAX_NAME];
	char lpDomain[MAX_NAME];
	PSID pSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		printf("OpenProcessToken Error %u\n", GetLastError());
		return FALSE;
	}

	ImpersonateLoggedOnUser(hToken);   ////////////impersonateUser

	if (!GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))
	{
		dwResult = GetLastError();
		if (dwResult != ERROR_INSUFFICIENT_BUFFER) {
			printf("Error on getting token information");
			return FALSE;
		}
	}
	// Allocate the buffer.
	pGroupInfo = (PTOKEN_GROUPS)GlobalAlloc(GPTR, dwSize);

	// Call GetTokenInformation again to get the group information.

	if (!GetTokenInformation(hToken, TokenGroups, pGroupInfo,
		dwSize, &dwSize))
	{
		printf("GetTokenInformation Error %u\n", GetLastError());
		return FALSE;
	}

	// Create a SID for the BUILTIN\Administrators group.
	if (!AllocateAndInitializeSid(&SIDAuth, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&pSID))
	{
		printf("AllocateAndInitializeSid Error %u\n", GetLastError());
		return FALSE;
	}
	// Loop through the group SIDs looking for the administrator SID.

	for (i = 0; i < pGroupInfo->GroupCount; i++)
	{
		if (EqualSid(pSID, pGroupInfo->Groups[i].Sid))
		{
			// Lookup the account name and print it.
			dwSize = MAX_NAME;
			if (!LookupAccountSidW(NULL, pGroupInfo->Groups[i].Sid,
				(LPWSTR)lpName, &dwSize, (LPWSTR)lpDomain,
				&dwSize, &SidType))
			{
				dwResult = GetLastError();
				if (dwResult == ERROR_NONE_MAPPED)
					strcpy_s(lpName, dwSize, "NONE_MAPPED");
				else
				{
					printf("LookupAccountSid Error %u\n", GetLastError());
					return FALSE;
				}
			}
			printf("\n\nToken INFORMATION :\nCurrent user domain : %s\n Group :%s\n\n",
				lpDomain, lpName);
		}
	}
	if (pSID)
		FreeSid(pSID);

	if (pGroupInfo)
		GlobalFree(pGroupInfo);
	return TRUE;
}

///report status t scm
void ServiceReportStatus(DWORD dwCurrentState,DWORD dwWin32ExitCode,DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		ServiceStatus.dwControlsAccepted = 0;
	else
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if (dwCurrentState == SERVICE_RUNNING || dwCurrentState == SERVICE_STOPPED)
		ServiceStatus.dwCheckPoint = 0;
	else
		ServiceStatus.dwCheckPoint = dwCheckPoint++;

	bSetServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);

	if (FALSE == bSetServiceStatus)
		cout << "Service status failed\n";
}

void  ServiceControlHandler(DWORD dwControl)
{
	switch (dwControl)
	{
	case SERVICE_CONTROL_STOP:
		cout << "SERVICE STOPED\n";
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_PAUSE:
		break;
	case SERVICE_CONTROL_CONTINUE:
		break;

	default:
		break;
	}
}

void ServiceInit(DWORD dwArgc, LPSTR* lpArgv)
{
	hServiceEvent = CreateEvent(
		NULL,
		TRUE,
		FALSE,
		NULL
	);
	if (NULL == hServiceEvent)
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	else
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	while (1) {
		WaitForSingleObject(hServiceEvent, INFINITE);
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
}

void  ServiceMain(DWORD dwArgc, LPSTR* lpArgv)
{
	BOOL bServiceStatus = FALSE;
	hServiceStatusHandle = RegisterServiceCtrlHandler(szSvcName,(LPHANDLER_FUNCTION)ServiceControlHandler);
	if (NULL != hServiceStatusHandle)
		cout << "Regsister servicectrHandler failed";
	else
		cout << "RegisterCtrlServiceDispatcher Succeess!";

	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (FALSE == bServiceStatus)
		cout << "SERVICE STATUS INITIAL SETUP ERROR\n";
	ServiceInit(dwArgc, lpArgv);
}


void deleteSvc()
{
	SC_HANDLE schSCManager, tmpHandle;
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	tmpHandle = OpenServiceW(
		schSCManager,         // SCM database 
		szSvcName,            // name of service 
		DELETE);  

	if (tmpHandle == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	if (!DeleteService(tmpHandle))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
	}
	else printf("Service deleted successfully\n");

	
	CloseServiceHandle(schSCManager);
	CloseServiceHandle(tmpHandle);
}

void start()
{
	SC_HANDLE schSCManager, schService, tmpHandle;

	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}
	tmpHandle = OpenServiceW(
		schSCManager,         // SCM database 
		szSvcName,            // name of service 
		SERVICE_ALL_ACCESS | SERVICE_START);  // full access 
	if (tmpHandle == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	QueryServiceStatusEx(
		tmpHandle,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)& ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded);

	if (!StartService(
		tmpHandle,  // handle to service 
		0,           // number of arguments 
		NULL))      // no arguments 
	{
		DWORD ERR = GetLastError();
		if (ERR == 1056)
			cout << "\nthe service is already running..\n";
		else if (ERR == 1058)
			cout << "\nThe service cannot be started, either because it is disabled or because it has no enabled devices associated with it.\n";
		else
		printf("StartService failed (%d)\n", GetLastError());
		CloseServiceHandle(tmpHandle);
		CloseServiceHandle(schSCManager);
		return;
	}
	else
		cout << "\nSTATRING.....!\n";
	if (!QueryServiceStatusEx(
		tmpHandle,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)& ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		CloseServiceHandle(tmpHandle);
		CloseServiceHandle(schSCManager);
		return;
	}
	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
	
		if (!QueryServiceStatusEx(tmpHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)& ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			cout << "QueryServiceStatusEx Failed " << GetLastError() << endl;
			break;
		}

	}
	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
		printf("Service started successfully.\n");
	else
		printf("Service not started. \n");
	CloseServiceHandle(tmpHandle);
	CloseServiceHandle(schSCManager);
}



void install()
{
	SC_HANDLE schSCManager, schService, tmpHandle;
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_CONNECT |
		SC_MANAGER_CREATE_SERVICE);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}
	schService = CreateService(
		schSCManager,              // SCM database 
		szSvcName,                   // name of service 
		lpszDisplayName,                   // service name to display 
		SERVICE_QUERY_STATUS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		lpszBinaryPathName,                    // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,
		NULL);                     // no password 

	if (schService == NULL)
	{
		DWORD ERR = GetLastError();
		if (ERR == 1073)
			cout << "\nThe specified service already exists.\n";
		else
		printf("specified Service already exist or CreateService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}
	else 
		printf("\nService installed successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

void stop()
{
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hScOpenSCMnanager;
	SC_HANDLE hScOpenService;
	BOOL bQueryServiceStatus = true;
	BOOL bControlService = true;
	DWORD dwBytesNeeded;

	
	hScOpenSCMnanager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == hScOpenSCMnanager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}

	hScOpenService = OpenServiceW(
		hScOpenSCMnanager,         // SCM database 
		szSvcName,            // name of service 
		SERVICE_ALL_ACCESS);  // full access 

	if (hScOpenService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(hScOpenSCMnanager);
		return;
	}
	// Make sure the service is not already stopped.
	bQueryServiceStatus = QueryServiceStatusEx(
		hScOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)& SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);
	if (FALSE == bQueryServiceStatus)
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());

		CloseServiceHandle(hScOpenService);
		CloseServiceHandle(hScOpenSCMnanager);
		return;
	}
	bControlService = ControlService(
		hScOpenService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)& SvcStatusProcess);
	if (TRUE != bControlService) {
		DWORD err = GetLastError();
		if(err==1062)
			cout << "The service has not been started.\n";
		else
			cout << "CONTROL SERVICE FAILURED "<< GetLastError();
		CloseServiceHandle(hScOpenService);
		CloseServiceHandle(hScOpenSCMnanager);
		return;
	}
	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		bQueryServiceStatus = QueryServiceStatusEx(
			hScOpenService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)& SvcStatusProcess,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded);
		if (TRUE == bQueryServiceStatus)
		{
			cout << "QUERY FAILED";
			CloseServiceHandle(hScOpenService);
			CloseServiceHandle(hScOpenSCMnanager);
		}
	}

	CloseServiceHandle(hScOpenService);
	CloseServiceHandle(hScOpenSCMnanager);
	cout << "SERVICE STOPPED";
}

VOID __stdcall DoEnableSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 
	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}
	schService = OpenService(
		schSCManager,            // SCM database 
		szSvcName,               // name of service 
		SERVICE_CHANGE_CONFIG);  // need change config access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Change the service start type.

	if (!ChangeServiceConfig(
		schService,            // handle of service 
		SERVICE_NO_CHANGE,     // service type: no change 
		SERVICE_DEMAND_START,  // service start type 
		SERVICE_NO_CHANGE,     // error control: no change 
		NULL,                  // binary path: no change 
		NULL,                  // load order group: no change 
		NULL,                  // tag ID: no change 
		NULL,                  // dependencies: no change 
		NULL,                  // account name: no change 
		NULL,                  // password: no change 
		NULL))                // display name: no change
	{
		printf("ChangeServiceConfig failed (%d)\n", GetLastError());
	}
	else printf("Service enabled successfully.\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

void  DoDisableSvc()
{
	SC_HANDLE schSCManager;
	SC_HANDLE schService;
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		return;
	}
	schService = OpenService(
		schSCManager,            // SCM database 
		szSvcName,               // name of service 
		SERVICE_CHANGE_CONFIG);  // need change config access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return;
	}

	// Change the service start type.

	if (!ChangeServiceConfig(
		schService,        // handle of service 
		SERVICE_NO_CHANGE, // service type: no change 
		SERVICE_DISABLED,  // service start type 
		SERVICE_NO_CHANGE, // error control: no change 
		NULL,              // binary path: no change 
		NULL,              // load order group: no change 
		NULL,              // tag ID: no change 
		NULL,              // dependencies: no change 
		NULL,              // account name: no change 
		NULL,              // password: no change 
		NULL))            // display name: no change
	{
		printf("ChangeServiceConfig failed (%d)\n", GetLastError());
	}
	else printf("Service disabled successfully.\n");
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

int main(void)
{
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };							//Specifies the window station, desktop, standard handles, and appearance of the main window
	int ch = 0;
	cout << "\n---------------------------------------\n";
	cout << "\t\nPROCESS :\n";
	if (!CreateProcessW(L"C:\\Windows\\System32\\explorer.exe", NULL, NULL, NULL, NULL, 0, NULL, NULL, &si, &pi))
		cout << "\nError on creating process " << GetLastError() << endl;
	else
		cout << "\nFILE EXPLORER IS OPENED SUCCESSFULLY!\n";
	cout << "\n---------------------------------------\n";
	SearchTokenGroupsForSID();
	SERVICE_TABLE_ENTRY ste[] = { {(LPWSTR)szSvcName, (LPSERVICE_MAIN_FUNCTION)ServiceMain}, {NULL, NULL} };
	TCHAR error[256];
	if (!StartServiceCtrlDispatcher(ste))
		wsprintf(error, TEXT("Error code for StartServiceCtrlDispatcher(): %u.\n"), GetLastError());
	cout << "\n---------------------------------------\n";
	cout << "Services :\n";
	cout << "\n0) Install\n1) Start\n2) Delete\n3) Stop\n4) Enable\n5) Disable\n6) EXit \n";
	cout << "\CHOOSE : (0 / 1 / 2 / 3 / 4 / 5 / 6)\t: ";
	cin >> ch;
	do {
		if (ch == 0)
			install();
		if (ch == 1)
			start();
		else if (ch == 2)
			deleteSvc();
		else if (ch == 3)
			stop();
		else if (ch == 6) {
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			exit(0);
		}
		else if (ch == 4)
			DoEnableSvc();
		else if (ch == 5)
			DoDisableSvc();

		cout << "\n\tEnter choice :\t";
		cin >> ch;
	} while (ch != 6);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
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

typedef struct _STARTUPINFOA {
  DWORD  cb;
  LPSTR  lpReserved;
  LPSTR  lpDesktop;
  LPSTR  lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  WORD   wShowWindow;
  WORD   cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOA, *LPSTARTUPINFOA;

VOID  start() {
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;
	SC_HANDLE schSCManager, schService;
	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (schSCManager == NULL) {
		cout << "OpenSCManager Failed " << GetLastError() << endl;
		return;
	}

	schService = OpenService(schSCManager, szSvcName, SERVICE_ALL_ACCESS);
	if (schService == NULL) {
		cout << "OpenService Failed " << GetLastError() << endl;
		CloseServiceHandle(schSCManager);
		return;
	}

	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)& ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		cout << "QueryServiceStatusEx Failed " << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		cout << "Cannot start the service because it is alraedy running\n";
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
		dwWaitTime = ssStatus.dwWaitHint / 10;
		if (dwWaitTime < 1000) {
			dwWaitTime = 1000;
		}
		else if (dwWaitTime > 10000) {
			dwWaitTime = 10000;
		}
		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)& ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			cout << "QueryServiceStatusEx Failed " << GetLastError() << endl;
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else {
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
				cout << "Timeout waiting for service to stop\n";
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return;
			}
		}
	}

	if (!StartService(schService, 0, NULL)) {
		cout << "StartService Failed " << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}
	else {
		cout << "Service start pending...\n";
	}

	if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)& ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
		cout << "QueryServiceStatusEx Failed " << GetLastError() << endl;
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
		dwWaitTime = ssStatus.dwWaitHint / 10;
		if (dwWaitTime < 1000) {
			dwWaitTime = 1000;
		}
		else if (dwWaitTime > 10000) {
			dwWaitTime = 10000;
		}
		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(schService, SC_STATUS_PROCESS_INFO, (LPBYTE)& ssStatus, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
			cout << "QueryServiceStatusEx Failed " << GetLastError() << endl;
			break;
		}
		if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else {
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
				break;
		}
	}

	if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
		cout << "Service started successfully\n";
	}
	else {
		cout << "Service not started\n";
		cout << "Current State: " << ssStatus.dwCurrentState << endl;
		cout << "Exit Code: " << ssStatus.dwWin32ExitCode << endl;
		cout << "Check Point: " << ssStatus.dwCheckPoint << endl;
		cout << "Wait Hint: " << ssStatus.dwWaitHint << endl;
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

*/
