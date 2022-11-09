#pragma once

using PPVOID = void**;
using KPRIORITY = LONG;

typedef struct _NON_PAGED_DEBUG_INFO
{
    USHORT      Signature;
    USHORT      Flags;
    ULONG       Size;
    USHORT      Machine;
    USHORT      Characteristics;
    ULONG       TimeDateStamp;
    ULONG       CheckSum;
    ULONG       SizeOfImage;
    ULONGLONG   ImageBase;
} NON_PAGED_DEBUG_INFO, * PNON_PAGED_DEBUG_INFO;

typedef struct _KLDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY              InLoadOrderLinks;
    PVOID                   ExceptionTable;
    ULONG                   ExceptionTableSize;
    PVOID                   GpValue;
    PNON_PAGED_DEBUG_INFO   NonPagedDebugInfo;
    PVOID                   DllBase;
    PVOID                   EntryPoint;
    ULONG                   SizeOfImage;
    UNICODE_STRING          FullDllName;
    UNICODE_STRING          BaseDllName;
    ULONG                   Flags;
    USHORT                  LoadCount;
    USHORT                  __Unused5;
    PVOID                   SectionPointer;
    ULONG                   CheckSum;
    PVOID                   LoadedImports;
    PVOID                   PatchInformation;
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;



namespace KE {

    __forceinline bool _wcscmp(const WCHAR* s1, const WCHAR* s2, SIZE_T maxlen) {
        while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0' && maxlen) {
            s1++;
            s2++;
            maxlen--;
        }
        return *s1 == *s2;
    }

    __forceinline SIZE_T _wcslen(const WCHAR* s) {
        const WCHAR* p = s;
        while (*p++) {}
        return p - s - 1;
    }

    template<typename T>
    bool FindKernelModule(PDRIVER_OBJECT ThisObj, const WCHAR* Name, T* ModuleBase, SIZE_T* ModuleSize) {
        PKLDR_DATA_TABLE_ENTRY Entry = nullptr;
        PLIST_ENTRY PsLoadedModuleList = nullptr;
        PLIST_ENTRY ListEntry = nullptr;

        PKLDR_DATA_TABLE_ENTRY LDR = reinterpret_cast<PKLDR_DATA_TABLE_ENTRY>(ThisObj->DriverSection);

        PsLoadedModuleList = LDR->InLoadOrderLinks.Flink;
        ListEntry = PsLoadedModuleList->Flink;

        while (ListEntry != PsLoadedModuleList)

        {
            Entry = CONTAINING_RECORD(ListEntry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

            if (&Entry->BaseDllName.Buffer != 0) {
                if (_wcscmp(Entry->BaseDllName.Buffer, Name, _wcslen(Name))) {
                    if (ModuleSize) { 
                        *ModuleSize = Entry->SizeOfImage; 
                    }
                    return *ModuleBase = reinterpret_cast<T>(Entry->DllBase);
                }
            }
            ListEntry = ListEntry->Flink;
        }
        return false;
    }


    bool GetKernelObject(const WCHAR* DriverName, const WCHAR* DeviceName, PDEVICE_OBJECT* DevObject, PDRIVER_OBJECT* DriverObject);
    bool GetKernelObject(const WCHAR* DriverName, PDEVICE_OBJECT* DevObject, PDRIVER_OBJECT* DriverObject);
}