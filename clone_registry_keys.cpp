#include <windows.h>
#include <tchar.h>
#include<bits/stdc++.h>
using namespace std;

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 20000

void QueryKey(HKEY hKey) 
{
    TCHAR achClass[MAX_PATH];  // buffer for class name & max_path sizw is 260 here
    DWORD cchClassName = MAX_PATH;              // size of class string 
    DWORD cSubKeys=0;                          // number of subkeys 
    DWORD cbMaxSubKey;                      // longest subkey size 
    DWORD cchMaxClass;                  // longest class string 
    DWORD cValues;              // number of values for key 
    DWORD cchMaxValue;          // longest value name 
    DWORD cbMaxValueData;       // longest value data 
    DWORD cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;                     // last write time 
 
    DWORD i, retCode; 

    TCHAR achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 
 
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
 
    if (cValues) 
    {
        for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        { 
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL,NULL, NULL);

            if (retCode == ERROR_SUCCESS ) 
            { 
                cout<<i+1<<" "<<achValue<<endl;
            } 
        }
    }
}

int main(void)
{
   HKEY hTestKey;
   if( RegOpenKeyEx( HKEY_LOCAL_MACHINE,
        TEXT("Hardware\\Description\\system\\CentralProcessor\\0"),0,KEY_READ,&hTestKey) == ERROR_SUCCESS
      )
   {
      QueryKey(hTestKey);
   }
   
   RegCloseKey(hTestKey);
   return 0;
}
