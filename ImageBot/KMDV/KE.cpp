#include "include.h"
#include "KE.h"

extern "C"
{
    NTKERNELAPI
        NTSTATUS
        ObReferenceObjectByName(
            IN PUNICODE_STRING ObjectName,
            IN ULONG Attributes,
            IN PACCESS_STATE PassedAccessState OPTIONAL,
            IN ACCESS_MASK DesiredAccess OPTIONAL,
            IN POBJECT_TYPE ObjectType,
            IN KPROCESSOR_MODE AccessMode,
            IN OUT PVOID ParseContext OPTIONAL,
            OUT PVOID* Object
        );
    extern POBJECT_TYPE* IoDeviceObjectType;
    extern POBJECT_TYPE* IoDriverObjectType;
}

bool KE::GetKernelObject(const WCHAR* DriverName, const WCHAR* DeviceName, PDEVICE_OBJECT* DeviceObject, PDRIVER_OBJECT *DriverObject) {
    PDRIVER_OBJECT  DriverObj   = nullptr;
    PDEVICE_OBJECT  DeviceObj   = nullptr;
    PFILE_OBJECT    FileObj     = nullptr;

    NTSTATUS        NtRet = STATUS_SUCCESS;
    UNICODE_STRING  DriverNameW;
    UNICODE_STRING  DeviceNameW;
   
    RtlInitUnicodeString(&DriverNameW, DriverName);
    RtlInitUnicodeString(&DeviceNameW, DeviceName);

    NtRet = IoGetDeviceObjectPointer(
        &DeviceNameW,
        FILE_ALL_ACCESS,
        &FileObj,
        &DeviceObj
    );

    if (!NT_SUCCESS(NtRet)) {
        DbgPrint("[KMDV] GetKernelObject: IoGetDeviceObjectPointer %ls failed: %x", DeviceName, NtRet);
        return false;
    }

    if (DeviceObject) {
        *DeviceObject = DeviceObj;
    }

    NtRet = ObReferenceObjectByName(
        &DriverNameW,
        OBJ_CASE_INSENSITIVE,
        NULL, GENERIC_ALL,
        *IoDriverObjectType,
        KernelMode,
        NULL,
        reinterpret_cast<PVOID*>(&DriverObj)
    );


    if (!NT_SUCCESS(NtRet)) {
        DbgPrint("[KMDV] GetKernelObject: ObReferenceObjectByName %ls failed: %x", DriverName, NtRet);
        ObDereferenceObject(&FileObj);
        return false;
    }

    if (DriverObject) {
        *DriverObject = DriverObj;
    }
    return true;
}

bool KE::GetKernelObject(const WCHAR* DriverName, PDEVICE_OBJECT* DeviceObject, PDRIVER_OBJECT* DriverObject)
{
    PDRIVER_OBJECT  DriverObj       = nullptr;
    PDEVICE_OBJECT  CurDeviceObj    = nullptr;

    NTSTATUS        NtRet = STATUS_SUCCESS;
    UNICODE_STRING  DriverNameW;

    RtlInitUnicodeString(&DriverNameW, DriverName);
    NtRet = ObReferenceObjectByName(
        &DriverNameW,
        OBJ_CASE_INSENSITIVE,
        NULL, 0,
        *IoDriverObjectType,
        KernelMode, 
        NULL,
        reinterpret_cast<PVOID*>(&DriverObj)
    );

    if (!NT_SUCCESS(NtRet)) {
        DbgPrint("[KMDV] GetKernelObject: ObReferenceObjectByName %ls failed: %x", DriverName, NtRet);
        return false;
    }

    if (DriverObject) {
        *DriverObject = DriverObj;
    }

    CurDeviceObj = DriverObj->DeviceObject;

    if (!CurDeviceObj) {
        DbgPrint("[KMDV] DeviceObject is null");
        ObDereferenceObject(DriverObj);
        return false;

    }
    
    while (CurDeviceObj) {
        if (!CurDeviceObj->NextDevice)
        {
            if (DeviceObject) {
                *DeviceObject = CurDeviceObj;
            }

            DbgPrint("[KMDV] %ls class device = %p .\n", DriverName, CurDeviceObj);
            break;
        }
        CurDeviceObj = CurDeviceObj->NextDevice;
    }

    ObDereferenceObject(DriverObj);

    return true;
}
