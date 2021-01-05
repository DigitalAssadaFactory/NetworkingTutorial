#pragma once
#include "TCPConnection.h"


namespace PNet {

	class Client {
	public:
		Client() {};
		bool Connect(IPEndpoint ip);
		bool Connect(std::string ip, unsigned short port);
		void Disconnect(std::string reason="");
		bool IsConnected();
		bool Frame();
		TCPConnection connection;
	protected:
		virtual bool ProcessPacket(std::shared_ptr<Packet> packet);
		virtual void OnConnect();
		virtual void OnFail();
		virtual void OnDisconnect(std::string reason);
		bool PollAssert(SHORT revents);
	private:
		bool isConnected = false;
		WSAPOLLFD master_fd;
		WSAPOLLFD use_fd;
	};

}