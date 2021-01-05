#pragma once
#include "TCPConnection.h"


namespace PNet {

	class Server {
	public:
		bool Init(IPEndpoint ip);
		bool Init(std::string ip, unsigned short port);
		void Frame();
	protected:
		virtual void OnConnect(TCPConnection & newConnection);
		virtual void OnDisconnect(TCPConnection& lostConnection, std::string reason);
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
		void CloseConnection(int connectionIndex, std::string reason);

		Socket listeningSocket;
		std::vector<TCPConnection> connections;
		std::vector<WSAPOLLFD> master_fd;
		std::vector<WSAPOLLFD> use_fd;
	};


}