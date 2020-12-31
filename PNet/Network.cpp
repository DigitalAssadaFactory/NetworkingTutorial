#include "Network.h"


namespace PNet {



	bool Network::Init()
	{
		WSADATA data;
		if (WSAStartup(MAKEWORD(2, 2), &data))
		{
			std::cerr << "Winsock init failed." << std::endl;
			return false;
		}
		if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2)
		{
			std::cerr << "No valid WINSOCK dll." << std::endl;
			return false;
		}
		return true;
	}

	void Network::Kill()
	{
	}

}