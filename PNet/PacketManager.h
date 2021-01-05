#pragma once
#include "Packet.h"
#include <queue>
#include <memory>


namespace PNet {

	enum class PacketManagerTask
	{
		ProcessPacketSize,
		ProcessPacketContents
	};

	class PacketManager {
		std::queue<std::shared_ptr<Packet>> packets;
	public:
		void Clear();
		bool HasPendingPackets();
		void Append(std::shared_ptr<Packet> p);
		std::shared_ptr<Packet> Retrieve();
		void Pop();
	
		uint16_t currentPacketSize = 0;
		int currentExtractionOffset = 0;
		PacketManagerTask currentTask = PacketManagerTask::ProcessPacketSize;
	};

}