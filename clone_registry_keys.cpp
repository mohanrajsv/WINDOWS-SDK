#include <windows.h>
#include <tchar.h>
#include<bits/stdc++.h>
using namespace std;
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 20000

void QueryKey(HKEY hKey,HKEY hDestKey) 
{
    TCHAR achClass[MAX_PATH];                   // buffer for class name & max_path sizw is 260 here
    DWORD cchClassName = MAX_PATH;                // size of class string 
    DWORD cSubKeys=0;                            // number of subkeys 
    DWORD cbMaxSubKey;                           // longest subkey size 
    DWORD cchMaxClass;                           // longest class string 
    DWORD cValues;                               // number of values for key 
    DWORD type;                                
    DWORD cchMaxValue;                          // longest value name 
    DWORD cbMaxValueData;                       // longest value data 
    DWORD cbSecurityDescriptor;                 // size of security descriptor 
    FILETIME ftLastWriteTime;                    // last write time 
    DWORD i, retCode,result; 
    TCHAR achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
    cchValue++;
    LPSTR lpName = NULL, lpData = NULL;
    retCode = RegQueryInfoKey(
        hKey,                    
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
 	lpName = (LPSTR)malloc(50*cchMaxValue);
    lpData = (LPSTR)malloc(50*cbMaxValueData);
    if(retCode!=ERROR_SUCCESS)
        return;

    if (cValues) 
    {
        system("cls");
        for (i=0;i<cValues; i++) 
        {
            cchValue = cchMaxValue; 
            DWORD dSize=cbMaxValueData;
            retCode = RegEnumValue(hKey, i, lpName, &cchValue, NULL, &type, (LPBYTE)lpData, &dSize);
            
            RegSetValueEx(hDestKey, lpName, 0, type, (LPBYTE)lpData, dSize);
        }
    }
	
        for (DWORD idx=0;idx<cSubKeys; idx++) 
        {
            HKEY hKeySrc,hKeyDest;
            DWORD namesz=cbMaxSubKey;
            cchValue = MAX_VALUE_NAME;
            RegEnumKeyEx(hKey, idx, achClass, &cchValue, NULL, NULL, NULL, NULL);
            result = RegOpenKeyEx(hKey, achClass, 0L, KEY_READ, &hKeySrc);

		    if (result != ERROR_SUCCESS)
			    return;
            RegCreateKeyEx(hDestKey, achClass, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyDest, NULL);

            QueryKey(hKeySrc,hKeyDest);                  //recursivly calling for subkey of subkey and so on... :(

            RegCloseKey(hKeySrc);
		    RegCloseKey(hKeyDest);
        }
	lpName = NULL;
}

int main(void)
{
    bool src = true, dest = true;
    HKEY hTestKey,hDestKey;
    if(RegOpenKeyEx( HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\DCAgent"),0,KEY_READ,&hTestKey) != ERROR_SUCCESS)
    {
       cout<<"\nSOURCE KEY ERROR\n";
       src=false;
    }

    if(RegOpenKeyEx( HKEY_LOCAL_MACHINE,TEXT("SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\zohocopy1"),0,KEY_READ,&hDestKey) == ERROR_FILE_NOT_FOUND)
    {
       if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Wow6432Node\\AdventNet\\DesktopCentral\\zohocopy1", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hDestKey, NULL) != ERROR_SUCCESS)
       dest=false;
    }

    if(src==false || dest==false)
        cout<<"ERROR HAS BEEN OCCURED";
    else
    {
        QueryKey(hTestKey,hDestKey);
        cout<<"\n\t\tdone\n";
    }

    RegCloseKey(hTestKey);
    RegCloseKey(hDestKey);
    return 0;
}
