#include "Server.h"

/*bool ProcessPacket(Packet& packet)
{
	switch (packet.GetPacketType())
	{
	case PacketType::PT_ChatMessage:
	{
		std::string msg;
		packet >> msg;
		std::cout << "CHAT: " << msg << "\n";
		break;
	}
	case PacketType::PT_IntegerArray:
	{
		uint32_t arraysize = 0;
		packet >> arraysize;
		std::string out;
		for (int i = 0; i < arraysize; ++i)
		{
			uint32_t element;
			packet >> element;
			out += "[" + std::to_string(element) + "] ";
		}
		std::cout << out << "\n";
		break;
	}
	default:
	{
		std::cout << "Packet processing failed.\n";
		return false;
	}
	}
	return true;
}*/


namespace PNet {

	bool Server::Init(IPEndpoint ip)
	{
		if (Network::Init())
		{
			std::cout << "Proper Init!" << std::endl;

			listeningSocket = Socket(ip.GetIPVersion());
			if (listeningSocket.Create() == PResult::P_Success)
			{
				std::cout << "Socket successfully created!\n";
				if (listeningSocket.Listen(ip) == PResult::P_Success)
				{
					std::cout << "Socket successfully listening.\n";
					return true;
				}
				else
				{
					std::cout << "Failed to listen socket to port 4790.\n";
				}
				listeningSocket.Close();
			}
			else
			{
				std::cerr << "Socket creation failed.\n";
			}
		}
		return false;
	}

	void Server::Frame()
	{
		WSAPOLLFD listeningSocketFD = {};
		listeningSocketFD.fd = listeningSocket.GetHandle();
		listeningSocketFD.events = POLLRDNORM;
		listeningSocketFD.revents = 0;
		
		if (WSAPoll(&listeningSocketFD, 1, 1) > 0)
		{
			if (listeningSocketFD.revents & POLLRDNORM)
			{
				Socket newConnection;
				if (listeningSocket.Accept(newConnection) == PResult::P_Success)
				{
					std::cout << "New connection accepted.\n";
					newConnection.Close();
				}
				else
				{
					std::cout << "Failed to accept new connection.\n";
				}
			}
		}
	}

}