#pragma once
#include "..\Image\Bitmap.h"
#include "..\Image\Color.h"

#include "D3D11.h"
#include "DXGI1_2.h"
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

namespace BotLib {
	class Dxgi {
	private:

		CComPtr<ID3D11Device>			mDevice;
		CComPtr<ID3D11DeviceContext>	mContext;
		CComPtr<IDXGIOutputDuplication>	mDeskDup;

		bool	mInited;
		UINT32	mWidth;
		UINT32	mHeight;

		bool CreateDevice();
		bool CreateDumplication();
		bool AttatchToThread();

		Dxgi(const Dxgi&) = delete;
		Dxgi& operator=(const Dxgi&) = delete;

		void ReInitialize();

	public:

		Dxgi() :
			mWidth(0), mHeight{ 0 }, mInited{ false },
			mDevice{ nullptr }, mContext{ nullptr }, mDeskDup{ nullptr }{};

		const bool & IsGood() { 
			return mInited;
		}

		bool Initialize();

		bool Update(BotLib::Bitmap<BGRA>& Out);

	};
}