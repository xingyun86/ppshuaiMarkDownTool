#pragma once

#include <comdef.h>
#include <ExDisp.h>
#include <ExDispID.h>
#include <Mshtml.h>
#include <mshtmdid.h>

class WebeventsInterface
{
protected:
	WebeventsInterface(void){};
	virtual ~WebeventsInterface(void){};
public:
	virtual STDMETHODIMP OnBeforeNavigate2(
		IDispatch *pDisp, VARIANT *pvUrl, VARIANT *pvFlags, VARIANT *pvTargetFrameName,
		VARIANT *pvPostData, VARIANT *pvHeaders, VARIANT_BOOL *pvCancel) = 0;

	virtual STDMETHODIMP OnDocumentComplete(IDispatch *pDisp, BSTR bstrUrl) = 0 ;

	virtual STDMETHODIMP OnStatusTextChange(IDispatch *pDisp, BSTR bstrText) = 0 ;

	virtual STDMETHODIMP OnClick() = 0;
};

// DWebBrowserEvents2
class WebbrowserSink: public DWebBrowserEvents2
{
public:
	WebbrowserSink(WebeventsInterface* pEventCallBack):	m_pEventCallBack(pEventCallBack), uRefCount(1)
	{
	}
	virtual ~WebbrowserSink() {
	}

protected:
	ULONG uRefCount;
private:
	WebeventsInterface* m_pEventCallBack;
public:

	STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject) {
		if (IsBadWritePtr(ppvObject, sizeof(void*)))
		{
			return E_POINTER;
		}

		*ppvObject = NULL;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch) || IsEqualIID(riid, DIID_DWebBrowserEvents2))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	STDMETHODIMP_(ULONG) AddRef() {
		InterlockedIncrement(&uRefCount);
		return uRefCount;
	}
	STDMETHODIMP_(ULONG) Release() {
		InterlockedDecrement(&uRefCount);
		if (uRefCount != 0) {
			return uRefCount;
		}
		else {
			delete this;
			return 0;
		}
	}
	// IDispatch methods
	STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {
		UNREFERENCED_PARAMETER(pctinfo);
		*pctinfo = 0;
		return S_OK;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) {
		UNREFERENCED_PARAMETER(iTInfo);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(ppTInfo);

		*ppTInfo = NULL;
		return E_NOTIMPL;
	}

	STDMETHODIMP GetIDsOfNames(REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) {
		UNREFERENCED_PARAMETER(riid);
		UNREFERENCED_PARAMETER(rgszNames);
		UNREFERENCED_PARAMETER(cNames);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(rgDispId);

		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) {
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(wFlags);
		UNREFERENCED_PARAMETER(pVarResult);
		UNREFERENCED_PARAMETER(pExcepInfo);
		UNREFERENCED_PARAMETER(puArgErr);

		if (!IsEqualIID(riid, IID_NULL))
		{
			return DISP_E_UNKNOWNINTERFACE; // riid should always be IID_NULL
		}

		//LogTrace(L"WebBrowserSink::Invoke dispIdMember = %d",dispIdMember);
		switch (dispIdMember)
		{
		case DISPID_BEFORENAVIGATE2:
			if (NULL != m_pEventCallBack)
			{
				m_pEventCallBack->OnBeforeNavigate2(
					(IDispatch*)pDispParams->rgvarg[6].byref,
					(VARIANT*)pDispParams->rgvarg[5].pvarVal,
					(VARIANT*)pDispParams->rgvarg[4].pvarVal,
					(VARIANT*)pDispParams->rgvarg[3].pvarVal,
					(VARIANT*)pDispParams->rgvarg[2].pvarVal,
					(VARIANT*)pDispParams->rgvarg[1].pvarVal,
					(VARIANT_BOOL*)pDispParams->rgvarg[0].pboolVal
				);
			}

			break;
		case DISPID_DOCUMENTCOMPLETE:
			if (NULL != m_pEventCallBack)
			{
				m_pEventCallBack->OnDocumentComplete(
					(IDispatch*)pDispParams->rgvarg[0].byref,
					pDispParams->rgvarg[0].pvarVal->bstrVal
				);
			}
			break;
		case DISPID_PROGRESSCHANGE:
			if (NULL != m_pEventCallBack)
			{

			}
			break;
		case DISPID_STATUSTEXTCHANGE:
			if (NULL != m_pEventCallBack)
			{
				m_pEventCallBack->OnStatusTextChange(
					(IDispatch*)pDispParams->rgvarg[0].byref,
					pDispParams->rgvarg[0].bstrVal
				);
			}
			break;
		default:
			break;
		}

		return S_OK;
	}

};

//  HTMLElementEvents2 
class HtmlElementSink: public HTMLElementEvents2 
{
public:
	HtmlElementSink(WebeventsInterface* pEventCallBack):m_pEventCallBack(pEventCallBack){
		m_dwRef = 1;
	}

	virtual ~HtmlElementSink(void) {
	}

protected:
	LONG m_dwRef;
private:
	WebeventsInterface* m_pEventCallBack;
public:
	STDMETHODIMP QueryInterface(REFIID riid,void **ppvObject) {
		if (IsBadWritePtr(ppvObject, sizeof(void*)))
		{
			return E_POINTER;
		}
		*ppvObject = NULL;

		if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDispatch) || IsEqualIID(riid, DIID_DWebBrowserEvents2))
		{
			AddRef();
			*ppvObject = this;
			return S_OK;
		}
		else
		{
			return E_NOINTERFACE;
		}
	}
	STDMETHODIMP_(ULONG) AddRef() {
		InterlockedIncrement(&m_dwRef);
		return m_dwRef;
	}
	STDMETHODIMP_(ULONG) Release() {
		InterlockedDecrement(&m_dwRef);
		if (m_dwRef == 0) {
			return m_dwRef;
		}
		else {
			delete this;
			return 0;
		}
	}
	// IDispatch methods
	STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) {
		UNREFERENCED_PARAMETER(pctinfo);
		*pctinfo = 0;
		return S_OK;
	}
	STDMETHODIMP GetTypeInfo(UINT iTInfo,LCID lcid,ITypeInfo **ppTInfo) {
		UNREFERENCED_PARAMETER(iTInfo);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(ppTInfo);

		*ppTInfo = NULL;
		return E_NOTIMPL;
	}
	STDMETHODIMP GetIDsOfNames(REFIID riid,LPOLESTR *rgszNames,UINT cNames,LCID lcid,DISPID *rgDispId) {
		UNREFERENCED_PARAMETER(riid);
		UNREFERENCED_PARAMETER(rgszNames);
		UNREFERENCED_PARAMETER(cNames);
		UNREFERENCED_PARAMETER(lcid);
		UNREFERENCED_PARAMETER(rgDispId);

		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispIdMember,REFIID riid,LCID lcid,WORD wFlags,DISPPARAMS *pDispParams,VARIANT *pVarResult,EXCEPINFO *pExcepInfo,UINT *puArgErr) {
		switch (dispIdMember)
		{
		case DISPID_HTMLELEMENTEVENTS2_ONCLICK:
			if (NULL != m_pEventCallBack)
			{
				m_pEventCallBack->OnClick();
			}
			break;

		default:
			break;
		}

		return S_OK;
	}
};