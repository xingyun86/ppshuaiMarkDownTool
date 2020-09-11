// Operator.h : Include file for standard system include files,
// or project specific include files.

#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#include <windows.h>
#include <netfw.h>
#include <objbase.h>
#include <oleauto.h>
//#pragma comment(lib,"ole32.lib")
//#pragma comment(lib,"oleaut32.lib")
class WindowFirewall {
private:
    HRESULT hComInitStatus = E_FAIL;
    INetFwProfile* fwProfile = NULL;
    void WfStartup()
    {
        HRESULT hResult = E_FAIL;

        // Initialize COM.
        hComInitStatus = hResult = CoInitializeEx(
            0,
            COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
        );

        // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
        // initialized with a different mode. Since we don't care what the mode is,
        // we'll just use the existing mode.
        if (hResult != RPC_E_CHANGED_MODE)
        {
            if (FAILED(hResult))
            {
                printf("CoInitializeEx failed: 0x%08lx\n", hResult);
                WfCleanup();
                return;
            }
        }

        // Retrieve the firewall profile currently in effect.
        hResult = WindowsFirewallInitialize();
        if (FAILED(hResult))
        {
            printf("WindowsFirewallInitialize failed: 0x%08lx\n", hResult);
            WfCleanup();
            return;
        }
    }
    void WfCleanup()
    {
        // Release the firewall profile.
        WindowsFirewallCleanup();

        // Uninitialize COM.
        if (SUCCEEDED(hComInitStatus))
        {
            CoUninitialize();
        }
    }

    HRESULT WindowsFirewallInitialize()
    {
        HRESULT hr = S_OK;
        INetFwMgr* fwMgr = NULL;
        INetFwPolicy* fwPolicy = NULL;

        fwProfile = NULL;

        // Create an instance of the firewall settings manager.
        hr = CoCreateInstance(
            __uuidof(NetFwMgr),
            NULL,
            CLSCTX_INPROC_SERVER,
            __uuidof(INetFwMgr),
            (void**)&fwMgr
        );
        if (FAILED(hr))
        {
            printf("CoCreateInstance failed: 0x%08lx\n", hr);
            goto error;
        }

        // Retrieve the local firewall policy.
        hr = fwMgr->get_LocalPolicy(&fwPolicy);
        if (FAILED(hr))
        {
            printf("get_LocalPolicy failed: 0x%08lx\n", hr);
            goto error;
        }

        // Retrieve the firewall profile currently in effect.
        hr = fwPolicy->get_CurrentProfile(&fwProfile);
        if (FAILED(hr))
        {
            printf("get_CurrentProfile failed: 0x%08lx\n", hr);
            goto error;
        }

    error:

        // Release the local firewall policy.
        if (fwPolicy != NULL)
        {
            fwPolicy->Release();
        }

        // Release the firewall settings manager.
        if (fwMgr != NULL)
        {
            fwMgr->Release();
        }

        return hr;
    }

    void WindowsFirewallCleanup()
    {
        ///////////////////////////////////////////////////////////
        // Not need release the firewall profile.Comment the codes
        //if (fwProfile != NULL)
        //{
        //    fwProfile->Release();
        //}
    }

public:
    WindowFirewall() {
        WfStartup();
    }
    ~WindowFirewall() {
        WfCleanup();
    }
public:
    HRESULT WindowsFirewallIsOn(OUT BOOL* fwOn)
    {
        HRESULT hr = S_OK;
        VARIANT_BOOL fwEnabled;

        _ASSERT(fwProfile != NULL);
        _ASSERT(fwOn != NULL);

        *fwOn = FALSE;

        // Get the current state of the firewall.
        hr = fwProfile->get_FirewallEnabled(&fwEnabled);
        if (FAILED(hr))
        {
            printf("get_FirewallEnabled failed: 0x%08lx\n", hr);
            goto error;
        }

        // Check to see if the firewall is on.
        if (fwEnabled != VARIANT_FALSE)
        {
            *fwOn = TRUE;
            printf("The firewall is on.\n");
        }
        else
        {
            printf("The firewall is off.\n");
        }

    error:

        return hr;
    }
    HRESULT WindowsFirewallTurnOn()
    {
        HRESULT hr = S_OK;
        BOOL fwOn;

        _ASSERT(fwProfile != NULL);

        // Check to see if the firewall is off.
        hr = WindowsFirewallIsOn(&fwOn);
        if (FAILED(hr))
        {
            printf("WindowsFirewallIsOn failed: 0x%08lx\n", hr);
            goto error;
        }

        // If it is, turn it on.
        if (!fwOn)
        {
            // Turn the firewall on.
            hr = fwProfile->put_FirewallEnabled(VARIANT_TRUE);
            if (FAILED(hr))
            {
                printf("put_FirewallEnabled failed: 0x%08lx\n", hr);
                goto error;
            }

            printf("The firewall is now on.\n");
        }

    error:

        return hr;
    }
    HRESULT WindowsFirewallTurnOff()
    {
        HRESULT hr = S_OK;
        BOOL fwOn;

        _ASSERT(fwProfile != NULL);

        // Check to see if the firewall is on.
        hr = WindowsFirewallIsOn(&fwOn);
        if (FAILED(hr))
        {
            printf("WindowsFirewallIsOn failed: 0x%08lx\n", hr);
            goto error;
        }

        // If it is, turn it off.
        if (fwOn)
        {
            // Turn the firewall off.
            hr = fwProfile->put_FirewallEnabled(VARIANT_FALSE);
            if (FAILED(hr))
            {
                printf("put_FirewallEnabled failed: 0x%08lx\n", hr);
                goto error;
            }

            printf("The firewall is now off.\n");
        }

    error:

        return hr;
    }
    HRESULT WindowsFirewallAppIsEnabled(
        IN const LPOLESTR fwProcessImageFileName,
        OUT BOOL* fwAppEnabled
    )
    {
        HRESULT hr = S_OK;
        BSTR fwBstrProcessImageFileName = NULL;
        VARIANT_BOOL fwEnabled;
        INetFwAuthorizedApplication* fwApp = NULL;
        INetFwAuthorizedApplications* fwApps = NULL;

        _ASSERT(fwProfile != NULL);
        _ASSERT(fwProcessImageFileName != NULL);
        _ASSERT(fwAppEnabled != NULL);

        *fwAppEnabled = FALSE;

        // Retrieve the authorized application collection.
        hr = fwProfile->get_AuthorizedApplications(&fwApps);
        if (FAILED(hr))
        {
            printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
            goto error;
        }

        // Allocate a BSTR for the process image file name.
        fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
        if (fwBstrProcessImageFileName == NULL)
        {
            hr = E_OUTOFMEMORY;
            printf("SysAllocString failed: 0x%08lx\n", hr);
            goto error;
        }

        // Attempt to retrieve the authorized application.
        hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
        if (SUCCEEDED(hr))
        {
            // Find out if the authorized application is enabled.
            hr = fwApp->get_Enabled(&fwEnabled);
            if (FAILED(hr))
            {
                printf("get_Enabled failed: 0x%08lx\n", hr);
                goto error;
            }

            if (fwEnabled != VARIANT_FALSE)
            {
                // The authorized application is enabled.
                *fwAppEnabled = TRUE;

                printf(
                    "Authorized application %lS is enabled in the firewall.\n",
                    fwProcessImageFileName
                );
            }
            else
            {
                printf(
                    "Authorized application %lS is disabled in the firewall.\n",
                    fwProcessImageFileName
                );
            }
        }
        else
        {
            // The authorized application was not in the collection.
            hr = S_OK;

            printf(
                "Authorized application %lS is disabled in the firewall.\n",
                fwProcessImageFileName
            );
        }

    error:

        // Free the BSTR.
        SysFreeString(fwBstrProcessImageFileName);

        // Release the authorized application instance.
        if (fwApp != NULL)
        {
            fwApp->Release();
        }

        // Release the authorized application collection.
        if (fwApps != NULL)
        {
            fwApps->Release();
        }

        return hr;
    }
    HRESULT WindowsFirewallAddApp(
        IN const LPOLESTR fwProcessImageFileName,
        IN const LPOLESTR fwName
    )
    {
        HRESULT hr = S_OK;
        BOOL fwAppEnabled;
        BSTR fwBstrName = NULL;
        BSTR fwBstrProcessImageFileName = NULL;
        INetFwAuthorizedApplication* fwApp = NULL;
        INetFwAuthorizedApplications* fwApps = NULL;

        _ASSERT(fwProfile != NULL);
        _ASSERT(fwProcessImageFileName != NULL);
        _ASSERT(fwName != NULL);

        // First check to see if the application is already authorized.
        hr = WindowsFirewallAppIsEnabled(
            fwProcessImageFileName,
            &fwAppEnabled
        );
        if (FAILED(hr))
        {
            printf("WindowsFirewallAppIsEnabled failed: 0x%08lx\n", hr);
            goto error;
        }

        // Only add the application if it isn't already authorized.
        if (!fwAppEnabled)
        {
            // Retrieve the authorized application collection.
            hr = fwProfile->get_AuthorizedApplications(&fwApps);
            if (FAILED(hr))
            {
                printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
                goto error;
            }

            // Create an instance of an authorized application.
            hr = CoCreateInstance(
                __uuidof(NetFwAuthorizedApplication),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(INetFwAuthorizedApplication),
                (void**)&fwApp
            );
            if (FAILED(hr))
            {
                printf("CoCreateInstance failed: 0x%08lx\n", hr);
                goto error;
            }

            // Allocate a BSTR for the process image file name.
            fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
            if (fwBstrProcessImageFileName == NULL)
            {
                hr = E_OUTOFMEMORY;
                printf("SysAllocString failed: 0x%08lx\n", hr);
                goto error;
            }

            // Set the process image file name.
            hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
            if (FAILED(hr))
            {
                printf("put_ProcessImageFileName failed: 0x%08lx\n", hr);
                goto error;
            }

            // Allocate a BSTR for the application friendly name.
            fwBstrName = SysAllocString(fwName);
            if (SysStringLen(fwBstrName) == 0)
            {
                hr = E_OUTOFMEMORY;
                printf("SysAllocString failed: 0x%08lx\n", hr);
                goto error;
            }

            // Set the application friendly name.
            hr = fwApp->put_Name(fwBstrName);
            if (FAILED(hr))
            {
                printf("put_Name failed: 0x%08lx\n", hr);
                goto error;
            }

            // Add the application to the collection.
            hr = fwApps->Add(fwApp);
            if (FAILED(hr))
            {
                printf("Add failed: 0x%08lx\n", hr);
                goto error;
            }

            printf(
                "Authorized application %lS is now enabled in the firewall.\n",
                fwProcessImageFileName
            );
        }

    error:

        // Free the BSTRs.
        SysFreeString(fwBstrName);
        SysFreeString(fwBstrProcessImageFileName);

        // Release the authorized application instance.
        if (fwApp != NULL)
        {
            fwApp->Release();
        }

        // Release the authorized application collection.
        if (fwApps != NULL)
        {
            fwApps->Release();
        }

        return hr;
    }
    HRESULT WindowsFirewallPortIsEnabled(
        IN LONG portNumber,
        IN NET_FW_IP_PROTOCOL ipProtocol,
        OUT BOOL* fwPortEnabled
    )
    {
        HRESULT hr = S_OK;
        VARIANT_BOOL fwEnabled;
        INetFwOpenPort* fwOpenPort = NULL;
        INetFwOpenPorts* fwOpenPorts = NULL;

        _ASSERT(fwProfile != NULL);
        _ASSERT(fwPortEnabled != NULL);

        *fwPortEnabled = FALSE;

        // Retrieve the globally open ports collection.
        hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
        if (FAILED(hr))
        {
            printf("get_GloballyOpenPorts failed: 0x%08lx\n", hr);
            goto error;
        }

        // Attempt to retrieve the globally open port.
        hr = fwOpenPorts->Item(portNumber, ipProtocol, &fwOpenPort);
        if (SUCCEEDED(hr))
        {
            // Find out if the globally open port is enabled.
            hr = fwOpenPort->get_Enabled(&fwEnabled);
            if (FAILED(hr))
            {
                printf("get_Enabled failed: 0x%08lx\n", hr);
                goto error;
            }

            if (fwEnabled != VARIANT_FALSE)
            {
                // The globally open port is enabled.
                *fwPortEnabled = TRUE;

                printf("Port %ld is open in the firewall.\n", portNumber);
            }
            else
            {
                printf("Port %ld is not open in the firewall.\n", portNumber);
            }
        }
        else
        {
            // The globally open port was not in the collection.
            hr = S_OK;

            printf("Port %ld is not open in the firewall.\n", portNumber);
        }

    error:

        // Release the globally open port.
        if (fwOpenPort != NULL)
        {
            fwOpenPort->Release();
        }

        // Release the globally open ports collection.
        if (fwOpenPorts != NULL)
        {
            fwOpenPorts->Release();
        }

        return hr;
    }


    HRESULT WindowsFirewallPortAdd(
        IN INetFwProfile* fwProfile,
        IN LONG portNumber,
        IN NET_FW_IP_PROTOCOL ipProtocol,
        IN const LPOLESTR name
    )
    {
        HRESULT hr = S_OK;
        BOOL fwPortEnabled;
        BSTR fwBstrName = NULL;
        INetFwOpenPort* fwOpenPort = NULL;
        INetFwOpenPorts* fwOpenPorts = NULL;

        _ASSERT(fwProfile != NULL);
        _ASSERT(name != NULL);

        // First check to see if the port is already added.
        hr = WindowsFirewallPortIsEnabled(
            portNumber,
            ipProtocol,
            &fwPortEnabled
        );
        if (FAILED(hr))
        {
            printf("WindowsFirewallPortIsEnabled failed: 0x%08lx\n", hr);
            goto error;
        }

        // Only add the port if it isn't already added.
        if (!fwPortEnabled)
        {
            // Retrieve the collection of globally open ports.
            hr = fwProfile->get_GloballyOpenPorts(&fwOpenPorts);
            if (FAILED(hr))
            {
                printf("get_GloballyOpenPorts failed: 0x%08lx\n", hr);
                goto error;
            }

            // Create an instance of an open port.
            hr = CoCreateInstance(
                __uuidof(NetFwOpenPort),
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(INetFwOpenPort),
                (void**)&fwOpenPort
            );
            if (FAILED(hr))
            {
                printf("CoCreateInstance failed: 0x%08lx\n", hr);
                goto error;
            }

            // Set the port number.
            hr = fwOpenPort->put_Port(portNumber);
            if (FAILED(hr))
            {
                printf("put_Port failed: 0x%08lx\n", hr);
                goto error;
            }

            // Set the IP protocol.
            hr = fwOpenPort->put_Protocol(ipProtocol);
            if (FAILED(hr))
            {
                printf("put_Protocol failed: 0x%08lx\n", hr);
                goto error;
            }

            // Allocate a BSTR for the friendly name of the port.
            fwBstrName = SysAllocString(name);
            if (SysStringLen(fwBstrName) == 0)
            {
                hr = E_OUTOFMEMORY;
                printf("SysAllocString failed: 0x%08lx\n", hr);
                goto error;
            }

            // Set the friendly name of the port.
            hr = fwOpenPort->put_Name(fwBstrName);
            if (FAILED(hr))
            {
                printf("put_Name failed: 0x%08lx\n", hr);
                goto error;
            }

            // Opens the port and adds it to the collection.
            hr = fwOpenPorts->Add(fwOpenPort);
            if (FAILED(hr))
            {
                printf("Add failed: 0x%08lx\n", hr);
                goto error;
            }

            printf("Port %ld is now open in the firewall.\n", portNumber);
        }

    error:

        // Free the BSTR.
        SysFreeString(fwBstrName);

        // Release the open port instance.
        if (fwOpenPort != NULL)
        {
            fwOpenPort->Release();
        }

        // Release the globally open ports collection.
        if (fwOpenPorts != NULL)
        {
            fwOpenPorts->Release();
        }

        return hr;
    }

public:
	static WindowFirewall* Inst()
	{
		static WindowFirewall windowFirewallInstance;
		return &windowFirewallInstance;
	}
};