#pragma once
#include <ntddk.h>
#include <windef.h>
#include <stdlib.h>
#include <string.h>

#include <ntddmou.h>
#include <ntddkbd.h>

#define	DEVICE_NAME			L"\\Device\\KMDV"
#define LINK_GLOBAL_NAME	L"\\DosDevices\\Global\\KMDV"

#define IOCTL_IO_MOUSE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IO_KEYBD		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#include "PE.h"
#include "KE.h"

#include "KMRoutines.h"
#include "Scanner.h"