#include "Packet.h"


namespace PNet {

	Packet::Packet(PacketType type)
	{
		Clear();
		SetPacketType(type);
	}
	void Packet::Clear()
	{
		buffer.resize(sizeof(PacketType));
		SetPacketType(PacketType::PT_Invalid);
		extractionOffset = sizeof(PacketType);
	}

	void Packet::Append(const void* data, uint32_t size)
	{
		if (buffer.size() + size > g_MaxPacketSize)
			throw PacketException("[Packet::Append(const void*, uint32_t)] - Packet exceeded max packet size.");

		buffer.insert(buffer.end(), (char*)data, (char*)data + size);
	}

	void Packet::SetPacketType(PacketType type)
	{
		/*if (buffer.size() < sizeof(type)) buffer.resize(sizeof(type));
		memset(buffer.data(), htons((uint16_t)type), sizeof(type));*/
		PacketType* packetTypePtr = reinterpret_cast<PacketType*>(&buffer[0]);
		*packetTypePtr = static_cast<PacketType>(htons((uint16_t)type));
	}

	PacketType Packet::GetPacketType()
	{
		PacketType* packetTypePtr = reinterpret_cast<PacketType*>(&buffer[0]);
		return static_cast<PacketType>(ntohs((uint16_t)*packetTypePtr));
	}

	Packet& Packet::operator<<(uint32_t data)
	{
		data = htonl(data);
		Append(&data, sizeof(data));
		return *this;
	}

	Packet& Packet::operator>>(uint32_t& data)
	{
		if (extractionOffset + sizeof(uint32_t) > buffer.size())
			throw PacketException("[Packet::operator >>(uint32_t& data)] - Extraction offset exceeded buffer size.");

		data = *reinterpret_cast<uint32_t*>(&buffer[extractionOffset]);
		data = ntohl(data);
		extractionOffset += sizeof(uint32_t);
		return *this;
	}

	Packet& Packet::operator<<(std::string data)
	{
		*this << (uint32_t)data.size();
		Append(data.data(), data.size());
		return *this;
	}

	Packet& Packet::operator>>(std::string& data)
	{
		data.clear();
		uint32_t stringSize = 0;
		*this >> stringSize;

		if(extractionOffset + stringSize > buffer.size())

		data.resize(stringSize);
		data.assign(&buffer[extractionOffset], stringSize);
		extractionOffset += stringSize;
		return *this;
	}

}