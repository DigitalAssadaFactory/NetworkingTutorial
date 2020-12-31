#pragma once
#include "PNet/Core.h"


namespace PNet {

	class Server {
	public:
		bool Init(IPEndpoint ip);
		void Frame();

	private:
		Socket listeningSocket;
	};


}