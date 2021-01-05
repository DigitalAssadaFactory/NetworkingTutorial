#include "Socket.h"


namespace PNet {

	Socket::Socket(IPVersion ip, SocketHandle h) : ipVersion(ip), handle(h)
	{
		assert(ipVersion == IPVersion::IPv4 || ipVersion == IPVersion::IPv6);
	}

	PResult Socket::Create()
	{
		assert(ipVersion == IPVersion::IPv4 || ipVersion == IPVersion::IPv6);

		if (handle != INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		handle = socket(ipVersion == IPVersion::IPv4 ? AF_INET : AF_INET6, SOCK_STREAM, IPPROTO_TCP);

		if (handle == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		if (SetBlocking(false) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		if (SetSocketOption(SocketOption::TCP_NoDelay, TRUE) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Close()
	{
		if (handle == INVALID_SOCKET)
		{
			return PResult::P_GenericError;
		}

		int result = closesocket(handle);
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		handle = INVALID_SOCKET;
		return PResult::P_Success;
	}

	PResult Socket::Bind(IPEndpoint endpoint)
	{
		assert(ipVersion == endpoint.GetIPVersion());

		if (ipVersion == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = bind(handle, (sockaddr*)&addr, sizeof(sockaddr_in));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		if (ipVersion == IPVersion::IPv6)
		{
			sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
			int result = bind(handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		return PResult::P_Success;
	}

	PResult Socket::Listen(IPEndpoint endpoint, int backlog)
	{
		if (ipVersion == IPVersion::IPv6)
		{
			if (SetSocketOption(SocketOption::IPv6_Only, FALSE) != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
		}

		if (Bind(endpoint) != PResult::P_Success)
		{
			return PResult::P_GenericError;
		}

		int result = listen(handle, backlog);
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}

		return PResult::P_Success;
	}

	PResult Socket::Accept(Socket& outSocket, IPEndpoint* endpoint)
	{
		assert(ipVersion == IPVersion::IPv4 || ipVersion == IPVersion::IPv6);

		if (ipVersion == IPVersion::IPv4)
		{
			sockaddr_in addr = {};
			int len = sizeof(sockaddr_in);
			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)&addr, &len);
			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr);
			}
			outSocket = Socket(IPVersion::IPv4, acceptedConnectionHandle);
		}
		else if (ipVersion == IPVersion::IPv6)
		{
			sockaddr_in6 addr = {};
			int len = sizeof(sockaddr_in6);
			SocketHandle acceptedConnectionHandle = accept(handle, (sockaddr*)&addr, &len);
			if (acceptedConnectionHandle == INVALID_SOCKET)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
			if (endpoint != nullptr)
			{
				*endpoint = IPEndpoint((sockaddr*)&addr);
			}
			outSocket = Socket(IPVersion::IPv6, acceptedConnectionHandle);
		}

		return PResult::P_Success;
	}

	PResult Socket::Connect(IPEndpoint endpoint)
	{
		assert(ipVersion == endpoint.GetIPVersion());

		if (ipVersion == IPVersion::IPv4)
		{
			sockaddr_in addr = endpoint.GetSockaddrIPv4();
			int result = connect(handle, (sockaddr*)&addr, sizeof(sockaddr_in));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		else if (ipVersion == IPVersion::IPv6)
		{
			sockaddr_in6 addr = endpoint.GetSockaddrIPv6();
			int result = connect(handle, (sockaddr*)&addr, sizeof(sockaddr_in6));
			if (result != 0)
			{
				int error = WSAGetLastError();
				return PResult::P_GenericError;
			}
		}
		return PResult::P_Success;
	}

	PResult Socket::Send(const void* data, int numberOfBytes, int& bytesSent)
	{
		bytesSent = send(handle, (const char*)data, numberOfBytes, NULL);

		if (bytesSent == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		return PResult::P_Success;
	}

	PResult Socket::Receive(void* dest, int numberOfBytes, int& bytesReceived)
	{
		bytesReceived = recv(handle, (char*)dest, numberOfBytes, NULL);
		if (bytesReceived == 0)
		{
			return PResult::P_GenericError;
		}
		if (bytesReceived == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		return PResult::P_Success;
	}

	PResult Socket::Send(Packet& packet)
	{
		uint16_t encodedPacketSize = htons(packet.buffer.size());
		if (SendAll(&encodedPacketSize, sizeof(encodedPacketSize)) != PResult::P_Success) return PResult::P_GenericError;
		if (SendAll(packet.buffer.data(), packet.buffer.size()) != PResult::P_Success) return PResult::P_GenericError;
		return PResult::P_Success;
	}

	PResult Socket::Receive(Packet& packet)
	{
		packet.Clear();
		uint16_t encodedSize = 0;
		if (ReceiveAll(&encodedSize, sizeof(encodedSize)) != PResult::P_Success) return PResult::P_GenericError;
		encodedSize = ntohs(encodedSize);
		if (encodedSize > g_MaxPacketSize) return PResult::P_GenericError;
		packet.buffer.resize(encodedSize);
		if (ReceiveAll(&packet.buffer[0], encodedSize) != PResult::P_Success) return PResult::P_GenericError;
		return PResult::P_Success;
	}

	PResult Socket::SendAll(const void* data, int numberOfBytes)
	{
		int totalBytesSent = 0;
		while (totalBytesSent < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesSent;
			int bytesSent = 0;
			PResult result = Send((char*)data + totalBytesSent, bytesRemaining, bytesSent);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
			totalBytesSent += bytesSent;
		}
		return PResult::P_Success;
	}

	PResult Socket::ReceiveAll(void* dest, int numberOfBytes)
	{
		int totalBytesReceived = 0;
		while (totalBytesReceived < numberOfBytes)
		{
			int bytesRemaining = numberOfBytes - totalBytesReceived;
			int bytesReceived = 0;
			PResult result = Receive((char*)dest + totalBytesReceived, bytesRemaining, bytesReceived);
			if (result != PResult::P_Success)
			{
				return PResult::P_GenericError;
			}
			totalBytesReceived += bytesReceived;
		}
		return PResult::P_Success;
	}

	SocketHandle Socket::GetHandle()
	{
		return handle;
	}

	IPVersion Socket::GetIPVersion()
	{
		return ipVersion;
	}

	PResult Socket::SetBlocking(bool isBlocking)
	{
		unsigned long nonBlocking = 1;
		unsigned long blocking = 0;
		int result = ioctlsocket(handle, FIONBIO, isBlocking ? &blocking : &nonBlocking);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		return PResult::P_Success;
	}

	PResult Socket::SetSocketOption(SocketOption option, BOOL value)
	{
		int result = 0;
		switch (option)
		{
		case SocketOption::TCP_NoDelay:
			result = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
			break;
		case SocketOption::IPv6_Only:
			result = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&value, sizeof(value));
			break;
		default:
			return PResult::P_GenericError;
		}
		if (result != 0)
		{
			int error = WSAGetLastError();
			return PResult::P_GenericError;
		}
		return PResult::P_Success;
	}

}