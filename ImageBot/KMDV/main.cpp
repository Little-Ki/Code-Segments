#include "include.h"

extern "C" {

	UNICODE_STRING LinkNameW	= RTL_CONSTANT_STRING(L"\\DosDevices\\Global\\KMDV");
	UNICODE_STRING DeviceNameW	= RTL_CONSTANT_STRING(L"\\Device\\KMDV");

	VOID DriverUnload(PDRIVER_OBJECT DriverObj);
	NTSTATUS IOControl(PDEVICE_OBJECT, PIRP IRP);
	NTSTATUS DispatchCreate(PDEVICE_OBJECT DevObj, PIRP IRP);
	NTSTATUS DispatchClose(PDEVICE_OBJECT DevObj, PIRP IRP);
	NTSTATUS DispatchWrite(PDEVICE_OBJECT DevObj, PIRP IRP);

	NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObj, PUNICODE_STRING)
	{
		DbgPrint("[KMDV] Load.\n");
		NTSTATUS NtRet = STATUS_SUCCESS;
		PDEVICE_OBJECT DeviceObj;
		
		DriverObj->MajorFunction[IRP_MJ_CREATE]			= DispatchCreate;
		DriverObj->MajorFunction[IRP_MJ_CLOSE]			= DispatchClose;
		DriverObj->MajorFunction[IRP_MJ_WRITE]			= DispatchWrite;
		DriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]	= IOControl;
		DriverObj->DriverUnload							= DriverUnload;

		//RtlInitUnicodeString(&DeviceNameW, DEVICE_NAME);
		NtRet = IoCreateDevice(DriverObj, 0, &DeviceNameW, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObj);

		if (!NT_SUCCESS(NtRet)) {

			DbgPrint("[KMDV] IoCreateDevice failed.\n");

			return NtRet;
		}
		
		//RtlInitUnicodeString(&LinkNameW, LINK_GLOBAL_NAME);
		NtRet = IoCreateSymbolicLink(&LinkNameW, &DeviceNameW);
		if (!NT_SUCCESS(NtRet))
		{
			DbgPrint("[KMDV] IoCreateSymbolicLink failed: %x\n", NtRet);
			
			IoDeleteDevice(DeviceObj);
			return NtRet;
		}

		//DbgPrint("[KMDV] Find KMDV at: %x\n", KE::FindKeModule<UINT_PTR>(DriverObj, L"KMDV.sys", nullptr));
		
		if (!KMRoutines::Initialize(DriverObj)) {

			DbgPrint("[KMDV] Initialize KMRoutines failed.\n");

			DbgPrint("[KDVM] KeyboardClassServiceCallback: %p.",KMRoutines:: gKeyboardClassServiceCallback);
			DbgPrint("[KDVM] MouseClassServiceCallback: %p.", KMRoutines::gMouseClassServiceCallback);
			DbgPrint("[KDVM] Keyboard device: %p.", KMRoutines::gKeyboardObject);
			DbgPrint("[KDVM] Mouse device: %p.", KMRoutines::gMouseObject);

			IoDeleteSymbolicLink(&LinkNameW);
			IoDeleteDevice(DeviceObj);
			return STATUS_FAILED_DRIVER_ENTRY;
		}
		
		return STATUS_SUCCESS;
	}

	VOID DriverUnload(PDRIVER_OBJECT DriverObject)
	{
		DbgPrint("[KMDV] UnLoad.\n");

		KMRoutines::Unload();
		IoDeleteSymbolicLink(&LinkNameW);
		IoDeleteDevice(DriverObject->DeviceObject);
	}

	NTSTATUS IOControl(PDEVICE_OBJECT, PIRP IRP)
	{
		DbgPrint("[KMDV] IO Control.\n");

		NTSTATUS ntRet = STATUS_SUCCESS;
		PIO_STACK_LOCATION IRPStack;
		ULONG IOCtrlCode;
		PVOID IOBuffer;
		ULONG InSize;
		ULONG OutSize;

		IRPStack	= IoGetCurrentIrpStackLocation(IRP);
		IOBuffer	= IRP->AssociatedIrp.SystemBuffer;
		IOCtrlCode	= IRPStack->Parameters.DeviceIoControl.IoControlCode;
		InSize		= IRPStack->Parameters.DeviceIoControl.InputBufferLength;
		OutSize		= IRPStack->Parameters.DeviceIoControl.OutputBufferLength;

		switch (IOCtrlCode)
		{
		case IOCTL_IO_MOUSE: {
			if (InSize % sizeof(MOUSE_INPUT_DATA) != 0) {

				DbgPrint("[KMDV] IO Control: Package size invalid.\n");
				ntRet = STATUS_BAD_DATA;
				break;
			}
			DbgPrint("[KMDV] IO Control: Mouse event.\n");
			KMRoutines::MouseEvent(reinterpret_cast<PMOUSE_INPUT_DATA>(IOBuffer), InSize / sizeof(MOUSE_INPUT_DATA));
			break;
		}
		case IOCTL_IO_KEYBD: {
			if (InSize % sizeof(KEYBOARD_INPUT_DATA) != 0) {

				DbgPrint("[KMDV] IO Control: Package size invalid.\n");
				ntRet = STATUS_BAD_DATA;
				break;
			}
			DbgPrint("[KMDV] IO Control: Keybd event.\n");
			KMRoutines::KeybdEvent(reinterpret_cast<PKEYBOARD_INPUT_DATA>(IOBuffer), InSize / sizeof(KEYBOARD_INPUT_DATA));
			break;
		}
		default:
			ntRet = STATUS_INVALID_DEVICE_REQUEST;
			DbgPrint("[KMDV] IO Control: Unknown control code: %X .\n", IOCtrlCode);
		}

		IRP->IoStatus.Information = ntRet ? 0 : OutSize;
		IRP->IoStatus.Status = ntRet;

		IoCompleteRequest(IRP, IO_NO_INCREMENT);
		return ntRet;
	}

	NTSTATUS DispatchWrite(PDEVICE_OBJECT DevObj, PIRP IRP) {
		DbgPrint("[KMDV] DispatchWrite\n");
		IRP->IoStatus.Status = STATUS_SUCCESS;
		IRP->IoStatus.Information = 0;
		IoCompleteRequest(IRP, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	NTSTATUS DispatchCreate(PDEVICE_OBJECT DevObj, PIRP IRP)
	{
		DbgPrint("[KMDV] DispatchCreate\n");
		IRP->IoStatus.Status = STATUS_SUCCESS;
		IRP->IoStatus.Information = 0;
		IoCompleteRequest(IRP, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

	NTSTATUS DispatchClose(PDEVICE_OBJECT DevObj, PIRP IRP)
	{
		DbgPrint("[KMDV] DispatchClose\n");
		IRP->IoStatus.Status = STATUS_SUCCESS;
		IRP->IoStatus.Information = 0;
		IoCompleteRequest(IRP, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}

}