#pragma once
#include <stdint.h>


namespace PNet {

	enum class PacketType : uint16_t
	{
		PT_Invalid,
		PT_ChatMessage,
		PT_IntegerArray
	};

}