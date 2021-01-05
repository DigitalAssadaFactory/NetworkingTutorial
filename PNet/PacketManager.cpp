#include "PacketManager.h"


namespace PNet {

	void PacketManager::Clear()
	{
		packets = std::queue<std::shared_ptr<Packet>>{};
	}

	bool PacketManager::HasPendingPackets()
	{
		return !packets.empty();
	}

	void PacketManager::Append(std::shared_ptr<Packet> p)
	{
		packets.push(std::move(p));
	}

	std::shared_ptr<Packet> PacketManager::Retrieve()
	{
		return packets.front();
	}

	void PacketManager::Pop()
	{
		packets.pop();
	}

}