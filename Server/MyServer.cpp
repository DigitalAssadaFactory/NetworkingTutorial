#include "MyServer.h"

using namespace PNet;
void MyServer::OnConnect(PNet::TCPConnection& newConnection)
{
	Server::OnConnect(newConnection);

	std::shared_ptr<Packet> welcomePacket = std::make_shared<Packet>(PacketType::PT_ChatMessage);
	*welcomePacket << "Welcome!";

	std::shared_ptr<Packet> connectMessage = std::make_shared<Packet>(PacketType::PT_ChatMessage);
	*connectMessage << "User joined channel.";
	for (auto& connection : connections)
	{
		if (&connection == &newConnection)
			connection.pm_outgoing.Append(welcomePacket);
		else
			connection.pm_outgoing.Append(connectMessage);
	}
}

void MyServer::OnDisconnect(PNet::TCPConnection& lostConnection, std::string reason)
{
	PNet::Server::OnDisconnect(lostConnection, reason);

	std::shared_ptr<Packet> disconnectMessage = std::make_shared<Packet>(PacketType::PT_ChatMessage);
	*disconnectMessage << "User disconnected.";
	for (auto& connection : connections)
	{
		if (&connection == &lostConnection)
			continue;
		else
			connection.pm_outgoing.Append(disconnectMessage);
	}
}

bool MyServer::ProcessPacket(std::shared_ptr<PNet::Packet> packet)
{
	switch (packet->GetPacketType())
	{
	case PacketType::PT_ChatMessage:
	{
		std::string chatmessage;
		*packet >> chatmessage;
		std::cout << "Chat Message: " << chatmessage << std::endl;
		break;
	}
	case PacketType::PT_IntegerArray:
	{
		uint32_t arraySize = 0;
		*packet >> arraySize;
		std::cout << "Array Size: " << arraySize << std::endl;
		for (uint32_t i = 0; i < arraySize; i++)
		{
			uint32_t element = 0;
			*packet >> element;
			std::cout << "Element[" << i << "] - " << element << std::endl;
		}
		break;
	}
	default:
		std::cout << "Unrecognized packet type.\n";
		return false;
	}

	return true;
}
