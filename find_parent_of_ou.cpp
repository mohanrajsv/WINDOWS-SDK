/*
Author : Mohanraj S
Date : 08/01/2019
Description : To find the parent of the child ou

*/

#include "stdafx.h"
#include "stdafx.h"
#include <stdio.h>
#include <wchar.h>
#include <objbase.h>
#include <activeds.h>
#include <iostream>
#define INPUT_BUFF_SIZE     MAX_PATH
#define SEARCH_FILTER_SIZE  INPUT_BUFF_SIZE + 64
using namespace std;


HRESULT GetParentObject(IADs *pObject, //Pointer the object whose parent to bind to.
                        IADs **ppParent //Return a pointer to the parent object.
                        );

HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPOLESTR szFindUser, //Name of user to find.
                       IADs **ppUser); //Return a pointer to the user

int main( int argc, wchar_t *argv[])
{
    int inputsize = 0;

    OLECHAR szBuffer[INPUT_BUFF_SIZE] = {0};
    if (NULL == argv[1])
    {
		cout << "Enter the OU name to  find its parent : ";
		fgetws(szBuffer,ARRAYSIZE(szBuffer),stdin);

        inputsize = wcsnlen(szBuffer, ARRAYSIZE(szBuffer));
        if (0 < inputsize && '\n' == szBuffer[inputsize-1])	//cancelling newline char here...
            szBuffer[inputsize-1] = '\0';
    }
    else
        wcscpy_s(szBuffer, ARRAYSIZE(szBuffer), argv[1]);

    //Intialize COM
	HRESULT hr= S_OK;
	hr = CoInitialize(NULL);
	if(FAILED(hr))
	{
		cout << "CoInitialize failed..! " << endl ;
		return 1;
	}
   

	IADs *pObject = NULL,
		*pParent = NULL;
	IDirectorySearch *pDS = NULL;
	OLECHAR szPath[MAX_PATH];
	VARIANT var;
	BSTR bstr = NULL,
		bstrC = NULL;
	VariantInit(&var);


    //Get rootDSE and the domain container's DN.
    hr = ADsOpenObject(L"LDAP://rootDSE",
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
        IID_IADs,
        (void**)&pObject);
    if (FAILED(hr))
    {
        wprintf(L"Not Found. Could not bind to the domain. hr=0x%x\n", hr);
        goto CleanUp;
    }

    BSTR bNamingContext = NULL;
    bNamingContext = SysAllocString(L"defaultNamingContext");
    if (NULL == bNamingContext)
    {
        wprintf(L"SysAllocString for defaultNamingContext failed.\n");
        goto CleanUp;
    }

    hr = pObject->Get(bNamingContext,&var);
    SysFreeString(bNamingContext);
    bNamingContext = NULL;

    if (FAILED(hr))
    {
        wprintf(L"Get for %s failed. hr=0x%x\n", bNamingContext, hr);
        goto CleanUp;
    }

    wcscpy_s(szPath, ARRAYSIZE(szPath), L"LDAP://"); // If you're running on NT 4.0 or Win9.x machine, you need to 
    // add the server name e.g L"LDAP://myServer"
    wcscat_s(szPath, ARRAYSIZE(szPath), var.bstrVal);

    //Bind to the root of the current domain.
    hr = ADsOpenObject(szPath,
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
        IID_IDirectorySearch,
        (void**)&pDS);
    if (FAILED(hr))
    {
        wprintf (L"ADsOpenObject failed for path %s. hr=0x%x\n", szPath, hr);
        goto CleanUp;
    }

    if (pObject)
    {
        pObject->Release();
        pObject = NULL;
    }

	////////calling FindUserByName to get user name via pointer
    hr =  FindUserByName(pDS, //Container to search
        szBuffer, //Name of user to find.
        &pObject); //Return a pointer to the user
    if (FAILED(hr))
    {
        wprintf(L"User \"%s\" not Found.\n",szBuffer);
        wprintf (L"FindUserByName failed with the following HR: 0x%x\n", hr);
        goto CleanUp;
    }

	//////////calling GetParentObject to get parent obj via pointer
    hr = GetParentObject(pObject, //Pointer the object whose parent to bind to.
        &pParent //Return a pointer to the parent object.
        );
    if (FAILED(hr))
    {
		cout << "GET PARENT IS FAILED WITH hr = " << hr << endl;
        goto CleanUp;
    }   

    //Get ADsPath
    hr = pParent->get_ADsPath(&bstr);
    if (FAILED(hr))
    {
		cout << "ADS PATH FIND FAILED hr = " << hr << endl;
        goto CleanUp;
    }

    //Get the distinguishedName property
    BSTR bDName = SysAllocString(L"distinguishedName");
    if (NULL == bDName)
    {
		cout << "SysAllocation FAILED hr = " << hr << endl;
        goto CleanUp;
    }

    VariantClear(&var);

    wprintf(L"\n Parent : %s\n",bstr);

CleanUp:

    if (bstr)
    {
        FreeADsStr(bstr);
        bstr = NULL;
    }
    if (bstrC)
    {
        FreeADsStr(bstrC);
        bstrC = NULL;
    }
    VariantClear(&var);

    if (pObject)
    {
        pObject->Release();
        pObject = NULL;
    }
    if (pDS)
    {
        pDS->Release();
        pDS = NULL;
    }
    if (pParent)
    {
        pParent->Release();
        pParent = NULL;
    }

    //Uninitalize COM
    CoUninitialize();
	Sleep(10000);
    return FAILED(hr)?1:0;
}

HRESULT GetParentObject(IADs *pObject, //Pointer the object whose parent to bind to.
                        IADs **ppParent //Return a pointer to the parent object.
                        )
{
    if ((!pObject)||(!ppParent))
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    BSTR bstr = NULL;
    hr = pObject->get_Parent(&bstr);

    if (SUCCEEDED(hr))
    {
        //Bind to the parent container.
        *ppParent = NULL;
        hr = ADsOpenObject(bstr,
            NULL,
            NULL,
            ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
            IID_IADs,
            (void**)ppParent);
        if(FAILED(hr))
        {
            if ((*ppParent))
            {
                (*ppParent)->Release();
                (*ppParent) = NULL;
            }
        }
    }
    if (bstr)
    {
        FreeADsStr(bstr);
    }
    return hr;
}





HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPOLESTR szFindUser, //Name of user to find.
                       IADs **ppUser) //Return a pointer to the user
{
    HRESULT hr = S_OK;

    if ((!pSearchBase)||(!szFindUser))
        return E_INVALIDARG;

    //Create search filter
    OLECHAR szSearchFilter[SEARCH_FILTER_SIZE] = {0};
    OLECHAR szADsPath[MAX_PATH] = {0};
    wcscpy_s(szSearchFilter, ARRAYSIZE(szSearchFilter), L"(&(objectCategory=organizationalUnit)(objectClass=organizationalUnit)(Name=");
    wcscat_s(szSearchFilter, ARRAYSIZE(szSearchFilter), szFindUser);
    wcscat_s(szSearchFilter, ARRAYSIZE(szSearchFilter), L"))");

    //Search entire subtree from root.
    ADS_SEARCHPREF_INFO SearchPrefs;
    SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
    SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
    SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
    DWORD dwNumPrefs = 1;

    // COL for iterations
    ADS_SEARCH_COLUMN col = {0};
    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch = NULL;
    // Set the search preference

    hr = pSearchBase->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

    // Set attributes to return
    LPOLESTR pszAttribute[1] = {L"ADsPath"};

    // Execute the search
    hr = pSearchBase->ExecuteSearch(szSearchFilter,
        pszAttribute,
        1,
        &hSearch
        );

    DWORD noUsersFound = 0; 
    if (SUCCEEDED(hr))
    {    

        // Call IDirectorySearch::GetNextRow() to retrieve the next row 
        //of data
        while( pSearchBase->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
        {	

            if(noUsersFound>0 && ((*ppUser) != NULL))
				(*ppUser)->Release();
            
            // Get the data for this column
            hr = pSearchBase->GetColumn( hSearch, pszAttribute[0], &col );
            if ( SUCCEEDED(hr) )
            {
                // Print the data for the column and free the column
                // Note the attribute we asked for is type CaseIgnoreString.
                wcscpy_s(szADsPath, ARRAYSIZE(szADsPath), col.pADsValues->CaseIgnoreString); 					
                wprintf(L"\n Child : %s\r\n",col.pADsValues->CaseIgnoreString); 
                hr = ADsOpenObject(szADsPath,
                    NULL,
                    NULL,
                    ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                    IID_IADs,
                    (void**)ppUser);
                if(SUCCEEDED(hr)) noUsersFound ++;

                pSearchBase->FreeColumn( &col );
            }
        }

    }
	if (noUsersFound > 1)
	{
		cout << " \n There are " << noUsersFound << "Childs found " << endl << "\t---------------------- " << endl;
	}

    // Close the search handle to clean up
    if (hSearch)
    {
        pSearchBase->CloseSearchHandle(hSearch);
        hSearch = NULL;
    }

    if(0 == noUsersFound) 
        hr = E_FAIL;

    return hr;
}
