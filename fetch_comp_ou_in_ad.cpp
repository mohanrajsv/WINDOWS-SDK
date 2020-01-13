#include "stdafx.h"
#include <wchar.h>
#include <activeds.h>
#include <time.h> 
#include <fstream>

HRESULT FindAllComputers();
HRESULT FindAllOu();
#define INPUT_BUFF_SIZE     MAX_PATH
#define SEARCH_FILTER_SIZE  INPUT_BUFF_SIZE + 64
static int totalCount = 0;
wfstream fio; 

HRESULT GetParentObject(IADs *pObject, //Pointer the object whose parent to bind to.
                        IADs **ppParent //Return a pointer to the parent object.
                        );
HRESULT FindUserByName(IDirectorySearch *pSearchBase, //Container to search
                       LPOLESTR szFindUser, //Name of user to find.
                       IADs **ppUser); //Return a pointer to the user

int main(int argc, char* argv[])
{
	CoInitialize(NULL);//Initialize COM
	HRESULT hr = S_OK;
	clock_t t; 
    t = clock(); 

	//fetching computers
	hr = FindAllComputers();
	if (FAILED(hr))
	  wprintf(L"Search for all computers failed with hr: %d\n", hr);

	//fetching OU
	hr = FindAllOu();
	if (FAILED(hr))
	  wprintf(L"Search for all OU failed with hr: %d\n", hr);

	t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds  
	cout << "\n\n\t\tTotal object fetched : " << totalCount << endl;
    cout <<"\n\t\tTime Taken to fetch  "<<  time_taken << "seconds" << endl;

	CoUninitialize();
	Sleep(10000);
	return 0;
}


HRESULT FindAllComputers()
{
	fio.open("computers.txt",fstream::out);
	cout << "\t\tFetching all computers please wait... " <<endl;
    HRESULT hr = E_FAIL;
    HRESULT hrGC = E_FAIL;
	VARIANT var;
	ULONG lFetch;

    // Interface Pointers
    IDirectorySearch *pGCSearch = NULL;
	IADsContainer *pContainer = NULL;
    IADs *pADs = NULL;
	IADs    *pRoot=NULL;
	VARIANT varDSRoot;
	OLECHAR szPath[MAX_PATH];
	BSTR bNamingContext = NULL;

	//Bind to Global Catalog
    hr = ADsOpenObject(L"GC:",  
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADsContainer,
				 (void**)&pContainer);

	//geting root domain RootSDE
	hr = ADsGetObject(L"LDAP://RootDSE",IID_IADs,(void**)&pRoot);
    bNamingContext = SysAllocString(L"defaultNamingContext");
    hr = pRoot->Get(bNamingContext,&var);
    SysFreeString(bNamingContext);
    bNamingContext = NULL;

    wcscpy_s(szPath, ARRAYSIZE(szPath), L"LDAP://"); 
    wcscat_s(szPath, ARRAYSIZE(szPath), var.bstrVal);

	 hr = ADsOpenObject(szPath,
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION, //  Use Secure Authentication.
				IID_IDirectorySearch,
				(void**)&pGCSearch);

	//Create search filter
	LPOLESTR pszSearchFilter = L"(objectCategory=computer)";

    //Search entire subtree from root.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = 100000;

    DWORD dwNumPrefs = 1;

	// COL for iterations
    ADS_SEARCH_COLUMN col;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
	
	// Set the search preference
    hr = pGCSearch->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

	// Set attributes to return
	CONST DWORD dwAttrNameSize = 2;
    LPOLESTR pszAttribute[dwAttrNameSize] = {L"Name",L"distinguishedName"};

    // Execute the search
    hr = pGCSearch->ExecuteSearch(pszSearchFilter,
		                          pszAttribute,
								  dwAttrNameSize,
								  &hSearch
								  );
	if ( SUCCEEDED(hr) )
	{    
        while( pGCSearch->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
		{
			totalCount++; //for counting no of computers
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {
				hr = pGCSearch->GetColumn( hSearch, pszAttribute[x], &col );
				if ( SUCCEEDED(hr) )
				{
					fio <<pszAttribute[x] << " : "<<col.pADsValues->CaseIgnoreString <<"\n"; //writing to a file
					//wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
					pGCSearch->FreeColumn( &col );
				}
				else
				{
					wprintf(L"Get for failed. hr=0x%x\n", hr);
					goto cleanUp;
				}
            }
			fio <<  " ----------------------------------------------------\n";
			//cout << " ----------------------------------------------------\n";
		}
        pGCSearch->CloseSearchHandle(hSearch); // Close the search handle to clean up
	} 
	fio.close();

cleanUp:
	if (pGCSearch)
		pGCSearch->Release();
    return hr;
}

 HRESULT FindAllOu()
{
	fio.open("ou.txt",fstream::out);
	//fio.open("ou.txt"); 
	cout << "\t\tFetching all OU please wait... " <<endl;
    HRESULT hr = E_FAIL;
    HRESULT hrGC = E_FAIL;
	VARIANT var;
	ULONG lFetch;

    // Interface Pointers
    IDirectorySearch *pGCSearch = NULL;
	IDirectorySearch *pDS = NULL;

	IADsContainer *pContainer = NULL;
    IADs *pADs = NULL;
	IADs    *pRoot=NULL;
	VARIANT varDSRoot;
	OLECHAR szPath[MAX_PATH];
	BSTR bNamingContext = NULL;
		IADs *pObject = NULL,
		*pParent = NULL;
	WCHAR szBuffer[512] = {0};
	BSTR bstr = NULL,
		bstrC = NULL;
	//Bind to Global Catalog
    hr = ADsOpenObject(L"GC:",  
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADsContainer,
				 (void**)&pContainer);

	//geting root domain RootSDE
	hr = ADsGetObject(L"LDAP://RootDSE",IID_IADs,(void**)&pRoot);
    bNamingContext = SysAllocString(L"defaultNamingContext");
    hr = pRoot->Get(bNamingContext,&var);
    SysFreeString(bNamingContext);
    bNamingContext = NULL;

    wcscpy_s(szPath, ARRAYSIZE(szPath), L"LDAP://"); 
    wcscat_s(szPath, ARRAYSIZE(szPath), var.bstrVal);

	 hr = ADsOpenObject(szPath,
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION, //  Use Secure Authentication.
				IID_IDirectorySearch,
				(void**)&pGCSearch);

	//Create search filter
	LPOLESTR pszSearchFilter = L"(objectCategory=organizationalunit)";

    //Search entire subtree from root.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_PAGESIZE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = 100000;

    DWORD dwNumPrefs = 1;

	// COL for iterations
    ADS_SEARCH_COLUMN col;

    // Handle used for searching
    ADS_SEARCH_HANDLE hSearch;
	
	// Set the search preference
    hr = pGCSearch->SetSearchPreference( &SearchPrefs, dwNumPrefs);
    if (FAILED(hr))
        return hr;

	// Set attributes to return
	CONST DWORD dwAttrNameSize = 2;
    LPOLESTR pszAttribute[dwAttrNameSize] = {L"Name",L"distinguishedName"};

    // Execute the search
    hr = pGCSearch->ExecuteSearch(pszSearchFilter,
		                          pszAttribute,
								  dwAttrNameSize,
								  &hSearch
								  );
	if ( SUCCEEDED(hr) )
	{    
        while( pGCSearch->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
		{
			totalCount++; //for counting no of computers
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {
				hr = pGCSearch->GetColumn( hSearch, pszAttribute[x], &col );
				if ( SUCCEEDED(hr) )
				{
					if(x==0)					
					{
						hr = ADsOpenObject(L"LDAP://rootDSE",
						NULL,
						NULL,
						ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
						IID_IADs,
						(void**)&pObject);

						hr =  FindUserByName(pGCSearch, //Container to search
						col.pADsValues->CaseIgnoreString, //Name of user to find.
						&pObject); //Return a pointer to the user

						 hr = GetParentObject(pObject, //Pointer the object whose parent to bind to.
						 &pParent //Return a pointer to the parent object.
						 );

						 if (FAILED(hr))
						{
							cout << "GET PARENT IS FAILED WITH hr = " << hr << endl;
							// goto CleanUp;
						} 
						 if( S_OK == pParent->get_Name(&bstr) ) {
							fio <<"Parent OU: " << bstr << "\n";
						}
						//  hr = pParent->get_(&bstr);
						 // fio << "Parent of OU : " << bstr << "\n";
						 // wprintf(L"Parent OU: %s\n",bstr);
					}

					fio <<pszAttribute[x] << " : "<<col.pADsValues->CaseIgnoreString <<"\n"; //writing to a file
					//wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
					
					pGCSearch->FreeColumn( &col );
				}
				else
				{
					wprintf(L"Get for failed. hr=0x%x\n", hr);
					goto cleanUp;
				}
            }
			fio << "---------------------------------------------------\n";
			//cout << " ----------------------------------------------------\n";
		}
        pGCSearch->CloseSearchHandle(hSearch); // Close the search handle to clean up
	} 
	fio.close();

cleanUp:
	if (pGCSearch)
		pGCSearch->Release();
    return hr;
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
              //  wprintf(L"\n Child : %s\r\n",col.pADsValues->CaseIgnoreString); 
                hr = ADsOpenObject(szADsPath,
                    NULL,
                    NULL,
                    ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
                    IID_IADs,
                    (void**)ppUser);
                if(SUCCEEDED(hr)) noUsersFound ++;

                pSearchBase->FreeColumn( &col );
				break;
            }
        }
	}
    
    if (hSearch)
    {
        pSearchBase->CloseSearchHandle(hSearch);
        hSearch = NULL;
    }
    if(0 == noUsersFound) 
        hr = E_FAIL;
    return hr;
}

