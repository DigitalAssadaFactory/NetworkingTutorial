#include "Client.h"


namespace PNet {

	bool Client::Connect(IPEndpoint ip)
	{
		isConnected = false;
		if (Network::Init())
		{
			std::cout << "Success!" << std::endl;
			socket = Socket(ip.GetIPVersion());
			if (socket.Create() == PResult::P_Success)
			{
				if (socket.SetBlocking(true) != PResult::P_Success) return false;

				std::cout << "Socket successfully created!\n";
				if (socket.Connect(ip) == PResult::P_Success)
				{
					std::cout << "Successfully connected to server.\n";
					return true;
				}
				else
				{
					std::cerr << "Failed to connect.\n";
				}
				socket.Close();
			}
			else
			{
				std::cerr << "Socket creation failed.\n";
			}
		}
		return false;
	}

	bool Client::IsConnected()
	{
		return isConnected;
	}

	bool Client::Frame()
	{

		Packet pack1(PacketType::PT_ChatMessage);
		Packet pack2(PacketType::PT_IntegerArray);
		pack1 << std::string("My Message 123.");
		pack2 << 3 << 5 << 7 << 2;

		if (rand() % 2 == 0)
		{
			if (socket.Send(pack1) != PResult::P_Success) return isConnected = false;
		}
		else
		{
			if (socket.Send(pack2) != PResult::P_Success) return isConnected = false;
		}

		std::cout << "Attepting to send chunk of data...\n";
		Sleep(1000);
	}

}