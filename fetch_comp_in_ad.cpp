#include "stdafx.h"
#include <wchar.h>
#include <activeds.h>
#include <time.h> 
#include <fstream>

HRESULT FindAllComputers();
static int totalComputers = 0;
wfstream fio; 

int main(int argc, char* argv[])
{

	//Initialize COM
	CoInitialize(NULL);
	HRESULT hr = S_OK;
	clock_t t; 
    t = clock(); 

	hr = FindAllComputers();
	if (FAILED(hr))
	  wprintf(L"Search for all computers failed with hr: %d\n", hr);
	t = clock() - t; 

    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds  
	cout << "\n\n\t\tTotal computers fetched : " << totalComputers << endl;
    cout <<"\n\t\tTime Taken to fetch  "<<  time_taken << "seconds" << endl;

	CoUninitialize();
	Sleep(5000);
	return 0;
}

 HRESULT FindAllComputers()
{
	
	fio.open("computers.txt"); 

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
			totalComputers++; //for counting no of computers
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
			fio <<"----------------------------------------------------\n";
		}
		// Close the search handle to clean up
        pGCSearch->CloseSearchHandle(hSearch);
	} 

	fio.close();

cleanUp:
	if (pGCSearch)
		pGCSearch->Release();
    return hr;
}
