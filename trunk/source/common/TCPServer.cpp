#include "stdafx.h"

#include "TCPServer.h"
#include "Stream.h"
#include "thread.h"
#include "../common/util.h"
#include "../common/log.h"

TCPServer::TCPServer(bool bHostServer) : bHost(bHostServer) {
	bLooping = false;

	streamLock.SetName("TCPServer::streamLock");
}

TCPServer::~TCPServer() {
	Close();
}

bool TCPServer::Open() {
	if (Sock != INVALID_SOCKET) {
		return true;
	}

	const char yes = 1;

	if ((Sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
		return false;
	}

	sockaddr_in address;

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(Port);
	address.sin_addr.s_addr = Host;

	if (bHost) {
		setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (::bind(Sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
			Close();
			return false;
		}
	}
	else {
		if (connect(Sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
			Close();
			LogError(LOG_NET, 0, "TCPServer : Unable to connect to %u:%u", Host, Port);
			return false;
		}

		WriteLocker lock(streamLock);
		Streams[Sock] = GetNewStream(address.sin_addr.s_addr, address.sin_port);
	}

	setsockopt(Sock, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(int));
	setsockopt(Sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int));

	if (bHost) {
		if (listen(Sock, 10) == SOCKET_ERROR) {
			Close();
			return false;
		}
	}

	bLooping = true;
	read_thread = ThreadManager::ThreadStart("TCPSelect", std::bind(&TCPServer::ReaderThread, this));

	LogDebug(LOG_NET, 0, "TCPServer : connected to %u:%u", Host, Port);

	return true;
}

bool TCPServer::Close() {
	if (Sock == INVALID_SOCKET) {
		return true;
	}

	if (bLooping) {
		bLooping = false;
		read_thread.join();
	}

	WriteLocker lock(streamLock);
	for (auto& itr : Streams) {
		itr.second->Process();
		delete itr.second;
		
		if (itr.first != Sock) {
			closesocket(itr.first);
		}
	}
	Streams.clear();

	SOCKET s = Sock;
	Sock = INVALID_SOCKET;
	closesocket(s);

	return true;
}

void TCPServer::ReaderThread() {
	SOCKET i, fd_new, fd_max = Sock;
	fd_set fds_read, fds_master;
	sockaddr_storage addr_s;
	socklen_t addr_len;
	unsigned char buf[4096];
	int count;
	timeval timeout;

	FD_ZERO(&fds_master);
	FD_ZERO(&fds_read);

	while (bLooping) {
		//Timeout of 5 seconds
		timeout.tv_usec = 0;
		timeout.tv_sec = 5;

		FD_SET(Sock, &fds_master);
		fds_read = fds_master;

		int result = select(static_cast<int>(fd_max + 1), &fds_read, NULL, NULL, &timeout);
		if (result <= 0) {
			continue;
		}

		for (i = 0; i <= fd_max; i++) {
			if (FD_ISSET(i, &fds_read)) {
				if (i == Sock && bHost) {
					addr_len = sizeof(addr_s);

					//We have a new client, so accept it.
					if ((fd_new = accept(Sock, reinterpret_cast<sockaddr*>(&addr_s), &addr_len)) == INVALID_SOCKET) {
						//LogError(LOG_TCP, 0, "Unable to accept new TCP connection: %s", Socket::SocketError().c_str());
						//break;
					}
					else {
						assert(addr_s.ss_family == AF_INET);
						const sockaddr_in& addr = *reinterpret_cast<sockaddr_in*>(&addr_s);

						//Add the client's socket to the fdset and update the max fd
						FD_SET(fd_new, &fds_master);
						if (fd_new > fd_max)
							fd_max = fd_new;

						Stream* client = GetNewStream(addr.sin_addr.s_addr, addr.sin_port);
						client->SetSocket(fd_new);
						client->SetServer(this);

						//Add the client to our streams
						WriteLocker lock(streamLock);
						Streams[fd_new] = client;
					}
				}
				else {
					//We have data from an existing client. Determine if it's a disconnect or actual data.
					WriteLocker lock(streamLock);

					auto itr = Streams.find(i);
					assert(itr != Streams.end());

					Stream* client = itr->second;

					if ((count = recv(i, reinterpret_cast<char*>(buf), sizeof(buf), 0)) <= 0) {
						//Disconnect
						FD_CLR(i, &fds_master);
						Streams.erase(itr);

						//Do one last process on this client before deletion
						client->Process();
						delete client;

						if (!bHost) {
							//Go ahead and close this connection
							ThreadManager::ThreadStart("TCPCloser", std::bind(&TCPServer::Close, this)).detach();
							return;
						}
						else {
							closesocket(i);
						}
					}
					else {
						//We have data from the client.
						client->Process(buf, count);
					}
				}
			}
		}
	}
}

bool TCPServer::Process() {
	if (Sock == INVALID_SOCKET) {
		return false;
	}

	//Run any processing on these streams now
	ReadLocker lock(streamLock);
	for (auto& itr : Streams) {
		itr.second->Process();
	}

	return true;
}