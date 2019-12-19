#include <iostream>
#include <tchar.h>
#include <objbase.h>  
#include <msxml6.h>
#import <msxml6.dll>
using namespace std;
// Macro that calls a COM method returning HRESULT value.
#define CHK_HR(stmt)        do { hr=(stmt); if (FAILED(hr)) goto CleanUp; } while(0)
// Macro to verify memory allcation.
#define CHK_ALLOC(p)        do { if (!(p)) { hr = E_OUTOFMEMORY; goto CleanUp; } } while(0)
// Macro that releases a COM object if not NULL.
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)
// Helper function to create a VT_BSTR variant from a null terminated string. 

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

void read()
{
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
        bstrQuery = SysAllocString(L"//student[1]/roll/*");
        CHK_ALLOC(bstrQuery);     
        CHK_HR(pXMLDom->selectNodes(bstrQuery, &pNodes));
        if (pNodes)
        {
            long length, len2;
            CHK_HR(pNodes->get_length(&length));
            for (long i = 0; i < length; i++)
            {                
                CHK_HR(pNodes->get_item(i, &pNode));
                CHK_HR(pNode->get_nodeName(&bstrNodeName));
                printf("%S : ", bstrNodeName);                
                                             
                HRESULT s = pNode->get_text(&text);
                CHK_HR(pNode->get_xml(&bstrNodeValue));
                printf(" %S\n", text);                        
                SysFreeString(bstrNodeValue);
                SysFreeString(bstrNodeName);
                SAFE_RELEASE(pNode);               
            }
        }
    }
    else
    {    
        // Failed to load xml, get last parsing error
        CHK_HR(pXMLDom->get_parseError(&pXMLErr));
        CHK_HR(pXMLErr->get_reason(&bstrErr));
        printf("Failed to load DOM from stocks.xml. %S\n", bstrErr);
    }

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
