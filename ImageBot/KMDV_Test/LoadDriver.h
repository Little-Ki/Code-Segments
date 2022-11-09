#pragma once
#include <Windows.h>
#include <Winternl.h>
#include <filesystem>
#include <string>
#include <atlbase.h>
#include <iostream>
#include <fstream>

#pragma comment(lib, "ntdll.lib")

#define ReCa reinterpret_cast

namespace fs = std::filesystem;

using f_NtLoadDriver = NTSTATUS(__stdcall*)(UNICODE_STRING* DriverServiceName);

LSTATUS NtLoadDriver(const std::string& service_name, const fs::path driver_path);