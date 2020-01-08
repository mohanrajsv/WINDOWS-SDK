#include "stdafx.h"
#include <wchar.h>
#include <activeds.h>
#include <time.h> 
HRESULT FindAllUsersInGC();
static int num = 0;
int main(int argc, char* argv[])
{

	//Initialize COM
	CoInitialize(NULL);

	HRESULT hr = S_OK;
	wprintf(L"This program is to find all OU \n");
	wprintf(L"------------------------------\n");
	clock_t t; 
    t = clock(); 

	hr = FindAllUsersInGC();
	if (FAILED(hr))
	  wprintf(L"Search for all users failed with hr: %d\n", hr);

	t = clock() - t; 
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds   
    cout <<"\n\n\t\tTime Taken to read ALL OU:  "<<  time_taken << "seconds" << endl;

	CoUninitialize();
	cin.get();
	return 0;
}

 HRESULT FindAllUsersInGC()
{
    HRESULT hr = E_FAIL;
    HRESULT hrGC = E_FAIL;

	VARIANT var;
	ULONG lFetch;

    // Interface Pointers
    IDirectorySearch *pGCSearch = NULL;
	IADsContainer *pContainer = NULL;
    IUnknown *pUnk = NULL;
	IEnumVARIANT *pEnum = NULL;
   	IDispatch *pDisp = NULL;
    IADs *pADs = NULL;
	IADs    *pRoot=NULL;
	VARIANT varDSRoot;

	
	hr = ADsGetObject(L"LDAP://RootDSE",IID_IADs,(void**)&pRoot);
	hr = pRoot->Get(L"rootDomainNamingContext",&varDSRoot);
	if(SUCCEEDED(hr))
		printf("\nRoot Domain Naming Context:%S\n",varDSRoot.bstrVal);
	VariantClear(&varDSRoot);		

	//Bind to Global Catalog
    hr = ADsOpenObject(L"GC:",  //NT 4.0, Win9.x client must include the servername, e.g GC://myServer
				 NULL,
				 NULL,
				 ADS_SECURE_AUTHENTICATION, //Use Secure Authentication
				 IID_IADsContainer,
				 (void**)&pContainer);
	
	if (SUCCEEDED(hr))
	{
       hr = pContainer->get__NewEnum( &pUnk );
  	   if (SUCCEEDED(hr))
	   {
		  hr = pUnk->QueryInterface( IID_IEnumVARIANT, (void**) &pEnum );
		  if (SUCCEEDED(hr))
		  {
			// Now Enumerate--there should be only one item.
			hr = pEnum->Next( 1, &var, &lFetch );
			if (SUCCEEDED(hr))
			{
				while( hr == S_OK )
				{
					if ( lFetch == 1 )
					{
						pDisp = V_DISPATCH(&var);
						hr = pDisp->QueryInterface( IID_IDirectorySearch, (void**)&pGCSearch); 
						hrGC = hr;
					}
					VariantClear(&var);
					hr = pEnum->Next( 1, &var, &lFetch );
				};
			}
		  }
	      if (pEnum)
	        pEnum->Release();
	   }
       if (pUnk)
	     pUnk->Release();
 	}
	if (pContainer)
	  pContainer->Release();


	if (FAILED(hrGC))
	{
		if (pGCSearch)
			pGCSearch->Release();
		return hrGC;
	}

	//Create search filter
	//(&(objectCategory=computer)(|(name=leased*)(name=corp*)))
	LPOLESTR pszSearchFilter = L"(&(objectCategory=organizationalunit)(objectClass=organizationalunit))";
    //Search entire subtree from root.
	ADS_SEARCHPREF_INFO SearchPrefs;
	SearchPrefs.dwSearchPref = ADS_SEARCHPREF_SEARCH_SCOPE;
	SearchPrefs.vValue.dwType = ADSTYPE_INTEGER;
	SearchPrefs.vValue.Integer = ADS_SCOPE_SUBTREE;
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

    // Call IDirectorySearch::GetNextRow() to retrieve the next row 
    //of data
        while( pGCSearch->GetNextRow( hSearch) != S_ADS_NOMORE_ROWS )
		{
			num++;

            // loop through the array of passed column names,
            // print the data for each column
            for (DWORD x = 0; x < dwAttrNameSize; x++)
            {

			    // Get the data for this column
                hr = pGCSearch->GetColumn( hSearch, pszAttribute[x], &col );

			    if ( SUCCEEDED(hr) )
			    {
				    // Print the data for the column and free the column
					// Note the attributes we asked for are type CaseIgnoreString.
                   wprintf(L"%s: %s\r\n",pszAttribute[x],col.pADsValues->CaseIgnoreString); 
				    pGCSearch->FreeColumn( &col );
			    }
			    else
				{
					  wprintf(L"<%s property is not a string>",pszAttribute[x]);
				}
				 
            }
		   wprintf(L"------------------------------\n");
		   if(num == 50 )
			   break;
		}

		// Close the search handle to clean up
        pGCSearch->CloseSearchHandle(hSearch);
	} 
	if (pGCSearch)
		pGCSearch->Release();
    return hr;
}
