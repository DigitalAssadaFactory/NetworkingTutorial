#pragma once
#include "PNet/Core.h"


class MyClient : public PNet::Client
{
	virtual bool ProcessPacket(std::shared_ptr<PNet::Packet> packet) override;
	virtual void OnConnect() override;
	virtual void OnFail() override;
	virtual void OnDisconnect(std::string reason) override;
};