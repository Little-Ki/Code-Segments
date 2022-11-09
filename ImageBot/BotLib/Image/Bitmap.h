#pragma once

namespace BotLib {

	class BGRA;

	template<typename T>
	class Bitmap {
	private:
		std::vector<T>		mBuffer;
		T					mReserved;

		UINT32				mWidth;
		UINT32				mHeight;

	public:

		Bitmap() :mWidth(0), mHeight(0), mReserved{ 0U }{}

		void Resize(UINT32 Width, UINT32 Height) {
			if (Width != mWidth || Height != mHeight) {
				mBuffer.resize(static_cast<UINT64>(Width) * Height);
				mWidth = Width;
				mHeight = Height;
			}
		}

		char* GetBuffer() {
			return reinterpret_cast<char*>(mBuffer.data());
		}

		UINT32 Width() const {
			return mWidth;
		}

		UINT32 Height() const {
			return mHeight;
		}

		void Clear() {
			memset(
				GetBuffer(),
				0,
				static_cast<ULONG64>(mWidth) * static_cast<ULONG64>(mHeight) * sizeof(T)
			);
			memset(
				reinterpret_cast<char*>(&mReserved),
				0,
				sizeof(T)
			);
		}

		T& operator()(UINT32 X, UINT32 Y) {
			if (X < mWidth && Y < mHeight) {
				return mBuffer[static_cast<ULONG64>(Y) * mWidth + X];
			}
			return mReserved;
		}
	};
}