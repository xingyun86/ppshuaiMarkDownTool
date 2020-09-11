#pragma once
#include <exdispid.h>
#include <mshtmhst.h>

__inline static
void CppCall(int uData)
{
	MessageBoxW(NULL, L"您调用了CppCall", L"提示(C++)", MB_OK);
}
//定义一个对象，并且实现IDispatch接口：
class ClientInvoke :public IDispatch
{
	ULONG uRefCount;
public:
	ClientInvoke():uRefCount(1)
	{
	}
	~ClientInvoke(void)
	{
	}
public:
	// IUnknown Methods
	STDMETHODIMP QueryInterface(REFIID iid, void** ppvObject)
	{
		if (iid == IID_IUnknown) {
			*ppvObject = this;
		}
		else if (iid == IID_IDispatch) {
			*ppvObject = (IDispatch*)this;
		}
		else {
			*ppvObject = NULL;
		}
		if (*ppvObject)
		{
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		return ::InterlockedIncrement(&uRefCount);
	}

	STDMETHODIMP_(ULONG) Release()
	{
		::InterlockedDecrement(&uRefCount);
		if (uRefCount != 0) {
			return uRefCount;
		}
		else {
			delete this;
			return 0;
		}
	}

	// IDispatch Methods

	HRESULT _stdcall GetTypeInfoCount(
		unsigned int* pctinfo)
	{
		return E_NOTIMPL;
	}

	HRESULT _stdcall GetTypeInfo(
		unsigned int iTInfo,
		LCID lcid,
		ITypeInfo FAR* FAR* ppTInfo)
	{
		return E_NOTIMPL;
	}

	HRESULT _stdcall GetIDsOfNames(
		REFIID riid,
		OLECHAR FAR* FAR* rgszNames,
		unsigned int cNames,
		LCID lcid,
		DISPID FAR* rgDispId
	)
	{
		if (lstrcmpW(rgszNames[0], L"CppCall") == 0)
		{
			//网页调用window.external.CppCall时，会调用这个方法获取CppCall的ID
			*rgDispId = 100;
		}
		return S_OK;
	}

	HRESULT _stdcall Invoke(
		DISPID dispIdMember,
		REFIID riid,
		LCID lcid,
		WORD wFlags,
		DISPPARAMS* pDispParams,
		VARIANT* pVarResult,
		EXCEPINFO* pExcepInfo,
		unsigned int* puArgErr
	)
	{
		switch (dispIdMember)
		{
		case DISPID_HTMLELEMENTEVENTS2_ONCLICK:
		{
			MessageBoxW(NULL, OLESTR("222"), OLESTR("111"), MB_OK);
		}
		break;
		default:
			break;
		}
		if (dispIdMember == 100)
		{
			//网页调用CppCall时，或根据获取到的ID调用Invoke方法
			CppCall(pDispParams->rgvarg[0].intVal);
		}
		return S_OK;
	}
};
class IDocHostUIHandlerImpl:public IDocHostUIHandler
{
public:
	IDocHostUIHandlerImpl(void) :pClientInvoke(new ClientInvoke()),uRefCount(0){
	}

	~IDocHostUIHandlerImpl(void) {
	}
	// IUnknown Method
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) {
		if (IsEqualIID(riid, IID_IUnknown))
		{
			*ppv = static_cast <IUnknown*> (this);
			return S_OK;
		}
		else if (IsEqualIID(riid, IID_IDocHostUIHandler))
		{
			*ppv = static_cast <IDocHostUIHandler*> (this);
			return S_OK;
		}
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
	}
	ULONG   __stdcall AddRef() {
		InterlockedIncrement(&uRefCount);

		return uRefCount;
	}
	ULONG   __stdcall Release() {
		InterlockedDecrement(&uRefCount);
		if (uRefCount != 0) {
			return uRefCount;
		}
		else {
			pClientInvoke->Release();
			delete this;
			return 0;
		}
	}

	// IDocHostUIHandler Method
	STDMETHOD(ShowContextMenu)(
		/* [in] */ DWORD dwID,
		/* [in] */ POINT __RPC_FAR *ppt,
		/* [in] */ IUnknown __RPC_FAR *pcmdtReserved,
		/* [in] */ IDispatch __RPC_FAR *pdispReserved) {
		return S_OK;
	}

	STDMETHOD(GetHostInfo)(
		/* [out][in] */ DOCHOSTUIINFO __RPC_FAR *pInfo) {
		pInfo->dwFlags = pInfo->dwFlags | DOCHOSTUIFLAG_NO3DBORDER;

		return S_OK;
	}

	STDMETHOD(ShowUI)(
		/* [in] */ DWORD dwID,
		/* [in] */ IOleInPlaceActiveObject __RPC_FAR *pActiveObject,
		/* [in] */ IOleCommandTarget __RPC_FAR *pCommandTarget,
		/* [in] */ IOleInPlaceFrame __RPC_FAR *pFrame,
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pDoc){ return E_NOTIMPL; }

	STDMETHOD(HideUI)( void){ return E_NOTIMPL; }

	STDMETHOD(UpdateUI)( void){ return E_NOTIMPL; }

	STDMETHOD(EnableModeless)(
		/* [in] */ BOOL fEnable){ return E_NOTIMPL; }

	STDMETHOD(OnDocWindowActivate)(
		/* [in] */ BOOL fActivate){ return E_NOTIMPL; }

	STDMETHOD(OnFrameWindowActivate)(
		/* [in] */ BOOL fActivate){ return E_NOTIMPL; }

	STDMETHOD(ResizeBorder)(
		/* [in] */ LPCRECT prcBorder,
		/* [in] */ IOleInPlaceUIWindow __RPC_FAR *pUIWindow,
		/* [in] */ BOOL fRameWindow){ return E_NOTIMPL; }

	STDMETHOD(TranslateAccelerator)(
		/* [in] */ LPMSG lpMsg,
		/* [in] */ const GUID __RPC_FAR *pguidCmdGroup,
		/* [in] */ DWORD nCmdID){ return E_NOTIMPL; }

	STDMETHOD(GetOptionKeyPath)(
		/* [out] */ LPOLESTR __RPC_FAR *pchKey,
		/* [in] */ DWORD dw){ return E_NOTIMPL; }

	STDMETHOD(GetDropTarget)(
		/* [in] */ IDropTarget __RPC_FAR *pDropTarget,
		/* [out] */ IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget){ return E_NOTIMPL; }

	STDMETHOD(GetExternal)(
		/* [out] */ IDispatch __RPC_FAR *__RPC_FAR *ppDispatch) {
		//重写GetExternal 返回一个对象
		(*ppDispatch) = pClientInvoke;
		return S_OK;
	}

	STDMETHOD(TranslateUrl)(
		/* [in] */ DWORD dwTranslate,
		/* [in] */ OLECHAR __RPC_FAR *pchURLIn,
		/* [out] */ OLECHAR __RPC_FAR *__RPC_FAR *ppchURLOut){ return E_NOTIMPL; }

	STDMETHOD(FilterDataObject)(
		/* [in] */ IDataObject __RPC_FAR *pDO,
		/* [out] */ IDataObject __RPC_FAR *__RPC_FAR *ppDORet){ return E_NOTIMPL; }

private:
	ULONG uRefCount;
	
	ClientInvoke* pClientInvoke;
};
