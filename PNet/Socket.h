#pragma once
#include "SocketHandle.h"
#include "PResult.h"
#include "IPVersion.h"
#include <assert.h>
#include "SocketOptions.h"
#include "IPEndpoint.h"
#include "Constants.h"
#include "Packet.h"


namespace PNet {

	class Socket {
	public:
		Socket(IPVersion ip = IPVersion::IPv4,
			SocketHandle h = INVALID_SOCKET);

		PResult Create();
		PResult Close();
		PResult Bind(IPEndpoint endpoint);
		PResult Listen(IPEndpoint endpoint, int backlog = 5);
		PResult Accept(Socket& outSocket);
		PResult Connect(IPEndpoint endpoint);
		PResult Send(const void* data, int numberOfBytes, int& bytesSent);
		PResult Receive(void* dest, int numberOfBytes, int& bytesReceived);
		PResult Send(Packet& packet);
		PResult Receive(Packet& packet);
		PResult SendAll(const void* data, int numberOfBytes);
		PResult ReceiveAll(void* dest, int numberOfBytes);

		SocketHandle GetHandle();
		IPVersion GetIPVersion();
		PResult SetBlocking(bool isBlocking);
	private:
		PResult SetSocketOption(SocketOption option, BOOL value);

		IPVersion ipVersion = IPVersion::IPv4;
		SocketHandle handle = INVALID_SOCKET;
	};

}