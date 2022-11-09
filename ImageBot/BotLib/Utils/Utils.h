#pragma once
namespace BotLib {
    namespace Utils {

        void RGBToHSB(const BGRA& C, HSB& H);
        
        template<typename T = BGRA>
        bool SaveImageToBitmap(Bitmap<T>& Img, const std::string Path, bool Forward) {
            BITMAPINFOHEADER InfoHeader = { 0 };
            InfoHeader.biSize = sizeof(BITMAPINFOHEADER);
            InfoHeader.biWidth = Img.Width();
            InfoHeader.biHeight = Img.Height();
            InfoHeader.biPlanes = 1;
            InfoHeader.biBitCount = sizeof(T) * 8;
            InfoHeader.biCompression = BI_RGB;
            InfoHeader.biSizeImage = Img.Width() * Img.Height() * sizeof(T);
            InfoHeader.biXPelsPerMeter = 0;
            InfoHeader.biYPelsPerMeter = 0;
            InfoHeader.biClrUsed = 0;
            InfoHeader.biClrImportant = 0;

            BITMAPFILEHEADER FileHeader = { 0 };
            FileHeader.bfType = 'M' << 8 | 'B';
            FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            FileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + InfoHeader.biSizeImage;


            UINT32 LineWidth = Img.Width() * sizeof(T);
            LineWidth = (LineWidth + 3) & ~3;

            std::ofstream Out;
            Out.open(Path, std::ios::out | std::ios::trunc | std::ios::binary);

            if (!Out.good()) {
                std::cout << "Open file failed.\n";
                return false;
            }

            Out.write(reinterpret_cast<char*>(&FileHeader), sizeof(BITMAPFILEHEADER));
            Out.write(reinterpret_cast<char*>(&InfoHeader), sizeof(BITMAPINFOHEADER));
            if (Forward) {
                for (UINT32 i = 0; i < Img.Height(); i++) {
                    auto Begin = &Img.GetBuffer()[LineWidth * (Img.Height() - i - 1)];
                    Out.write(Begin, LineWidth);
                }
            }
            else {
                Out.write(Img.GetBuffer(), InfoHeader.biSizeImage);
            }

            Out.close();

            return true;
        }

        std::string RandomFileName(const std::string& Subfix);

    }
}