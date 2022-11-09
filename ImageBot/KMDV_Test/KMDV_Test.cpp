// KMDV_Test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "include.h"

#define IOCTL_IO_MOUSE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_IO_KEYBD		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)


struct MOUSE_BUTTON {
	UINT16 Data;
	struct {
		bool LeftDown : 1;
		bool LeftUp : 1;
		bool RightDown : 1;
		bool RightUp : 1;
		bool MiddleDown : 1;
		bool MiddleUp : 1;
		bool Btn4Down : 1;
		bool Btn4Up : 1;
		bool Btn5Down : 1;
		bool Btn5Up : 1;
		bool Wheel : 1;
		bool HWheel : 1;
		bool Pad : 5;
	} b;
};

struct KEYBOARD_INPUT_DATA {
	USHORT 	UnitId;
	USHORT 	MakeCode;
	USHORT 	Flags;
	USHORT 	Reserved;
	ULONG 	ExtraInformation;
};

struct MOUSE_INPUT_DATA {
	USHORT	UnitId;
	USHORT	Flags;
	union {
		ULONG Buttons;
		struct {
			MOUSE_BUTTON	ButtonFlags;
			USHORT			ButtonData;
		};
	};

	ULONG	RawButtons;
	LONG	LastX;
	LONG	LastY;
	ULONG	ExtraInformation;

};

std::string GetAppPath(std::string FileName) //最后带斜杠
{
	char Buffer[255];
	GetCurrentDirectoryA(255, Buffer);
	return std::string(Buffer) + "\\" + FileName;
};

enum KSC {
	KSC_ESC = 0x01,
	KSC_1,
	KSC_2,
	KSC_3,
	KSC_4,
	KSC_5,
	KSC_6,
	KSC_7,
	KSC_8,
	KSC_9,
	KSC_0,
	KSC_MINUS,
	KSC_EQUAL,
	KSC_BACK,
	KSC_TAB,
	KSC_Q,
	KSC_W,
	KSC_E,
	KSC_R,
	KSC_T,
	KSC_Y,
	KSC_U,
	KSC_I,
	KSC_O,
	KSC_P,
	KSC_LSBRACKET,
	KSC_RSBRACKET,
	KSC_CTRL = 0x1D,
	KSC_A,
	KSC_S,
	KSC_D,
	KSC_F,
	KSC_G,
	KSC_H,
	KSC_J,
	KSC_K,
	KSC_L,
	KSC_SEMICOLON,
	KSC_APOSTROPHE,
	KSC_BACKTICKS,
	KSC_LSHIFT,
	KSC_BACKSLASH,
	KSC_Z,
	KSC_X,
	KSC_C,
	KSC_V,
	KSC_B,
	KSC_N,
	KSC_M,
	KSC_COMMA,
	KSC_DOT,
	KSC_SLASH,
	KSC_RSHIFT,
	KSC_PRTSC,
	KSC_ALT,
	KSC_SPACE,
	KSC_CAPS,
	KSC_F1,
	KSC_F2,
	KSC_F3,
	KSC_F4,
	KSC_F5,
	KSC_F6,
	KSC_F7,
	KSC_F8,
	KSC_F9,
	KSC_F10,
	KSC_NUM,
	KSC_SCROLL,
	KSC_HOME,
	KSC_UP,
	KSC_PGUP,
	KSC_MINUS2,
	KSC_LEFT,
	KSC_CENTER,
	KSC_RIGHT,
	KSC_ADD,
	KSC_END,
	KSC_DOWN,
	KSC_PGDOWN,
	KSC_INS,
	KSC_DEL
};


void PressKey(DriverCtrl& Ctrl, KSC Key) {
	KEYBOARD_INPUT_DATA Pack[2] = { 0 };
	Pack[0].MakeCode = Key;
	Pack[1].MakeCode = Key;
	Pack[0].Flags = 0;
	Pack[1].Flags = 1;
	Ctrl.IoControl(IOCTL_IO_KEYBD, &Pack, sizeof(Pack), nullptr, 0, nullptr);
	Sleep(200);
}

int main()
{
    DriverCtrl			Ctrl;
	char SvcLnkName[]		= "KMDV";;

	std::string SysFile = GetAppPath("KMDV.sys");
	std::cout << SysFile << std::endl;
	system("pause");
	bool Ret = true;

	Ret = Ctrl.Install((char*)SysFile.c_str(), SvcLnkName, SvcLnkName);
	if (!Ret) {
		std::cout << "Install failed.\n";
		system("pause");
		return 0;
	}

	Ret = Ctrl.Start();
	if (!Ret) {
		std::cout << "Start failed.\n";
		system("pause");
		return 0;
	}

	char SvcLnkName2[] = "\\\\.\\KMDV";
	Ret &= Ctrl.Open(SvcLnkName2);
	if (!Ret) {
		std::cout << "Open failed.\n";
		system("pause");
		return 0;
	}

	std::cout << "Sleep...\n";
	Sleep(3000);
	for (int i = 0; i < 3; i++) {
		PressKey(Ctrl, KSC::KSC_A);
		Sleep(1000);
	}
	CloseHandle(Ctrl.mDriverHandle);

	Ctrl.Stop();
	Ctrl.Remove();
	system("pause");
	return 0;
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
