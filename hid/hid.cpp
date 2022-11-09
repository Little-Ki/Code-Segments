#include "HID.h"

const static GUID HID_GUID = { 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

byte hid::getReportID(const device& device, HANDLE handle, HIDP_REPORT_TYPE type)
{
	uint16_t capsLen;
	PHIDP_PREPARSED_DATA pData;
	BYTE reportID;
	if (device.caps.NumberOutputButtonCaps != 0) {
		capsLen = (type == HIDP_REPORT_TYPE::HidP_Feature) ?
			device.caps.NumberFeatureButtonCaps :
			device.caps.NumberOutputButtonCaps;
		HIDP_BUTTON_CAPS* buttonCaps = new HIDP_BUTTON_CAPS[capsLen];
		HidD_GetPreparsedData(handle, &pData);
		HidP_GetButtonCaps(type, buttonCaps, &capsLen, pData);
		reportID = buttonCaps[0].ReportID;
		delete[] buttonCaps;
	}
	else {
		capsLen = (type == HIDP_REPORT_TYPE::HidP_Feature) ?
			device.caps.NumberFeatureValueCaps :
			device.caps.NumberOutputValueCaps;
		HIDP_VALUE_CAPS* valueCaps = new HIDP_VALUE_CAPS[capsLen];
		HidD_GetPreparsedData(handle, &pData);
		HidP_GetValueCaps(type, valueCaps, &capsLen, pData);
		reportID = valueCaps[0].ReportID;
		delete[] valueCaps;
	}
	return reportID;
}

#ifdef UNICODE
std::wstring hid::getDevPath(HDEVINFO devinfo, SP_DEVICE_INTERFACE_DATA& devInfoData)
{
	DWORD nRequiredSize = 0;
	if (!SetupDiGetDeviceInterfaceDetail(devinfo, &devInfoData, NULL, 0, &nRequiredSize, NULL))
	{
		BYTE* buffer = new BYTE[nRequiredSize];
		SP_DEVICE_INTERFACE_DETAIL_DATA* mDetail = (SP_DEVICE_INTERFACE_DETAIL_DATA*)buffer;
		mDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (SetupDiGetDeviceInterfaceDetail(devinfo, &devInfoData, mDetail, nRequiredSize, &nRequiredSize, NULL))
		{
			std::wstring result = std::wstring(mDetail->DevicePath);
			delete[] buffer;
			return result;
		}
	}
	return std::wstring(TEXT(""));
}
#else
std::string HID::getDevPath(HDEVINFO devinfo, SP_DEVICE_INTERFACE_DATA& devInfoData)
{
	DWORD mRequiredSize = 0;
	if (!SetupDiGetDeviceInterfaceDetail(devinfo, &devInfoData, nullptr, 0, &mRequiredSize, nullptr))
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA mDetail;
		mDetail.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		if (SetupDiGetDeviceInterfaceDetail(devinfo, &devInfoData, &mDetail, mRequiredSize, &mRequiredSize, nullptr))
		{
			return std::string(mDetail.DevicePath);
		}
	}
	return std::string("");
}
#endif

void hid::enumDevices(std::function<bool(const device&)> callback)
{
	uint32_t index = 0;
	HDEVINFO hDevInfo = SetupDiGetClassDevs(
		&HID_GUID,
		NULL,
		0,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
	);
	HIDD_ATTRIBUTES attributes;
	if (hDevInfo == INVALID_HANDLE_VALUE) {
		std::cout << "SetupDiGetClassDevs failed:" << GetLastError();
		return;
	}
	SP_DEVICE_INTERFACE_DATA devInfoData;
	devInfoData.cbSize = sizeof(devInfoData);

	while (SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &HID_GUID, index, &devInfoData)) {
		std::wstring path = getDevPath(hDevInfo, devInfoData);
		uint32_t flag = 0;
		HANDLE devHandle =
			CreateFile(
				path.c_str(),
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

		if (devHandle) {
			attributes.Size = sizeof(_HIDD_ATTRIBUTES);
			if (HidD_GetAttributes(devHandle, &attributes)) {
				device device;
				HIDP_CAPS caps;
				PHIDP_PREPARSED_DATA pData;
				HidD_GetPreparsedData(devHandle, &pData);
				HidP_GetCaps(pData, &caps);
				HidD_FreePreparsedData(pData);
				device.reportIDFuture = getReportID(device, devHandle, HIDP_REPORT_TYPE::HidP_Feature);
				device.reportIDOutput = getReportID(device, devHandle, HIDP_REPORT_TYPE::HidP_Output);
				device.handle = devHandle;
				device.attributes = attributes;
				device.caps = caps;
				if (callback(device)) {
					SetupDiDestroyDeviceInfoList(hDevInfo);
					return;
				}
				CloseHandle(devHandle);
			}
		}
		index++;
	};
	SetupDiDestroyDeviceInfoList(hDevInfo);
}

bool hid::writeData(const device& device, BYTE data[], int length)
{
	if (device.caps.OutputReportByteLength == 0 ||
		length != device.caps.OutputReportByteLength - 1) {
		return false;
	}
	else {
		DWORD out;
		BYTE* tmpData = new BYTE[device.caps.OutputReportByteLength];
		tmpData[0] = device.reportIDOutput;
		memcpy(&tmpData[1], &data[0], device.caps.OutputReportByteLength - 1);
		bool result = WriteFile(
			device.handle,
			tmpData,
			device.caps.OutputReportByteLength,
			&out,
			0
		);
		return result;
	}
}

bool hid::read_data(const device& device, byte* data)
{
	if (device.caps.InputReportByteLength == 0) {
		return false;
	}
	else {
		HANDLE hEvent = CreateEvent(NULL, true, false, NULL);
		OVERLAPPED overlap = { 0,0,0,0,hEvent };
		bool result = ReadFile(
			device.handle,
			data,
			device.caps.InputReportByteLength,
			nullptr,
			&overlap
		);
		WaitForSingleObject(hEvent, 500);
		return result;
	}
}

