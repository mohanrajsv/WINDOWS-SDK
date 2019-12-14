#include <windows.h>
#include <tchar.h>
#include <iostream>
using namespace std;
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 20000

void QueryKey(HKEY hSrcKey, HKEY hDestKey)
{
    /*
    LSTATUS RegQueryInfoKeyW(
        HKEY      hKey,
        LPWSTR    lpClass,
        LPDWORD   lpcchClass,
        LPDWORD   lpReserved,
        LPDWORD   lpcSubKeys,
        LPDWORD   lpcbMaxSubKeyLen,
        LPDWORD   lpcbMaxClassLen,
        LPDWORD   lpcValues,
        LPDWORD   lpcbMaxValueNameLen,
        LPDWORD   lpcbMaxValueLen,
        LPDWORD   lpcbSecurityDescriptor,
        PFILETIME lpftLastWriteTime
    );*/

    TCHAR achClass[MAX_PATH];                   // buffer for class name & max_path sizw is 260 here
    DWORD cchClassName = MAX_PATH;                // size of class string 
    DWORD cSubKeys = 0;                            // number of subkeys 
    DWORD cbMaxSubKey;                           // longest subkey size 
    DWORD cchMaxClass;                           // longest class string 
    DWORD cValues;                               // number of values for key 
    DWORD type;
    DWORD cchMaxValue;                          // longest value name 
    DWORD cbMaxValueData;                       // longest value data 
    DWORD cbSecurityDescriptor;                 // size of security descriptor 
    FILETIME ftLastWriteTime;                    // last write time 
    DWORD i, retCode, result;
    DWORD cchValue = MAX_VALUE_NAME;
    ///cchValue++;
    LPSTR lpName = NULL, lpData = NULL;

    retCode = RegQueryInfoKeyW(
        hSrcKey,
        achClass,
        &cchClassName,
        NULL,
        &cSubKeys,
        &cbMaxSubKey,
        &cchMaxClass,
        &cValues,
        &cchMaxValue,
        &cbMaxValueData,
        &cbSecurityDescriptor,
        &ftLastWriteTime);
    if (retCode != ERROR_SUCCESS)
    {
        return;
    }
    cbMaxSubKey++;
    cchMaxValue++;

    if (cchMaxValue > cbMaxSubKey)
        cbMaxSubKey = cchMaxValue;
    
    lpName = (LPSTR)malloc(sizeof(DWORD)*cchMaxValue);
    lpData = (LPSTR)malloc(sizeof(DWORD) * cbMaxValueData);
    if (cValues)
    {
        for (int i = 0; i < cValues; i++)
        {
            cchValue = cchMaxValue;
            cchValue++;
            DWORD dSize = cbMaxValueData;
            dSize++;
            retCode = RegEnumValueW(hSrcKey, i, (LPWSTR)lpName, &cchValue, NULL, &type, (LPBYTE)lpData, &dSize);
            RegSetValueExW(hDestKey, (LPCWSTR)lpName, 0, type, (LPBYTE)lpData, dSize);
        }
    }
    if (lpData != NULL) {
        //free(lpData);
        lpData = NULL;
    }


    for (DWORD idx = 0; idx < cSubKeys; idx++)
    {
        HKEY hKeySrc, hKeyDest;
        DWORD namesz = cbMaxSubKey;
        namesz++;
        RegEnumKeyEx(hSrcKey, idx, (LPWSTR)lpName, &namesz, NULL, NULL, NULL, NULL);
        result = RegOpenKeyExW(hSrcKey, (LPWSTR)lpName, 0L, KEY_READ, &hKeySrc);

        if (result != ERROR_SUCCESS)
            return;
        RegCreateKeyExW(hDestKey, (LPWSTR)lpName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyDest, NULL);
        
        QueryKey(hKeySrc, hKeyDest);                  //recursivly calling for subkey of subkey and so on... :(

        RegCloseKey(hKeySrc);
        RegCloseKey(hKeyDest);
    }
    
    lpName = NULL;
}


int main(void)
{
    bool src = true, dest = true;
    HKEY hTestKey, hDestKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\DCAgent"), 0, KEY_READ, &hTestKey) != ERROR_SUCCESS)
    {
        src = false;
    }
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\clone"), 0, KEY_READ, &hDestKey) == ERROR_FILE_NOT_FOUND)
    {
        if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\clone", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hDestKey, NULL) != ERROR_SUCCESS)
            dest = false;
    }
    if (src == false || dest == false)
        cout << "ERROR HAS BEEN OCCURED";
    else
    {
        QueryKey(hTestKey, hDestKey);
        cout << " \t\t Cloning Mission completed \n " <<endl ;
    }
    RegCloseKey(hTestKey);
    RegCloseKey(hDestKey);
    return 0;
}
