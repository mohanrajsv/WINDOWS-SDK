#include <stdio.h>
#include <tchar.h>
#include <msxml6.h>
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

HRESULT CreateAndInitDOM(IXMLDOMDocument** ppDoc)
{
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDoc));
    return hr;
}

HRESULT CreateElement(IXMLDOMDocument* pXMLDom, PCWSTR wszName, IXMLDOMElement** ppElement)
{
    HRESULT hr = S_OK;
   // *ppElement = NULL;
    BSTR bstrName = SysAllocString(wszName);
    CHK_ALLOC(bstrName);
    CHK_HR(pXMLDom->createElement(bstrName, ppElement));

CleanUp:
    SysFreeString(bstrName);
    return hr;
}

HRESULT AppendChildToParent(IXMLDOMNode* pChild, IXMLDOMNode* pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMNode* pChildOut = NULL;
    CHK_HR(pParent->appendChild(pChild, &pChildOut));

CleanUp:
    SAFE_RELEASE(pChildOut);
    return hr;
}

HRESULT CreateAndAddPINode(IXMLDOMDocument* pDom, PCWSTR wszTarget, PCWSTR wszData)
{
    HRESULT hr = S_OK;
    IXMLDOMProcessingInstruction* pPI = NULL;

    BSTR bstrTarget = SysAllocString(wszTarget);
    BSTR bstrData = SysAllocString(wszData);
    CHK_ALLOC(bstrTarget);
    CHK_ALLOC(bstrData);

    CHK_HR(pDom->createProcessingInstruction(bstrTarget, bstrData, &pPI));
    CHK_HR(AppendChildToParent(pPI, pDom));

CleanUp:
    SAFE_RELEASE(pPI);
    SysFreeString(bstrTarget);
    SysFreeString(bstrData);
    return hr;
}

HRESULT CreateAndAddTextNode(IXMLDOMDocument* pDom, PCWSTR wszText, IXMLDOMNode* pParent)
{
    HRESULT hr = S_OK;
    IXMLDOMText* pText = NULL;

    BSTR bstrText = SysAllocString(wszText);
    CHK_ALLOC(bstrText);

    CHK_HR(pDom->createTextNode(bstrText, &pText));
    CHK_HR(AppendChildToParent(pText, pParent));

CleanUp:
    SAFE_RELEASE(pText);
    SysFreeString(bstrText);
    return hr;
}

// Helper function to create and append an element node to a parent node, and pass the newly created
// element node to caller if it wants.
HRESULT CreateAndAddElementNode(IXMLDOMDocument* pDom, PCWSTR wszName, PCWSTR wszNewline, IXMLDOMNode* pParent, IXMLDOMElement** ppElement = NULL)
{
    HRESULT hr = S_OK;
    IXMLDOMElement* pElement = NULL;
    CHK_HR(CreateElement(pDom, wszName, &pElement));

    // Add NEWLINE+TAB for identation before this element.
    CHK_HR(CreateAndAddTextNode(pDom, wszNewline, pParent));
    // Append this element to parent.
    CHK_HR(AppendChildToParent(pElement, pParent));

CleanUp:
    if (ppElement)
        *ppElement = pElement;  // Caller is repsonsible to release this element.
    else
        SAFE_RELEASE(pElement); // Caller is not interested on this element, so release it.

    return hr;
}

void write()
{
    HRESULT hr = S_OK;
    IXMLDOMDocument* pXMLDom = NULL;
    IXMLDOMElement* pRoot = NULL;
    IXMLDOMElement* pNode = NULL;
    IXMLDOMElement* pSubNode = NULL;
    IXMLDOMDocumentFragment* pDF = NULL;
    BSTR bstrXML = NULL;
    VARIANT varFileName;
    VariantInit(&varFileName);

    CHK_HR(CreateAndInitDOM(&pXMLDom));
    // Create a processing instruction element.
    CHK_HR(CreateAndAddPINode(pXMLDom, L"xml", L"version='1.0'"));
    // Create the root element.
    CHK_HR(CreateElement(pXMLDom, L"student", &pRoot));

    ////Creating NODE 1 
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"id", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"1", pNode));
    SAFE_RELEASE(pNode);
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"name", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"Mohanraj", pNode));
    SAFE_RELEASE(pNode);
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"Department", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"IT", pNode));
    SAFE_RELEASE(pNode);
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"\n", pRoot));

    //creating NODE 2 
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"id", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"2", pNode));
    SAFE_RELEASE(pNode);   
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"name", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"SureshKumar", pNode));
    SAFE_RELEASE(pNode);
    CHK_HR(CreateAndAddElementNode(pXMLDom, L"Department", L"\n\t", pRoot, &pNode));
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"CSE", pNode));
    SAFE_RELEASE(pNode);

    // Add NEWLINE for identation before </root>.
    CHK_HR(CreateAndAddTextNode(pXMLDom, L"\n", pRoot));
    // add <root> to document
    CHK_HR(AppendChildToParent(pRoot, pXMLDom));

    CHK_HR(pXMLDom->get_xml(&bstrXML));
    printf("XML DATA :\n%S\n", bstrXML);
    CHK_HR(VariantFromString(L"student.xml", varFileName));
    CHK_HR(pXMLDom->save(varFileName));
    printf("File saved as student.xml\n");

CleanUp:
    SAFE_RELEASE(pXMLDom);
    SAFE_RELEASE(pRoot);
    SAFE_RELEASE(pNode);
    SAFE_RELEASE(pDF);
    SAFE_RELEASE(pSubNode);
    SysFreeString(bstrXML);
    VariantClear(&varFileName);
}

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        write();
        CoUninitialize();
    }
    return 0;
}
