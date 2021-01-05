#include "Client.h"


namespace PNet {

	bool Client::Connect(IPEndpoint ip)
	{
		isConnected = false;

		std::cout << "Success!" << std::endl;
		Socket socket = Socket(ip.GetIPVersion());
		if (socket.Create() == PResult::P_Success)
		{
			if (socket.SetBlocking(true) != PResult::P_Success) return false;

			std::cout << "Socket successfully created!\n";
			if (socket.Connect(ip) == PResult::P_Success)
			{
				if (socket.SetBlocking(false) == PResult::P_Success)
				{
					connection = TCPConnection(socket, ip);
					master_fd.fd = connection.socket.GetHandle();
					master_fd.events = POLLRDNORM;
					master_fd.revents = 0;
					OnConnect();
					return isConnected = true;
				}
			}
			else
			{
				std::cerr << "Failed to connect.\n";
			}
			socket.Close();
		}
		else
		{
			std::cerr << "Socket creation failed.\n";
		}
		OnFail();
		return false;
	}

	bool Client::Connect(std::string ip, unsigned short port)
	{
		return Connect(IPEndpoint(ip.c_str(), port));
	}

	bool Client::IsConnected()
	{
		return isConnected;
	}

	bool Client::Frame()
	{
		if (connection.pm_outgoing.HasPendingPackets())
		{
			master_fd.events = POLLRDNORM | POLLWRNORM;
		}
		
		use_fd = master_fd;

		if (WSAPoll(&use_fd, 1, 1) > 0)
		{
			if (PollAssert(use_fd.revents)) return false;
			if (use_fd.revents & POLLRDNORM)
			{
				int bytesReceived = 0;
				if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize)
				{
					bytesReceived = recv(use_fd.fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentExtractionOffset,
						sizeof(uint16_t) - connection.pm_incoming.currentExtractionOffset, 0);
				}
				else //Process packet
				{
					bytesReceived = recv(use_fd.fd, (char*)&connection.buffer + connection.pm_incoming.currentExtractionOffset,
						connection.pm_incoming.currentPacketSize - connection.pm_incoming.currentExtractionOffset, 0);
				}

				if (bytesReceived == 0)
				{
					Disconnect("recv==0");
					return false;
				}
				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						Disconnect("recv<0");
						return false;
					}
				}

				if (bytesReceived > 0)
				{
					connection.pm_incoming.currentExtractionOffset += bytesReceived;
					if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize)
					{
						if (connection.pm_incoming.currentExtractionOffset == sizeof(uint16_t))
						{
							connection.pm_incoming.currentPacketSize = ntohs(connection.pm_incoming.currentPacketSize);
							if (connection.pm_incoming.currentPacketSize > g_MaxPacketSize)
							{
								Disconnect("Packet size too large.");
								return false;
							}
							connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketContents;
							connection.pm_incoming.currentExtractionOffset = 0;
						}
					}
					else //Process packet
					{
						if (connection.pm_incoming.currentExtractionOffset == connection.pm_incoming.currentPacketSize)
						{
							std::shared_ptr<Packet> pack = std::make_shared<Packet>();
							pack->buffer.resize(connection.pm_incoming.currentPacketSize);
							memcpy(&pack->buffer[0], connection.buffer, connection.pm_incoming.currentPacketSize);

							connection.pm_incoming.Append(pack);

							connection.pm_incoming.currentPacketSize = 0;
							connection.pm_incoming.currentExtractionOffset = 0;
							connection.pm_incoming.currentTask = PacketManagerTask::ProcessPacketSize;
						}
					}
				}
			}

			if (use_fd.revents & POLLWRNORM)
			{
				PacketManager& pm = connection.pm_outgoing;
				while (pm.HasPendingPackets())
				{
					if (pm.currentTask == PacketManagerTask::ProcessPacketSize)
					{
						pm.currentPacketSize = pm.Retrieve()->buffer.size();
						uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
						int bytesSent = send(use_fd.fd, (char*)(&bigEndianPacketSize) + pm.currentExtractionOffset,
							sizeof(uint16_t) - pm.currentExtractionOffset, 0);
						if (bytesSent > 0)
						{
							pm.currentExtractionOffset += bytesSent;
						}
						if (pm.currentExtractionOffset == sizeof(uint16_t))
						{
							pm.currentExtractionOffset = 0;
							pm.currentTask = PacketManagerTask::ProcessPacketContents;
						}
						else
						{
							break;
						}
					}
					else // Send contents
					{
						char* bufferPtr = &pm.Retrieve()->buffer[0];
						int bytesSent = send(use_fd.fd, bufferPtr + pm.currentExtractionOffset,
							pm.currentPacketSize - pm.currentExtractionOffset, 0);
						if (bytesSent > 0)
						{
							pm.currentExtractionOffset += bytesSent;
						}
						if (pm.currentExtractionOffset == pm.currentPacketSize)
						{
							pm.currentExtractionOffset = 0;
							pm.currentTask = PacketManagerTask::ProcessPacketSize;
							pm.Pop();
						}
						else
						{
							break;
						}
					}
				}
				if (!connection.pm_outgoing.HasPendingPackets())
				{
					master_fd.events = POLLRDNORM;
				}
			}


			while (connection.pm_incoming.HasPendingPackets())
			{
				std::shared_ptr<Packet> frontPacket = connection.pm_incoming.Retrieve();
				if (!ProcessPacket(frontPacket))
				{
					Disconnect("Failed to process packet.");
					return false;
				}
				connection.pm_incoming.Pop();
			}
		}
	}

	bool Client::ProcessPacket(std::shared_ptr<Packet> packet)
	{
		std::cout << "Packet received with size: " << packet->buffer.size() << "\n";
		return true;
	}

	void Client::OnConnect()
	{
		std::cout << "Successfully connected.\n";
	}

	void Client::OnFail()
	{
		std::cout << "Failed to connect.\n";
	}

	void Client::OnDisconnect(std::string reason)
	{
		std::cout << "Disconnected. " << reason << "\n";
	}

	bool Client::PollAssert(SHORT revents)
	{
		if (revents & POLLERR)
		{
			Disconnect("POLLERR");
			return true;
		}
		if (revents & POLLHUP)
		{
			Disconnect("POLLHUP");
			return true;
		}
		if (revents & POLLNVAL)
		{
			Disconnect("POLLNVAL");
			return true;
		}
		return false;
	}

	void Client::Disconnect(std::string reason)
	{
		OnDisconnect(reason);
		master_fd.fd = 0;
		connection.Close();
		isConnected = false;
	}

}