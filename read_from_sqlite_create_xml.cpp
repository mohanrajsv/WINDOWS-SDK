#include <iostream>
#include <tchar.h>
#include <objbase.h>  
#include <msxml6.h>
#include "sqlite3.h";
#include <string>
using namespace std;

#pragma comment(lib, "sqlite3.lib")
using namespace std;
// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)
// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)
// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)
// Helper function to create a VT_BSTR variant from a null terminated string. 
void processNode(IXMLDOMNode *);
static int callback(void* NotUsed, int argc, char** argv, char** azColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}
HRESULT VariantFromString(PCWSTR wszValue, VARIANT& Variant)
{
    HRESULT hr = S_OK;
    BSTR bstr = SysAllocString(wszValue);
    CHK_ALLOC(bstr);
    V_VT(&Variant) = VT_BSTR;
    V_BSTR(&Variant) = bstr;

CleanUp:
    return hr;
}

// Helper function to create a DOM instance. 
HRESULT CreateAndInitDOM(IXMLDOMDocument** ppDoc)
{
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
   
    return hr;
}

/*
void processNode(IXMLDOMNode* pNode)
{
    
    BSTR name = NULL;
    BSTR root = (BSTR)"student";
    BSTR text;
    HRESULT hRes = pNode->get_nodeName(&name);
    if (SUCCEEDED(hRes))
    {
        printf("%S : ", name);
        
    }
    if (name == root)
    {
        pNode->get_text(&text);
        printf(" %S\n", text);

    }
    SysFreeString(name);

    IXMLDOMNode* pChild = NULL;
    hRes = pNode->get_firstChild(&pChild);
    if (hRes == S_OK)
    {
        do
        {
            DOMNodeType type;
            hRes = pChild->get_nodeType(&type);
            if (SUCCEEDED(hRes) && type == NODE_ELEMENT)
            {
                processNode(pChild);              
            }             

                IXMLDOMNode * pSibling = NULL;
                hRes = pChild->get_nextSibling(&pSibling);
                if (hRes != S_OK) break;

                pChild->Release();
                pChild = pSibling;
              
               
        } while (true);

                pChild->Release();
    }
}
*/

void read()
{
    int round = 0;
    sqlite3* db;
    sqlite3* db2;
    char* sql;
    char* createDB;
    string tsql;
    char* zErrMsg = 0;
    int rc;

    rc = sqlite3_open("ex3.db", &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }
    /*
    createDB = (char*)"CREATE TABLE student2("  \
        "id INT PRIMARY KEY     NOT NULL," \
        "name           TEXT    NOT NULL," \
        "department TEXT NOT NULL);";

    rc = sqlite3_exec(db, createDB, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else {
        fprintf(stdout, "Table created successfully\n");
    }*/

    HRESULT hr = S_OK;
    IXMLDOMDocument* pXMLDom = NULL;
    IXMLDOMParseError* pXMLErr = NULL;
    IXMLDOMElement* pIXMLDOMElement = NULL;
    IXMLDOMNodeList* pNodes = NULL;
    BSTR bstrXML = NULL;
    BSTR bstrErr = NULL;
    VARIANT_BOOL varStatus;
    VARIANT varFileName;
    VariantInit(&varFileName);
    CHK_HR(CreateAndInitDOM(&pXMLDom));

    pXMLDom->put_async(VARIANT_FALSE);
    pXMLDom->put_validateOnParse(VARIANT_TRUE);
    pXMLDom->put_resolveExternals(VARIANT_FALSE);
    pXMLDom->put_preserveWhiteSpace(VARIANT_FALSE); // <--

    CHK_HR(VariantFromString(L"student.xml", varFileName));
    CHK_HR(pXMLDom->load(varFileName, &varStatus));

    if (varStatus == VARIANT_TRUE)
    {
        BSTR bstrQuery = NULL;
        IXMLDOMNode* pNode = NULL;
        IXMLDOMNode* pNodeD = NULL;
        BSTR bstrNodeName = NULL; //to store nodename
        BSTR bstrNodeValue = NULL; //to store node value
        BSTR text;
        HRESULT hr;
        string d_id, d_name, d_depart ;
        bstrQuery = SysAllocString(L"//student/*");
        CHK_ALLOC(bstrQuery);
        CHK_HR(pXMLDom->selectNodes(bstrQuery, &pNodes));
        if (pNodes)
        {
            long length, len2;
            CHK_HR(pNodes->get_length(&length));
            for (long i = 0; i < length; i++)
            {
                round++;
                CHK_HR(pNodes->get_item(i, &pNode));
                CHK_HR(pNode->get_nodeName(&bstrNodeName));
                
                if (0 == wcscmp(bstrNodeName, L"id"))
                {
                    HRESULT s = pNode->get_text(&text);
                    wstring t_id(text);
                     string temp(t_id.begin(), t_id.end());
                     d_id = temp;                  
                }
                if (0 == wcscmp(bstrNodeName, L"name"))
                {
                    HRESULT s = pNode->get_text(&text);
                    wstring t_name(text);
                    string temp(t_name.begin(), t_name.end());
                    d_name = temp;
                }
                if (0 == wcscmp(bstrNodeName, L"Department"))
                {
                    HRESULT s = pNode->get_text(&text);
                    wstring t_dp(text);
                    string temp(t_dp.begin(), t_dp.end());
                    d_depart = temp;
                }

                if (round == 3) {
                    tsql = "INSERT INTO student2 (id,name,department) VALUES ('" + d_id + "','" + d_name +
                        "', '" + d_depart + "')";
                    //cout << tsql;
                    const char* sql = tsql.c_str();

                    rc = sqlite3_exec(db, sql, callback, NULL, &zErrMsg);
                    if (rc != SQLITE_OK)
                    {
                        fprintf(stderr, "SQL error: %s\n", zErrMsg);
                        sqlite3_free(zErrMsg);
                    }
                    else {
                        //fprintf(stdout, "Operation done successfully\n");
                    }                                   
                    round = 0;
                }
                SysFreeString(bstrNodeName);                
                SAFE_RELEASE(pNode);
            }          
        }
     fprintf(stdout, "Operation done successfully\n");
    }
    else
    {
        // Failed to load xml, get last parsing error
        CHK_HR(pXMLDom->get_parseError(&pXMLErr));
        CHK_HR(pXMLErr->get_reason(&bstrErr));
        printf("Failed to load DOM from stocks.xml. %S\n", bstrErr);
    }
    sqlite3_close(db);
CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pXMLErr);
    SysFreeString(bstrXML);
    SysFreeString(bstrErr);
    VariantClear(&varFileName);
}

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = CoInitialize(NULL);
    read();
    CoUninitialize();
    return 0;
}
