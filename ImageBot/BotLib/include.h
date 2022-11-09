#pragma once


#define WIN32_LEAN_AND_MEAN             
#include <Windows.h>
#include <atlbase.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <ctime>

#include <memory>
#include <algorithm>

#include <assert.h>
#include <process.h>

#include "D3D11.h"
#include "DXGI1_2.h"
#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

#include "Image\Color.h"
#include "Image\Bitmap.h"

#include "Dxgi\Dxgi.h"

#include "Utils\Utils.h"