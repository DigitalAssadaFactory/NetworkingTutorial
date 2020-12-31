#include "Server.h"


using namespace PNet;
int main() {
	Server server;
	if (server.Init(IPEndpoint("::", 6112)))
	{
		while(true)
		{
			server.Frame();
		}
	}
	Network::Kill();
	system("pause");
	return 0;
}