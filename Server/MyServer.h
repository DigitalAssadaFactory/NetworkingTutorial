#pragma once
#include "PNet/Core.h"


class MyServer : public PNet::Server
{
	void OnConnect(PNet::TCPConnection& newConnection) override;
	void OnDisconnect(PNet::TCPConnection& lostConnection, std::string reason) override;
	bool ProcessPacket(std::shared_ptr<PNet::Packet> packet) override;
};