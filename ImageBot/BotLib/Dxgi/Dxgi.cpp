#include "..\include.h"
#include "Dxgi.h"

bool BotLib::Dxgi::CreateDevice()
{
	if (mInited) {
		return  true;
	}
	
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	HRESULT HR = S_OK;

	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		
		HR = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &mDevice, &FeatureLevel, &mContext);
		if (SUCCEEDED(HR))
		{
			break;
		}
	}

	if (FAILED(HR))
	{
		return false;
	}

	return true;
}

bool BotLib::Dxgi::CreateDumplication()
{
	HRESULT HR;
	CComPtr<IDXGIDevice>	DxgiDevice{ nullptr };
	CComPtr<IDXGIAdapter>	DxgiAdapter{ nullptr };
	CComPtr<IDXGIOutput>	DxgiOutput{ nullptr };
	CComPtr<IDXGIOutput1>	DxgiOutput1{ nullptr };
	DXGI_OUTPUT_DESC		DxgiOutDesc;

	HR = mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	if (FAILED(HR)) { return false; }

	HR = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	if (FAILED(HR)) { return false; }

	INT OutputCounts = 0;
	HR = DxgiAdapter->EnumOutputs(OutputCounts, &DxgiOutput);
	if (FAILED(HR)) { return false; }

	DxgiOutput->GetDesc(&DxgiOutDesc);
	mWidth	= DxgiOutDesc.DesktopCoordinates.right	- DxgiOutDesc.DesktopCoordinates.left;
	mHeight	= DxgiOutDesc.DesktopCoordinates.bottom - DxgiOutDesc.DesktopCoordinates.top;

	HR = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	if (FAILED(HR)) { return false; }

	HR = DxgiOutput1->DuplicateOutput(mDevice, &mDeskDup);
	if (FAILED(HR)) { return false; }

	return true;
}

inline bool BotLib::Dxgi::AttatchToThread()
{
	HDESK Hold = GetThreadDesktop(GetCurrentThreadId());
	HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hCurrentDesktop)
	{
		return false;
	}

	bool bDesktopAttached = SetThreadDesktop(hCurrentDesktop) ? true : false;
	CloseDesktop(Hold);
	CloseDesktop(hCurrentDesktop);

	return bDesktopAttached;
}

void BotLib::Dxgi::ReInitialize() {
	mDevice		= nullptr;
	mContext	= nullptr;
	mDeskDup	= nullptr;
	mInited		= false;
	Initialize();
}

bool BotLib::Dxgi::Initialize()
{
	if (!CreateDevice()) {
		return false;
	}

	if (!CreateDumplication()) {
		return false;
	}

	mInited = true;

	return true;
}

bool BotLib::Dxgi::Update(BotLib::Bitmap<BGRA>& Out)
{
	CComPtr<ID3D11Texture2D>	AcquiredDesktopImage{ nullptr };
	CComPtr<ID3D11Texture2D>	NewDesktopImage{ nullptr };
	CComPtr<IDXGIResource>		DesktopResource{ nullptr };
	CComPtr<IDXGISurface>		StagingSurface{ nullptr };
	DXGI_OUTDUPL_FRAME_INFO		FrameInfo;
	D3D11_TEXTURE2D_DESC		FrameDesc;
	DXGI_SURFACE_DESC			StagingSurfaceDesc;
	DXGI_MAPPED_RECT			MappedRect;

	mDeskDup->ReleaseFrame();
	HRESULT HR;
	HR = mDeskDup->AcquireNextFrame(
		1000,
		&FrameInfo,
		&DesktopResource
	);

	if (FAILED(HR))
	{
		if (HR != DXGI_ERROR_WAIT_TIMEOUT) {
			ReInitialize();
		}
		return false;
	}

	HR = DesktopResource->QueryInterface(
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&AcquiredDesktopImage)
	);
	if (FAILED(HR)) { return false; }

	AcquiredDesktopImage->GetDesc(&FrameDesc);

	FrameDesc.Usage = D3D11_USAGE_STAGING;
	FrameDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	FrameDesc.BindFlags = 0;
	FrameDesc.MiscFlags = 0;
	FrameDesc.MipLevels = 1;
	FrameDesc.ArraySize = 1;
	FrameDesc.SampleDesc.Count = 1;

	HR = mDevice->CreateTexture2D(&FrameDesc, NULL, &NewDesktopImage);
	if (FAILED(HR))
	{
		mDeskDup->ReleaseFrame();
		return false;
	}

	mContext->CopyResource(NewDesktopImage, AcquiredDesktopImage);
	mDeskDup->ReleaseFrame();

	HR = NewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void**)(&StagingSurface));
	if (FAILED(HR))
	{
		return false;
	}

	StagingSurface->GetDesc(&StagingSurfaceDesc);

	HR = StagingSurface->Map(&MappedRect, DXGI_MAP_READ);
	UINT32 ImgSize = mWidth * mHeight * 4;
	if (SUCCEEDED(HR))
	{
		Out.Resize(mWidth, mHeight);
		memcpy(Out.GetBuffer(), MappedRect.pBits, ImgSize);
		StagingSurface->Unmap();
	}

	return SUCCEEDED(HR);
}

