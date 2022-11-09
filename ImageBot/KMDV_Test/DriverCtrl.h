#pragma once
#include "include.h"
#pragma comment(lib,"advapi32.lib")

class DriverCtrl
{
public:
	DriverCtrl()
	{
		mSysPath		= nullptr;
		mServiceName	= nullptr;
		mDisplayName	= nullptr;
		mSCManager		= nullptr;
		mService		= nullptr;
		mLastError		= 0;
		mDriverHandle	= INVALID_HANDLE_VALUE;
	}

	~DriverCtrl()
	{
		CloseServiceHandle(mService);
		CloseServiceHandle(mSCManager);
		CloseHandle(mDriverHandle);
	}

public:
	DWORD		mLastError;
	PCHAR		mSysPath;
	PCHAR		mServiceName;
	PCHAR		mDisplayName;
	HANDLE		mDriverHandle;
	SC_HANDLE	mSCManager;
	SC_HANDLE	mService;
public:
	BOOL Install(PCHAR pSysPath, PCHAR pServiceName, PCHAR pDisplayName);
	BOOL Start();
	BOOL Stop();
	BOOL Remove();
	BOOL Open(PCHAR pLinkName);
	BOOL IoControl(DWORD dwIoCode, PVOID InBuff, DWORD InBuffLen, PVOID OutBuff, DWORD OutBuffLen, DWORD* RealRetBytes);

private:
	BOOL GetSvcHandle(PCHAR pServiceName);
	DWORD CTL_CODE_GEN(DWORD lngFunction);
};