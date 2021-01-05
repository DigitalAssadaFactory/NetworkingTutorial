#include "MyClient.h"


using namespace PNet;

bool MyClient::ProcessPacket(std::shared_ptr<PNet::Packet> packet)
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

void MyClient::OnConnect()
{
	std::cout << "Successfully connected to the server.\n";

	std::shared_ptr<Packet> helloMessage = std::make_shared<Packet>(PacketType::PT_ChatMessage);
	*helloMessage << "Hello from client.";
	connection.pm_outgoing.Append(helloMessage);
}

void MyClient::OnFail()
{
	Client::OnFail();
}

void MyClient::OnDisconnect(std::string reason)
{
	Client::OnDisconnect(reason);
}
