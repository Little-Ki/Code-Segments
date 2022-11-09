#pragma once
#include "..\include.h"
#include <assert.h>
#include <process.h>
#include "D3D11.h"
#include "DXGI1_2.h"
#include <iostream>
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

#define RESET_OBJECT(obj) { if(obj) obj->Release(); obj = NULL; }

struct color {
	unsigned char b, g, r, a;
public:

	color operator=(const color& that) {
		r = that.r;
		g = that.g;
		b = that.b;
		a = that.a;
		return *this;
	}

	color(uint32_t col) {
		r = col >> 24;
		g = (col >> 16) & 0xFF;
		b = (col >> 8) & 0xFF;
		a = col & 0xFF;
	}

	color() {
		r = 0;
		g = 0;
		b = 0;
		a = 0;
	}

	color(int r, int g, int b) {
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 255;
	}
};

class capture
{
public:
	capture();
	~capture();
	bool update();
	bool init();
	void uninit();
	int width() { return m_width; }
	int height() { return m_height; }
	color * get_pixel(int x, int y);
	uint8_t* get_bitmap() { return m_data; }
private:
	bool updateimpl(void* pImgData, INT& nImgSize, int time);

	bool attatchToThread(VOID)
	{
		HDESK hold = GetThreadDesktop(GetCurrentThreadId());
		HDESK hCurrentDesktop = OpenInputDesktop(0, FALSE, GENERIC_ALL);
		if (!hCurrentDesktop)
		{
			return FALSE;
		}

		bool bDesktopAttached = SetThreadDesktop(hCurrentDesktop) ? true : false;
		int err = GetLastError();
		CloseDesktop(hold);
		CloseDesktop(hCurrentDesktop);

		return bDesktopAttached;
	}

	capture(const capture&) = delete;
	capture& operator=(const capture&) = delete;

	ID3D11Device* m_hDevice = nullptr;
	ID3D11DeviceContext* m_hContext = nullptr;
	IDXGIOutputDuplication* m_hDeskDupl = nullptr;

	unsigned char* m_data = nullptr;
	int m_width = 0;
	int m_height = 0;
	int m_imgsize = 0;
};

