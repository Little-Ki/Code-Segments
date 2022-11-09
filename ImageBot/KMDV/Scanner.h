#pragma once

namespace Scanner
{
	__forceinline bool IsHex(const char c) {
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	__forceinline UINT16 ToDec(const char c) {
		if (c >= '0' && c <= '9') {
			return c - '0';
		}
		if (c >= 'a' && c <= 'f') {
			return c - 'a' + 10;
		}
		if (c >= 'A' && c <= 'F') {
			return c - 'A' + 10;
		}
		return 0;
	}

	class PatternByte {
	private:
		UINT8	mByte;
		bool	mWild;

	public:
		PatternByte(UINT8 Byte) :mByte(Byte), mWild(false) {};
		PatternByte() :mByte(0), mWild(true) {};

		const UINT8& Byte() const {
			return mByte;
		}

		const bool& Wild() const {
			return mWild;
		}


		__forceinline const bool Compare(const UINT8* c) const {
			return mWild ? true : *c == mByte;
		}


		__forceinline const bool Compare(const UINT8& c) const {
			return mWild ? true : c == mByte;
		}
	};

	template<SIZE_T N, SIZE_T M>
	class Pattern {
	private:
		PatternByte mData[N];
	public:
		Pattern(const Pattern<N, M>&) = default;
		Pattern(const char* Buffer)
		{
			UINT32 i = 0;
			const char* c = Buffer;
			const char* end = Buffer + M;
			while (c < end) {
				if (IsHex(c[0]) && IsHex(c[1])) {
					mData[i++] = PatternByte((ToDec(c[0]) << 4) + ToDec(c[1]));
				}
				else if (c[0] == '?') {
					mData[i++] = PatternByte();
				}
				c++;
			}
		}

		const PatternByte& operator[](UINT32 Index) const {
			return mData[Index];
		}
	};


	template<SIZE_T N, SIZE_T M>
	bool CompareData(PVOID Base, const Pattern<N, M>& P) {
		UINT8* Cur = reinterpret_cast<UINT8*>(Base);
		for (int i = 0; i < N; i++) {
			if (!P[i].Compare(Cur[i])) {
				return false;
			}
		}
		return true;
	}

#define MEM_FIND_TAG 'iTag'

	template<typename T, SIZE_T N, SIZE_T M>
	T Find(PVOID Base, SIZE_T Size, const Pattern<N, M>& P, INT32 Offset) {
		UINT8* Buffer = reinterpret_cast<UINT8*>(Base);
		for (UINT32 i = 0; i < Size - N + 1; i++) {
			if (CompareData(&Buffer[i], P)) {
				return reinterpret_cast<T>(reinterpret_cast<UINT8*>(Base) + i + Offset);
			}
		}
		return static_cast<T>(0);
	}


	template<typename T, SIZE_T N, SIZE_T M>
	T FineInCodes(PVOID Base, const Pattern<N, M>& P, INT32 Offset) {
		auto UBase = reinterpret_cast<UINT8*>(Base);
		auto DosHeader	= reinterpret_cast<PIMAGE_DOS_HEADER>(UBase);
		auto NtHeader	= reinterpret_cast<PIMAGE_NT_HEADERS64>(UBase + DosHeader->e_lfanew);
		auto Sections	= reinterpret_cast<PIMAGE_SECTION_HEADER>(NtHeader + 1);

		for (UINT32 i = 0; i < NtHeader->FileHeader.NumberOfSections; i++) {
			auto Section	= &Sections[i];
			auto Desc		= Section->Characteristics;
			if (
				(Desc & IMAGE_SCN_CNT_CODE) &&
				(Desc & IMAGE_SCN_MEM_EXECUTE) &&
				(Desc & IMAGE_SCN_MEM_READ) &&
				!(Desc & IMAGE_SCN_MEM_DISCARDABLE)
				) {
				T Tmp = Find<T>(UBase + Section->VirtualAddress, Section->SizeOfRawData, P, Offset);
				if (Tmp) {
					return Tmp;
				}
			}
		}
		return static_cast<T>(0);
	}
};

#define MAKE_PATTERN(str,n) Scanner::Pattern<n,sizeof(str) - 1>(str)