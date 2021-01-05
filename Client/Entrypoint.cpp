#include "MyClient.h"
#include <thread>

using namespace PNet;

void Run(Client* client)
{
	while (client->IsConnected())
	{
		client->Frame();
	}
}

std::string UCString(std::string s)
{
	std::string temp="";
	for (auto& c : s) temp.push_back(toupper(c));
	return temp;
}

int main() {

	if (PNet::Network::Init())
	{
		std::cout << "Network successfully initialized.\n";

		std::string ip = "";
		std::cout << "Enter IP address: ";
		std::cin >> ip;

		MyClient client;
		if (client.Connect(ip, 6112))
		{
			std::thread t(Run, &client);

			std::string msg = "";
			while (true)
			{
				std::getline(std::cin, msg);
				if (msg != "")
				{
					if (UCString(msg) == "LOGOUT")
					{
						client.Disconnect();
						break;
					}
					else
					{
						std::shared_ptr<Packet> pack = std::make_shared<Packet>(PacketType::PT_ChatMessage);
						*pack << msg;
						client.connection.pm_outgoing.Append(pack);
						msg = "";
					}
				}
			}
			t.join();
		}
		PNet::Network::Kill();
	}
	else
	{
		std::cout << "Failet to init the Network.\n";
	}
	system("pause");
	return 0;
}