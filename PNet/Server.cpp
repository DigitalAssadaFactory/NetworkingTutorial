#include "Server.h"


namespace PNet {

	bool Server::Init(IPEndpoint ip)
	{
		master_fd.clear();
		connections.clear();

		listeningSocket = Socket(ip.GetIPVersion());
		if (listeningSocket.Create() == PResult::P_Success)
		{
			std::cout << "Socket successfully created!\n";
			if (listeningSocket.Listen(ip) == PResult::P_Success)
			{
				WSAPOLLFD listeningSocketFD = {};
				listeningSocketFD.fd = listeningSocket.GetHandle();
				listeningSocketFD.events = POLLRDNORM;
				listeningSocketFD.revents = 0;
				master_fd.push_back(listeningSocketFD);

				std::cout << "Socket successfully listening.\n";
				return true;
			}
			else
			{
				std::cout << "Failed to listen socket.\n";
			}
			listeningSocket.Close();
		}
		else
		{
			std::cerr << "Socket creation failed.\n";
		}

		return false;
	}

	bool Server::Init(std::string ip, unsigned short port)
	{
		return Init(IPEndpoint(ip.c_str(), port));
	}

	void Server::Frame()
	{
		for (int i = 0; i < connections.size(); ++i)
		{
			if (connections[i].pm_outgoing.HasPendingPackets())
			{
				master_fd[i + 1].events = POLLRDNORM | POLLWRNORM;
			}
		}

		use_fd = master_fd;

		if (WSAPoll(use_fd.data(), use_fd.size(), 1) > 0)
		{
			WSAPOLLFD& listeningSocketFD = use_fd[0];
			if (listeningSocketFD.revents & POLLRDNORM)
			{
				Socket newConnectionSocket;
				IPEndpoint newConnectionEndpoint;
				if (listeningSocket.Accept(newConnectionSocket, &newConnectionEndpoint) == PResult::P_Success)
				{
					connections.emplace_back(TCPConnection(newConnectionSocket, newConnectionEndpoint));
					TCPConnection& acceptedConnection = connections[connections.size() - 1];
					WSAPOLLFD newConnectionFD = {};
					newConnectionFD.fd = newConnectionSocket.GetHandle();
					newConnectionFD.events = POLLRDNORM;
					newConnectionFD.revents = 0;
					master_fd.push_back(newConnectionFD);
					OnConnect(acceptedConnection);
				}
				else
				{
					std::cout << "Failed to accept new connection.\n";
				}
			}
		}

		for (int i = use_fd.size() - 1; i > 0; --i)
		{
			TCPConnection& connection = connections[i - 1];
			if (use_fd[i].revents & POLLERR)
			{
				CloseConnection(i - 1, "POLLERR");
				continue;
			}
			if (use_fd[i].revents & POLLHUP)
			{
				CloseConnection(i - 1, "POLLHUP");
				continue;
			}
			if (use_fd[i].revents & POLLNVAL)
			{
				CloseConnection(i - 1, "POLLNVAL");
				continue;
			}

			if (use_fd[i].revents & POLLRDNORM)
			{
				int bytesReceived = 0;

				if (connection.pm_incoming.currentTask == PacketManagerTask::ProcessPacketSize)
				{
					bytesReceived = recv(use_fd[i].fd, (char*)&connection.pm_incoming.currentPacketSize + connection.pm_incoming.currentExtractionOffset,
						sizeof(uint16_t) - connection.pm_incoming.currentExtractionOffset, 0);
				}
				else //Process packet
				{
					bytesReceived = recv(use_fd[i].fd, (char*)&connection.buffer + connection.pm_incoming.currentExtractionOffset,
						connection.pm_incoming.currentPacketSize - connection.pm_incoming.currentExtractionOffset, 0);
				}


				if (bytesReceived == 0)
				{
					CloseConnection(i - 1, "recv==0");
					continue;
				}
				if (bytesReceived == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					if (error != WSAEWOULDBLOCK)
					{
						CloseConnection(i - 1, "recv<0");
						continue;
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
								CloseConnection(i - 1, "Packet size too large.");
								continue;
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

			if (use_fd[i].revents & POLLWRNORM)
			{
				PacketManager& pm = connection.pm_outgoing;
				while (pm.HasPendingPackets())
				{
					if (pm.currentTask == PacketManagerTask::ProcessPacketSize)
					{
						pm.currentPacketSize = pm.Retrieve()->buffer.size();
						uint16_t bigEndianPacketSize = htons(pm.currentPacketSize);
						int bytesSent = send(use_fd[i].fd, (char*)(&bigEndianPacketSize) + pm.currentExtractionOffset,
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
						int bytesSent = send(use_fd[i].fd, bufferPtr + pm.currentExtractionOffset,
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
				if (!pm.HasPendingPackets())
				{
					master_fd[i].events = POLLRDNORM;
				}
			}
		}

		for (int i = connections.size() - 1; i >= 0; --i)
		{
			while (connections[i].pm_incoming.HasPendingPackets())
			{
				std::shared_ptr<Packet> frontPacket = connections[i].pm_incoming.Retrieve();
				if (!ProcessPacket(frontPacket))
				{
					CloseConnection(i, "Failed to process packet.");
					break;
				}
				connections[i].pm_incoming.Pop();
			}
		}
	}

	void Server::OnConnect(TCPConnection& newConnection)
	{
		std::cout << newConnection.ToString() << " - New connection accepted.\n";
	}

	void Server::OnDisconnect(TCPConnection& lostConnection, std::string reason)
	{
		std::cout << "[" << reason << "] Connection lost: " << lostConnection.ToString() << ".\n";
	}

	void Server::CloseConnection(int connectionIndex, std::string reason)
	{
		TCPConnection& connection = connections[connectionIndex];
		OnDisconnect(connection, reason);
		master_fd.erase(master_fd.begin() + connectionIndex + 1);
		use_fd.erase(use_fd.begin() + connectionIndex + 1);
		connection.Close();
		connections.erase(connections.begin() + connectionIndex);
	}

	bool Server::ProcessPacket(std::shared_ptr<Packet> packet)
	{
		std::cout << "Packet received with size: " << packet->buffer.size() << "\n";
		return true;
	}

}