#include "MyServer.h"



int main() {

	if (PNet::Network::Init())
	{
		std::cout << "Network successfully initialized.\n";
		MyServer server;
		if (server.Init("::", 6112))
		{
			while (true)
			{
				server.Frame();
			}
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