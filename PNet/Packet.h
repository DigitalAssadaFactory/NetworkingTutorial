#pragma once
#include <vector>
#include <string>
#include <WinSock2.h>
#include "PacketException.h"
#include "Constants.h"
#include "PacketType.h"


namespace PNet {

	class Packet {
	public:
		Packet(PacketType type = PacketType::PT_Invalid);

		void Clear();
		void Append(const void* data, uint32_t size);

		void SetPacketType(PacketType type);
		PacketType GetPacketType();

		Packet& operator << (uint32_t data);
		Packet& operator >>	(uint32_t& data);
		Packet& operator << (std::string data);
		Packet& operator >> (std::string& data);

		uint32_t extractionOffset = 0;
		std::vector<char> buffer;
	};

}