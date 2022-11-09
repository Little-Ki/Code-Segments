#include "include.h"
#include "KMRoutines.h"

KMRoutines::f_KeyboardClassServiceCallback	KMRoutines::gKeyboardClassServiceCallback;
KMRoutines::f_MouseClassServiceCallback		KMRoutines::gMouseClassServiceCallback;
PDEVICE_OBJECT					            KMRoutines::gKeyboardObject;
PDEVICE_OBJECT					            KMRoutines::gMouseObject;


__forceinline SIZE_T StrLen(const char* Str) {
    const char* c = Str;
    while (*c++) {}
    return c - Str - 1;
}

template<typename T>
__forceinline T RelToAbs(int* Address) {
    return reinterpret_cast<T>(((char*)(Address + 1)) + *Address);
}

UINT_PTR FastScan(const char* Begin, UINT32 ScanSize, const char* Pattern, const char* Mask, SIZE_T PatternSize) {

    const char* End = Begin + ScanSize;

    int* Next = reinterpret_cast<int*>(ExAllocatePool(PagedPool, PatternSize * sizeof(int)));
   
    if (!Next) {
        return 0;
    }
    // I cant determine why memset works bad
    for (int i = 0; i < PatternSize; i++) {
        Next[i] = 0;
    }

    for (int i = 0; i < PatternSize - 1; i++) {

        if (Mask[i] == 'x' &&
            Pattern[Next[i]] == Pattern[i] &&
            Mask[i + 1] != '?' &&
            Next[i] != i) {
            Next[i + 1] = Next[i] + 1;
        }
    }
    SIZE_T i = 0;
    while (Begin != End) {
        if (Begin + PatternSize - i > End) {
            return 0;
        }
        if (*Begin == Pattern[i] || Mask[i] == '?') {
            if (i == PatternSize - 1) {
                ExFreePool(Next);
                return reinterpret_cast<UINT_PTR>( Begin - i);
            }
            else {
                Begin++;
                i++;
            }
        }
        else {
            if (i == 0) {
                Begin++;
            }
            else {
                i = Next[i];
            }
        }
    }
    ExFreePool(Next);
    return 0;
}

bool SearchFunctions(PDRIVER_OBJECT ThisObj)
{
    bool Ret = true;

    Ret &= KE::GetKernelObject(L"\\Driver\\kbdclass"    , &KMRoutines::gKeyboardObject  , nullptr);
    Ret &= KE::GetKernelObject(L"\\Driver\\mouclass"    , &KMRoutines::gMouseObject     , nullptr);
    // Ret &= KE::GetKernelObject(L"\\Driver\\kbdclass", L"\\Device\\KeyboardClass0", &KMRoutines::gKeyboardObject, nullptr);
    // Ret &= KE::GetKernelObject(L"\\Driver\\mouclass", L"\\Device\\PointerClass0", &KMRoutines::gMouseObject, nullptr);

    if (!Ret) {
        DbgPrint("[KMDV] SearchFunctions failed.\n");
        return false;
    }

    UINT8*  KeybdModuleBase;
    SIZE_T  KeybdModuleSize;

    UINT8*  MouseModuleBase;
    SIZE_T  MouseModuleSize;

    Ret &= KE::FindKernelModule(ThisObj, L"kbdclass.sys", &KeybdModuleBase, &KeybdModuleSize);
    Ret &= KE::FindKernelModule(ThisObj, L"mouclass.sys", &MouseModuleBase, &MouseModuleSize);

    if (!Ret) {
        DbgPrint("[KMDV] kbdclass.sys or mouclass.sys not found.\n");
        return false;
    }

    
    DbgPrint("[KMDV] kbdclass.sys: base %p, size %X.\n", KeybdModuleBase, KeybdModuleSize);
    DbgPrint("[KMDV] mouclass.sys: base %p, size %X.\n", MouseModuleBase, MouseModuleSize);

    int* Rel;
    Rel = Scanner::FineInCodes<int*>(KeybdModuleBase,MAKE_PATTERN("B9 03 02 0B 00 48 8D 05 ? ? ? ? 48 89 44 24 68", 17), 8);
    if (!Rel) {
        DbgPrint("[KMDV] Find KeyboardClassServiceCallback in kbdclass.sys failed.\n");
        return false;
    }

    KMRoutines::gKeyboardClassServiceCallback = RelToAbs<KMRoutines::f_KeyboardClassServiceCallback>(Rel);

    Rel = Scanner::FineInCodes<int*>(MouseModuleBase, MAKE_PATTERN("B9 03 02 0F 00 48 8D 05 ? ? ? ? 48 89 44 24 68", 17), 8);
    if (!Rel) {
        DbgPrint("[KMDV] Find MouseClassServiceCallback in mouclass.sys failed.\n");
        return false;
    }
    KMRoutines::gMouseClassServiceCallback = RelToAbs<KMRoutines::f_MouseClassServiceCallback>(Rel);
    
    return true;

}


void KMRoutines::MouseEvent(const PMOUSE_INPUT_DATA Package, UINT32 Size)
{
    if (!gMouseClassServiceCallback) {
        return;
    }

    ULONG InputDataConsumed = 0;
    gMouseClassServiceCallback(gMouseObject, Package, Package + Size, &InputDataConsumed);
}

void KMRoutines::KeybdEvent(const PKEYBOARD_INPUT_DATA Package, UINT32 Size)
{
    if (!gKeyboardClassServiceCallback) {
        return;
    }

    ULONG InputDataConsumed = 0;
    gKeyboardClassServiceCallback(gKeyboardObject, Package, Package + Size, &InputDataConsumed);
}

bool KMRoutines::Initialize(PDRIVER_OBJECT ThisObj)
{
	if (!SearchFunctions(ThisObj)) {
		return false;
	}

    DbgPrint("[KMDV] KMRoutines initialized.\n");
    DbgPrint("[KMDV] KeyboardClassServiceCallback: %p.\n", gKeyboardClassServiceCallback);
    DbgPrint("[KMDV] MouseClassServiceCallback: %p.\n", gMouseClassServiceCallback);
    DbgPrint("[KMDV] Keyboard device: %p.\n", gKeyboardObject);
    DbgPrint("[KMDV] Mouse device: %p.\n", gMouseObject);
	return true;
}

void KMRoutines::Unload()
{
}
