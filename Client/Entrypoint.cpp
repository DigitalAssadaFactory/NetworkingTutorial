#include "Client.h"


using namespace PNet;
int main() {
	Client client;
	if (client.Connect(IPEndpoint("::1", 6112)))
	{
		while (client.IsConnected())
		{
			client.Frame();
		}
	}
	Network::Kill();
	system("pause");
	return 0;
}