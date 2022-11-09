#pragma once

namespace BotLib {

	class BGRA {
	public:
		union {
			UINT32 Data;
			struct {
				UINT8 B, G, R, A;
			};
		};
		BGRA() : Data{ 0U } {}
		BGRA(UINT32 C) : Data{ C } {}
	};

	struct HSB {
		float h, s, b;
	};
}