#pragma once
namespace KMRoutines
{
	using f_MouseClassServiceCallback = VOID(__fastcall*)(
		PDEVICE_OBJECT		DeviceObject,
		PMOUSE_INPUT_DATA	InputDataStart,
		PMOUSE_INPUT_DATA	InputDataEnd,
		PULONG				InputDataConsumed
		);

	using f_KeyboardClassServiceCallback = VOID(__fastcall*)(
		PDEVICE_OBJECT			DeviceObject,
		PKEYBOARD_INPUT_DATA	InputDataStart,
		PKEYBOARD_INPUT_DATA	InputDataEnd,
		PULONG					InputDataConsumed
		);

	extern f_KeyboardClassServiceCallback	gKeyboardClassServiceCallback;
	extern f_MouseClassServiceCallback		gMouseClassServiceCallback;

	extern PDEVICE_OBJECT					gKeyboardObject;
	extern PDEVICE_OBJECT					gMouseObject;

	void MouseEvent(const PMOUSE_INPUT_DATA Package, UINT32 Size);
	void KeybdEvent(const PKEYBOARD_INPUT_DATA Package, UINT32 Size);

	bool Initialize(PDRIVER_OBJECT ThisObj);

	void Unload();

};

