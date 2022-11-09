#include "include.h"
#include "DriverCtrl.h"



BOOL DriverCtrl::GetSvcHandle(PCHAR pServiceName)
{
	mServiceName = pServiceName;
	mSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == mSCManager)
	{
		mLastError = GetLastError();
		return FALSE;
	}
	mService = OpenServiceA(mSCManager, mServiceName, SERVICE_ALL_ACCESS);
	if (NULL == mService)
	{
		CloseServiceHandle(mSCManager);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL DriverCtrl::Install(PCHAR pSysPath, PCHAR pServiceName, PCHAR pDisplayName)
{
	mSysPath = pSysPath;
	mServiceName = pServiceName;
	mDisplayName = pDisplayName;
	mSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (NULL == mSCManager)
	{
		mLastError = GetLastError();
		return FALSE;
	}
	mService = CreateServiceA(mSCManager, mServiceName, mDisplayName,
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		mSysPath, NULL, NULL, NULL, NULL, NULL);
	if (NULL == mService)
	{
		mLastError = GetLastError();
		if (ERROR_SERVICE_EXISTS == mLastError)
		{
			mService = OpenServiceA(mSCManager, mServiceName, SERVICE_ALL_ACCESS);
			if (NULL == mService)
			{
				CloseServiceHandle(mSCManager);
				return FALSE;
			}
		}
		else
		{
			CloseServiceHandle(mSCManager);
			return FALSE;
		}
	}
	return TRUE;
	
}

BOOL DriverCtrl::Start()
{
	if (!StartServiceA(mService, NULL, NULL))
	{
		mLastError = GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL DriverCtrl::Stop()
{
	SERVICE_STATUS ss;
	GetSvcHandle(mServiceName);
	if (!ControlService(mService, SERVICE_CONTROL_STOP, &ss))
	{
		mLastError = GetLastError();
		return FALSE;
	}
	return TRUE;

}

BOOL DriverCtrl::Remove()
{
	GetSvcHandle(mServiceName);
	if (!DeleteService(mService))
	{
		mLastError = GetLastError();
		return FALSE;
	}
	return TRUE;
}

BOOL DriverCtrl::Open(PCHAR pLinkName)//example: \\\\.\\xxoo
{
	if (mDriverHandle != INVALID_HANDLE_VALUE)
		return TRUE;
	mDriverHandle = CreateFileA(pLinkName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (mDriverHandle != INVALID_HANDLE_VALUE)
		return TRUE;
	else
		return FALSE;
}

BOOL DriverCtrl::IoControl(DWORD dwIoCode, PVOID InBuff, DWORD InBuffLen, PVOID OutBuff, DWORD OutBuffLen, DWORD* RealRetBytes)
{
	DWORD dw;
	BOOL b = DeviceIoControl(mDriverHandle, dwIoCode, InBuff, InBuffLen, OutBuff, OutBuffLen, &dw, NULL);
	if (RealRetBytes)
		*RealRetBytes = dw;
	return b;
}

DWORD DriverCtrl::CTL_CODE_GEN(DWORD lngFunction)
{
	return (FILE_DEVICE_UNKNOWN * 65536) | (FILE_ANY_ACCESS * 16384) | (lngFunction * 4) | METHOD_BUFFERED;
}