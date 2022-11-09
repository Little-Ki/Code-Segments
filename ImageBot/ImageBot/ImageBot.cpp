// ImageBot.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "..\BotLib\include.h"
#include <iostream>

bool KeepRunning = true;
bool ThreadExited = false;

BOOL WINAPI CmdHandlerRoutine(DWORD dwCtrlType) {
    if (CTRL_CLOSE_EVENT == dwCtrlType) {
        KeepRunning = false;
        while (!ThreadExited) {
            Sleep(100);
        }
    }
    return true;
}

UINT32 __stdcall Thread(void* arg) {

    BotLib::Bitmap<BotLib::BGRA> Img;
    BotLib::Dxgi Dxgi;

    if (!Dxgi.Initialize()) {
        std::cout << "Dxgi.Initialize() Failed.\n";
        return 0;
    }

    while (KeepRunning)
    {
        SHORT LastKeyState = false;
        SHORT KeyState = GetAsyncKeyState(VK_F8);
        Sleep(2500);
        if (!Dxgi.IsGood() || !Dxgi.Update(Img)) {
            std::cout << "Dxgi.Update(&) Failed.\n";
            continue;
        }
        std::cout << "Dxgi.Update(&) Success.\n";
        BotLib::Utils::SaveImageToBitmap(Img, BotLib::Utils::RandomFileName(".bmp"), true);

    }
    ThreadExited = true;
    return 0;
}

int main()
{

    SetConsoleCtrlHandler(CmdHandlerRoutine, TRUE);
    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, Thread, NULL, 0, NULL);
    if (hThread) { 
        CloseHandle(hThread);
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
