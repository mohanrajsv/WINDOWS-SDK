#include <Windows.h>
#include <iostream>
using namespace std;

#define SERVICE_NAME L"service_for_backup1"
SERVICE_STATUS        ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
HANDLE                hServiceEvent = INVALID_HANDLE_VALUE;

////methods for services
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv);
VOID WINAPI ServiceControlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
VOID ServiceReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);

void ServiceInit(DWORD dwArgc, LPTSTR* Argv);
int ServiceInstall(void);
int ServiceRemove(void);
int ServiceStart(void);
int ServiceStop(void);
void FolderBackUp(void);

int main(int argc, char* argv[])
{
	cout << "FILE BACKUP SYSTEM" << endl;
	BOOL bStServiceCtrlDispatcher = FALSE;
	int choice = 0;

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};
	bStServiceCtrlDispatcher = StartServiceCtrlDispatcher(ServiceTable);
	if (bStServiceCtrlDispatcher == FALSE)
	{
		//cout << "Main: StartServiceCtrlDispatcher returned error" << endl;
	}
	else
	{
		cout << "Success" << endl;
	}

	do
	{
		cout << "option < 1.install, 2.remove, 3.start, 4.stop, 5.exit >" << endl;
		cin >> choice;
		switch (choice)
		{
		case 1:
			if (ServiceInstall())
			{
				cout << "Installation Success" << endl;
			}
			else
			{
				cout << "Installation Failed" << endl;
			}
			break;
		case 2:
			if (ServiceRemove())
			{
				cout << "Removed Successfully" << endl;
				choice = 5;
			}
			else
			{
				cout << "Cannot Remove the service" << endl;
			}
			break;
		case 3:
			if (ServiceStart())
			{
				cout << "Started Successfully" << endl;
			}
			else
			{
				cout << "Cannot Start the service" << endl;
			}
			break;
		case 4:
			if (ServiceStop())
			{
				cout << "Stopped Successfully" << endl;
			}
			else
			{
				cout << "Cannot Stop the service" << endl;
			}
			break;
		case 5:
			break;
		default:cout << "Enter a valid option" << endl;
			break;
		}
	} while (choice != 5);

	return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR* lpArgv)
{
	BOOL bServiceStatus = FALSE;

	hServiceStatusHandle = RegisterServiceCtrlHandlerW(SERVICE_NAME, ServiceControlHandler);
	if (hServiceStatusHandle == NULL)
	{
		//cout << "RegisterCtrlHandler Failed" << GetLastError << endl;
	}
	else
	{
		//cout << "RegisterCtrlHandler Success" << endl;
	}
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (bServiceStatus == FALSE)
	{
		//cout << "ServiceStatus Init Failed" << endl;
	}
	else
	{
		//cout << "ServiceStatus Init Success" << endl;
	}

	HANDLE hThread = NULL;
	hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	ServiceInit(dwArgc, lpArgv);
}

VOID WINAPI ServiceControlHandler(DWORD dwControl)
{
	//cout << "In ServiceControlHandler" << endl;
	if (dwControl == SERVICE_CONTROL_STOP)
	{
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		//cout << "Service Stopped" << endl;
	}
}

VOID ServiceReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;
	//cout << "In ServiceReportStatus :" << endl;

	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
	{
		ServiceStatus.dwControlsAccepted = 0;
	}
	else
	{
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		ServiceStatus.dwCheckPoint = 0;
	}
	else
	{
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}

	bSetServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (bSetServiceStatus == FALSE)
	{
		//cout << "ServiceStatus Failed" << endl;
		return;
	}
	else
	{
		//cout << "ServiceStatus Success" << endl;
	}
	//cout << "Service Status : End" << endl;
}

void ServiceInit(DWORD dwArgc, LPTSTR* Argv)
{
	//cout << "In Service Init" << endl;
	hServiceEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	if (hServiceEvent == NULL)
	{
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	else
	{
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}

	WaitForSingleObject(hServiceEvent, INFINITE);
	ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
}

int ServiceInstall(void)
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScCreateService = NULL;
	DWORD dwGetModuleFileName = 0;
	TCHAR sPath[MAX_PATH];

	//cout << "In Service Install" << endl;

	dwGetModuleFileName = GetModuleFileNameW(NULL, sPath, MAX_PATH);
	if (dwGetModuleFileName == 0)
	{
		//cout << "Service Installation Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "Installation Success" << endl;
	}

	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == 0)
	{
		//cout << "OpenSCManager Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenSCManager Success" << endl;
	}

	hScCreateService = CreateService(hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		sPath, NULL, NULL,
		NULL,
		NULL,
		NULL);
	if (hScCreateService == NULL)
	{
		//cout << "Create Service Failed" << GetLastError() << endl;
		CloseServiceHandle(hScOpenSCManager);
		return 0;
	}
	else
	{
		//cout << "Create Service Success" << endl;
	}

	CloseServiceHandle(hScCreateService);
	CloseServiceHandle(hScOpenSCManager);

	//cout << "Service Install : End" << endl;
	return 1;
}

int ServiceRemove(void)
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bRemoveService = FALSE;

	//cout << "In service remove" << endl;

	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == 0)
	{
		//cout << "OpenSCManager Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenSCManager Success" << endl;
	}

	hScOpenService = OpenService(hScOpenSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (hScOpenService == 0)
	{
		//cout << "OpenService Failed" << GetLastError() << endl;
		//CloseHandle(hScOpenSCManager);
		return 0;
	}
	else
	{
		//cout << "OpenService Success" << endl;
		if (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
		{
			ServiceStop();
		}
	}



	bRemoveService = DeleteService(hScOpenService);
	if (bRemoveService == FALSE)
	{
		//cout << "Remove Service Failed" << GetLastError() << endl;

		return 0;
	}
	else
	{
		//cout << "Remove Service Success" << endl;
	}

	//cout << "Service Remove : End";
	return 1;
}

int ServiceStart(void)
{
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS svcStatusProcess;
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwBytesNeeded = NULL;

	//cout << "In Service Start" << endl;

	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == 0)
	{
		//cout << "OpenSCManager Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenSCManager Success" << endl;
	}

	hScOpenService = OpenService(hScOpenSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (hScOpenService == 0)
	{
		//cout << "OpenService Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenService Success" << endl;
	}

	bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)&svcStatusProcess, sizeof(svcStatusProcess), &dwBytesNeeded);
	if (bQueryServiceStatus == 0)
	{
		//cout << "QueryService Failed" << GetLastError() << endl;
		CloseHandle(hScOpenSCManager);
		CloseHandle(hScOpenService);
		return 0;
	}
	else
	{
		//cout << "QueryService Success" << endl;
	}

	if ((svcStatusProcess.dwCurrentState != SERVICE_STOPPED) && (svcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING))
	{
		//cout << "Service is already running" << endl;
		return 0;
	}
	else
	{
		//cout << "Service is already stopped" << endl;
	}

	while (svcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING)
	{
		bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)&svcStatusProcess, sizeof(svcStatusProcess), &dwBytesNeeded);
		if (bQueryServiceStatus == 0)
		{
			//cout << "QueryService Failed" << GetLastError() << endl;
			CloseHandle(hScOpenSCManager);
			CloseHandle(hScOpenService);
			return 0;
		}
		else
		{
			//cout << "QueryService Success" << endl;
		}
	}

	bStartService = StartServiceW(hScOpenService, NULL, NULL);
	if (bStartService == 0)
	{
		//cout << "Start Service Failed" << GetLastError() << endl;
		CloseHandle(hScOpenSCManager);
		CloseHandle(hScOpenService);
		return 0;
	}
	else
	{

		//cout << "Start Service Success" << endl;
	}

	bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)&svcStatusProcess, sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded);
	if (bQueryServiceStatus == 0)
	{
		//cout << "QueryService Failed" << GetLastError() << endl;
		CloseHandle(hScOpenSCManager);
		CloseHandle(hScOpenService);
		return 0;
	}
	else
	{
		//cout << "QueryService Success" << endl;
	}

	if (svcStatusProcess.dwCurrentState != SERVICE_RUNNING)
	{
		//cout << "Service started running" << endl;
	}
	else
	{
		//cout << "Service running failed" << endl;
		CloseHandle(hScOpenSCManager);
		CloseHandle(hScOpenService);
		return 0;
	}

	//cout << "Service start : End" << endl;
	return 1;
}

int ServiceStop(void)
{
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS svcStatusProcess = { 0 };
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	BOOL bControlService = FALSE;
	DWORD dwBytesNeeded = NULL;

	//cout << "In Service Stop" << endl;

	hScOpenSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == 0)
	{
		//cout << "OpenSCManager Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenSCManager Success" << endl;
	}

	hScOpenService = OpenService(hScOpenSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
	if (hScOpenService == 0)
	{
		//cout << "OpenService Failed" << GetLastError() << endl;
		return 0;
	}
	else
	{
		//cout << "OpenService Success" << endl;
	}

	bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)&svcStatusProcess, sizeof(svcStatusProcess), &dwBytesNeeded);
	if (bQueryServiceStatus == FALSE)
	{
		//cout << "QueryService Failed" << GetLastError() << endl;
		CloseHandle(hScOpenSCManager);
		CloseHandle(hScOpenService);
		return 0;
	}
	else
	{
		//cout << "QueryService Success" << endl;
	}

	bControlService = ControlService(hScOpenService, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&svcStatusProcess);
	if (bControlService == FALSE)
	{
		//cout << "Control Service Fails" << endl;

		return 0;
	}
	else
	{
		//cout << "Control Service Success" << endl;
	}

	while (svcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		bQueryServiceStatus = QueryServiceStatusEx(hScOpenService, SC_STATUS_PROCESS_INFO, (LPBYTE)&svcStatusProcess, sizeof(svcStatusProcess), &dwBytesNeeded);
		if (bQueryServiceStatus == 0)
		{
			//cout << "QueryService Failed" << GetLastError() << endl;
			CloseHandle(hScOpenSCManager);
			CloseHandle(hScOpenService);
			return 0;
		}
		else
		{
			//cout << "QueryService Success" << endl;
		}

		if (svcStatusProcess.dwCurrentState == SERVICE_STOPPED)
		{

			//cout << "Service stopped successfully" << endl;
			break;
		}
		else
		{
			//cout << "Stop service failed" << GetLastError() << endl;
			CloseHandle(hScOpenSCManager);
			CloseHandle(hScOpenService);
			return 0;
		}
	}

	//cout << "Service stop : End" << endl;
	return 1;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	while (WaitForSingleObject(hServiceEvent, 0) != WAIT_OBJECT_0)
	{
		FolderBackUp();
		Sleep(5000);
	}
	Sleep(3000);
	return ERROR_SUCCESS;
}

///funtion for getting backup
void FolderBackUp(void)
{
	SHFILEOPSTRUCT sf = { 0 };
	memset(&sf, 0, sizeof(sf));
	sf.hwnd = 0;
	sf.wFunc = FO_COPY;
	sf.pFrom = L"C:\\mohanraj_new";
	sf.pTo = L"C:\\mohanraj_BACKUP";
	sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI;
	SHFileOperation(&sf);
}
