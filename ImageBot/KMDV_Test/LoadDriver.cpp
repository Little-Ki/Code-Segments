#include "LoadDriver.h"

LSTATUS RequirePrivilege(LPCTSTR lpPrivilege) {
	HANDLE				TokenHandle;
	BOOL				Error = FALSE;
	TOKEN_PRIVILEGES	TokenPriv;
	LUID				Luid;

	Error = LookupPrivilegeValue(NULL, lpPrivilege, &Luid); // lookup LUID for privilege on local system
	if (!Error) {
		return -1;
	}

	Error = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &TokenHandle);
	if (!Error) {
		return -2;
	}

	if (ANYSIZE_ARRAY != 1) {
		return -3;
	}
	TokenPriv.PrivilegeCount = 1; // only adjust one privilege
	TokenPriv.Privileges[0].Luid = Luid;
	TokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	Error = AdjustTokenPrivileges(TokenHandle, FALSE, &TokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL);

	if (Error != TRUE || GetLastError() != ERROR_SUCCESS) {
		return -4;
	}

	CloseHandle(TokenHandle);
	return 0;
}

fs::path CopyDriverFile(fs::path sourcePath) {
	auto out = fs::path(R"(C:\Windows\Temp)") / sourcePath.filename();
	std::ifstream  src(sourcePath, std::ios::binary);
	std::ofstream  dst(out, std::ios::binary);
	dst << src.rdbuf();
	return out;
}


LSTATUS NtLoadDriver(const std::string& service_name, const fs::path driver_path) {
	USES_CONVERSION;

	f_NtLoadDriver	NtLoadDriverProc;
	HMODULE			NTDllHandle;
	std::string		DevPathA;
	LSTATUS			Status{ 0 };
	std::string		ServiceKey;
	std::string		DriverKey;
	HKEY			KeyHandle;
	DWORD			DummyDword{ 1 };
	std::string		RelativePath;

	ANSI_STRING		DriverKeyA;
	UNICODE_STRING	DriverKeyU;

	auto DevPath	= CopyDriverFile(driver_path);

	NTDllHandle		= GetModuleHandleA("ntdll.dll");
	
	if (!NTDllHandle) {
		goto CLEAN;
	}

	NtLoadDriverProc = ReCa<f_NtLoadDriver>(GetProcAddress(NTDllHandle, "NtLoadDriver"));

	if (!NtLoadDriverProc) {
		return -1;
	}

	Status = RequirePrivilege(SE_LOAD_DRIVER_NAME);
	if (Status) {
		goto CLEAN;
	}

	ServiceKey = std::format("System\\CurrentControlSet\\Services\\{}", service_name);
	Status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, ServiceKey.c_str(), 0, KEY_READ, &KeyHandle);
	if (Status == ERROR_SUCCESS) {
		return -1;
	}

	Status = RegCreateKeyA(HKEY_LOCAL_MACHINE, ServiceKey.c_str(), &KeyHandle);
	if (Status) {
		goto CLEAN;
	}

	Status = RegSetValueExA(KeyHandle, "Type", 0, REG_DWORD, (BYTE*)&DummyDword, sizeof(DWORD));	
	if (Status) {
		goto CLEAN;
	}


	Status = RegSetValueExA(KeyHandle, "ErrorControl", 0, REG_DWORD, (BYTE*)&DummyDword, sizeof(DWORD));
	if (Status) {
		goto CLEAN;
	}

	DummyDword = 3;
	Status = RegSetValueExA(KeyHandle, "Start", 0, REG_DWORD, (BYTE*)&DummyDword, sizeof(DWORD));
	if (Status) {
		goto CLEAN;
	}

	DevPathA = W2A(DevPath.c_str());
	RelativePath = DevPathA.substr(sizeof("C:\\Windows\\") - 1);
	Status = RegSetValueExA(KeyHandle, "ImagePath", 0, REG_SZ, ReCa<const BYTE*>(RelativePath.c_str()), RelativePath.size() + 1);
	if (Status) {
		goto CLEAN;
	}

	DriverKey = std::format("\\Registry\\Machine\\System\\CurrentControlSet\\Services\\{}", service_name);

	RtlInitAnsiString(&DriverKeyA, DriverKey.c_str());
	Status = RtlAnsiStringToUnicodeString(&DriverKeyU, &DriverKeyA, true);
	if (Status) {
		goto CLEAN;
	}

CLEAN:
	if (DriverKeyU.Length) {
		RtlFreeUnicodeString(&DriverKeyU);
	}

	if (!ServiceKey.empty()) {
		SHDeleteKeyA(KeyHandle, ServiceKey.c_str());
	}

	if (fs::is_regular_file(DevPath)) {
		try {
			fs::remove(DevPath);
		}
		catch (std::exception& e) {
		}
	}

	return Status;
}