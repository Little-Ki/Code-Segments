#include "..\include.h"
#include "Utils.h"

void BotLib::Utils::RGBToHSB(const BGRA& C, HSB& H) {
#define min3v(v1, v2, v3)   ((v1)>(v2)? ((v2)>(v3)?(v3):(v2)):((v1)>(v3)?(v3):(v2)))
#define max3v(v1, v2, v3)   ((v1)<(v2)? ((v2)<(v3)?(v3):(v2)):((v1)<(v3)?(v3):(v1)))
    INT16 Max = max3v(C.R, C.G, C.B);
    INT16 Min = min3v(C.R, C.G, C.B);
    float Disp = static_cast<float>(Max - Min);

    H.h = 0;
    H.s = (Max == 0) ? 0 : ((float)(Max - Min) / (float)Max * 100);
    H.b = Max / 255.f * 100;

    if (Max == C.R && C.G >= C.B) {
        H.h = 60 * (float)(C.G - C.B) / Disp;
    }
    else if (Max == C.R && C.G < C.B) {
        H.h = 60 * (float)(C.G - C.B) / Disp + 360;
    }
    else if (Max == C.G) {
        H.h = 60 * (float)(C.B - C.R) / Disp + 120;
    }
    else if (Max == C.B) {
        H.h = 60 * (float)(C.R - C.G) / Disp + 240;
    }
}

std::string BotLib::Utils::RandomFileName(const std::string& Subfix) {
    std::stringstream ss;
    srand(time(0));
    for (int i = 6; i--;) {
        ss << 'A' + (rand() % 26);
    }
    ss << Subfix;
    return ss.str();
}
