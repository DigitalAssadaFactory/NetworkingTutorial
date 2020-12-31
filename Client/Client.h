#pragma once
#include "PNet/Core.h"


namespace PNet {

	class Client {
	public:
		bool Connect(IPEndpoint ip);
		bool IsConnected();
		bool Frame();
	private:
		bool isConnected = false;
		Socket socket;
	};

}