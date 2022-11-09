#include "capture.h"


capture::capture()
{
}

capture::~capture()
{
	uninit();
}

bool capture::init()
{
	HRESULT hr = S_OK;

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	//
	// Create D3D device
	//
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels, D3D11_SDK_VERSION, &m_hDevice, &FeatureLevel, &m_hContext);
		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	if (FAILED(hr))
	{
		return false;
	}

	//
	// Get DXGI device
	//
	IDXGIDevice* hDxgiDevice = NULL;
	hr = m_hDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&hDxgiDevice));
	if (FAILED(hr))
	{
		return false;
	}

	//
	// Get DXGI adapter
	//
	IDXGIAdapter* hDxgiAdapter = NULL;
	hr = hDxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&hDxgiAdapter));
	RESET_OBJECT(hDxgiDevice);
	if (FAILED(hr))
	{
		return false;
	}

	//
	// Get output
	//
	INT nOutput = 0;
	IDXGIOutput* hDxgiOutput = NULL;
	hr = hDxgiAdapter->EnumOutputs(nOutput, &hDxgiOutput);
	RESET_OBJECT(hDxgiAdapter);
	if (FAILED(hr))
	{
		return false;
	}

	//
	// get output description struct
	//
	DXGI_OUTPUT_DESC        m_dxgiOutDesc;
	hDxgiOutput->GetDesc(&m_dxgiOutDesc);
	m_width = m_dxgiOutDesc.DesktopCoordinates.right - m_dxgiOutDesc.DesktopCoordinates.left;
	m_height = m_dxgiOutDesc.DesktopCoordinates.bottom - m_dxgiOutDesc.DesktopCoordinates.top;
	m_imgsize = m_width * m_height * 4;
	//
	// QI for Output 1
	//
	IDXGIOutput1* hDxgiOutput1 = NULL;
	hr = hDxgiOutput->QueryInterface(__uuidof(hDxgiOutput1), reinterpret_cast<void**>(&hDxgiOutput1));
	RESET_OBJECT(hDxgiOutput);
	if (FAILED(hr))
	{
		return false;
	}

	//
	// Create desktop duplication
	//
	hr = hDxgiOutput1->DuplicateOutput(m_hDevice, &m_hDeskDupl);
	RESET_OBJECT(hDxgiOutput1);
	if (FAILED(hr))
	{
		return false;
	}

	m_data = new unsigned char[m_imgsize];
	return true;
}

void capture::uninit()
{
	RESET_OBJECT(m_hDeskDupl);
	RESET_OBJECT(m_hContext);
	RESET_OBJECT(m_hDevice);
	if (m_data != nullptr) {
		delete[] m_data;
		m_data = nullptr;
	}
}

color* capture::get_pixel(int x, int y)
{
	static color black;
	if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
		return (color*)(
			&m_data[y * m_width * 4 + x * 4]
			);
	}
	return &black;
}

bool capture::updateimpl(void* pImgData, INT& nImgSize, int time)
{
	if (!attatchToThread())
	{
		return false;
	}

	nImgSize = 0;

	IDXGIResource* hDesktopResource = NULL;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	m_hDeskDupl->ReleaseFrame();
	HRESULT hr = m_hDeskDupl->AcquireNextFrame(1000, &FrameInfo, &hDesktopResource);
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
			return true;
		}
		uninit();
		init();
		return false;
	}

	//
	// query next frame staging buffer
	//
	ID3D11Texture2D* hAcquiredDesktopImage = NULL;
	hr = hDesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&hAcquiredDesktopImage));
	RESET_OBJECT(hDesktopResource);
	if (FAILED(hr))
	{
		return false;
	}

	//
	// copy old description
	//
	D3D11_TEXTURE2D_DESC frameDescriptor;
	hAcquiredDesktopImage->GetDesc(&frameDescriptor);

	//
	// create a new staging buffer for fill frame image
	//
	ID3D11Texture2D* hNewDesktopImage = NULL;
	frameDescriptor.Usage = D3D11_USAGE_STAGING;
	frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	frameDescriptor.BindFlags = 0;
	frameDescriptor.MiscFlags = 0;
	frameDescriptor.MipLevels = 1;
	frameDescriptor.ArraySize = 1;
	frameDescriptor.SampleDesc.Count = 1;
	hr = m_hDevice->CreateTexture2D(&frameDescriptor, NULL, &hNewDesktopImage);
	if (FAILED(hr))
	{
		RESET_OBJECT(hAcquiredDesktopImage);
		m_hDeskDupl->ReleaseFrame();
		return false;
	}

	//
	// copy next staging buffer to new staging buffer
	//
	m_hContext->CopyResource(hNewDesktopImage, hAcquiredDesktopImage);

	RESET_OBJECT(hAcquiredDesktopImage);
	m_hDeskDupl->ReleaseFrame();

	//
	// create staging buffer for map bits
	//
	IDXGISurface* hStagingSurf = NULL;
	hr = hNewDesktopImage->QueryInterface(__uuidof(IDXGISurface), (void**)(&hStagingSurf));
	RESET_OBJECT(hNewDesktopImage);
	if (FAILED(hr))
	{
		return false;
	}
	DXGI_SURFACE_DESC hStagingSurfDesc;
	// BGRA8
	hStagingSurf->GetDesc(&hStagingSurfDesc);
	//
	// copy bits to user space
	//
	DXGI_MAPPED_RECT mappedRect;
	hr = hStagingSurf->Map(&mappedRect, DXGI_MAP_READ);
	int imgSize = m_width * m_height * 4;
	if (SUCCEEDED(hr))
	{
		nImgSize = imgSize;
		memcpy(pImgData, mappedRect.pBits, imgSize);
		hStagingSurf->Unmap();
	}
	else return false;
	RESET_OBJECT(hStagingSurf);
	return SUCCEEDED(hr);
}


bool capture::update()
{
	return updateimpl(m_data, m_imgsize, 0);
}
