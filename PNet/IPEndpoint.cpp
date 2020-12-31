#include "IPEndpoint.h"


namespace PNet {

	IPEndpoint::IPEndpoint(const char* ip, unsigned short port)
	{
		this->port = port;

		// IPv4
		in_addr addr;
		int result = inet_pton(AF_INET, ip, &addr);

		if (result == 1)
		{
			if (addr.S_un.S_addr != INADDR_NONE)
			{
				hostname = ip;
				ip_string = ip;
				ipVersion = IPVersion::IPv4;
				ip_bytes.resize(sizeof(ULONG));
				memcpy(ip_bytes.data(), &addr.S_un.S_addr, sizeof(ULONG));
				return;
			}
		}

		addrinfo hints = {};
		hints.ai_family = AF_INET;
		addrinfo* hostinfo = nullptr;
		result = getaddrinfo(ip, NULL, &hints, &hostinfo);
		if (result == 0)
		{
			sockaddr_in* hostaddr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

			ip_string.resize(16);
			inet_ntop(AF_INET, &hostaddr->sin_addr, &ip_string[0], 16);
			hostname = ip;

			ULONG ip_ulong = hostaddr->sin_addr.S_un.S_addr;
			ip_bytes.resize(sizeof(ULONG));
			memcpy(ip_bytes.data(), &ip_ulong, sizeof(ULONG));

			ipVersion = IPVersion::IPv4;

			freeaddrinfo(hostinfo);
			return;
		}

		// IPv6
		in6_addr addr6;
		result = inet_pton(AF_INET6, ip, &addr6);

		if (result == 1)
		{

			hostname = ip;
			ip_string = ip;
			ipVersion = IPVersion::IPv6;
			ip_bytes.resize(16);
			memcpy(ip_bytes.data(), &addr6.u, 16);
			return;
		}

		addrinfo hints6 = {};
		hints6.ai_family = AF_INET6;
		addrinfo* hostinfo6 = nullptr;
		result = getaddrinfo(ip, NULL, &hints6, &hostinfo6);
		if (result == 0)
		{
			sockaddr_in6* hostaddr6 = reinterpret_cast<sockaddr_in6*>(hostinfo6->ai_addr);

			ip_string.resize(46);
			inet_ntop(AF_INET6, &hostaddr6->sin6_addr, &ip_string[0], 46);
			hostname = ip;

			ip_bytes.resize(16);
			memcpy(ip_bytes.data(), &hostaddr6->sin6_addr, 16);

			ipVersion = IPVersion::IPv6;

			freeaddrinfo(hostinfo6);
			return;
		}

	}

	IPEndpoint::IPEndpoint(sockaddr* addr)
	{
		assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);
		if (addr->sa_family == AF_INET)
		{
			sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(addr);
			ipVersion = IPVersion::IPv4;
			port = ntohs(addr4->sin_port);
			ip_bytes.resize(sizeof(ULONG));
			memcpy(ip_bytes.data(), &addr4->sin_addr, sizeof(ULONG));
			ip_string.resize(16);
			inet_ntop(AF_INET, &addr4->sin_addr, &ip_string[0], 16);
			hostname = ip_string;
		}
		else if (addr->sa_family == AF_INET6)
		{
			sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(addr);
			ipVersion = IPVersion::IPv6;
			port = ntohs(addr6->sin6_port);
			ip_bytes.resize(16);
			memcpy(ip_bytes.data(), &addr6->sin6_addr, 16);
			ip_string.resize(46);
			inet_ntop(AF_INET6, &addr6->sin6_addr, &ip_string[0], 46);
			hostname = ip_string;
		}
	}

	void IPEndpoint::Print()
	{
		switch (ipVersion)
		{
		case IPVersion::IPv4: std::cout << "IPVersion: IPv4\n"; break;
		case IPVersion::IPv6: std::cout << "IPVersion: IPv6\n"; break;
		default: std::cout << "IPVersion: Unknown\n";
		}
		std::cout << "Hostname: " << hostname << "\n";
		std::cout << "IP: " << ip_string << "\n";
		std::cout << "Port: " << port << "\n";
		std::cout << "IP Bytes: | ";
		for (auto& digit : ip_bytes) std::cout << (int)digit << " | ";
		std::cout << "\n";
	}

	IPVersion IPEndpoint::GetIPVersion()
	{
		return ipVersion;
	}

	std::vector<uint8_t> IPEndpoint::GetIPBytes()
	{
		return ip_bytes;
	}

	std::string IPEndpoint::GetHostname()
	{
		return hostname;
	}

	std::string IPEndpoint::GetIPString()
	{
		return ip_string;
	}

	unsigned short IPEndpoint::GetPort()
	{
		return port;
	}

	sockaddr_in IPEndpoint::GetSockaddrIPv4()
	{
		assert(ipVersion == IPVersion::IPv4);
		sockaddr_in addr = {};
		addr.sin_family = AF_INET;
		memcpy(&addr.sin_addr, ip_bytes.data(), sizeof(ULONG));
		addr.sin_port = htons(port);
		return addr;
	}

	sockaddr_in6 IPEndpoint::GetSockaddrIPv6()
	{

		assert(ipVersion == IPVersion::IPv6);
		sockaddr_in6 addr = {};
		addr.sin6_family = AF_INET6;
		memcpy(&addr.sin6_addr, ip_bytes.data(), 16);
		addr.sin6_port = htons(port);
		return addr;
	}

}