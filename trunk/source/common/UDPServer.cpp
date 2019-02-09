#include "stdafx.h"

#include "UDPServer.h"
#include "log.h"
#include "Stream.h"

UDPServer::~UDPServer() {
	for (auto& itr : Streams) {
		delete itr.second;
	}
}

bool UDPServer::Open() {
	sockaddr_in address;

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(Port);
	address.sin_addr.s_addr = Host;//htonl(INADDR_ANY);

	Sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (Sock < 0) {
		LogError(LOG_NET, 0, "Failed to open socket. (%i)", WSAGetLastError());
		return false;
	}

	if (::bind(Sock, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
		_close(Sock);
		Sock = -1;
		LogError(LOG_NET, 0, "Failed to bind socket.");
		return false;
	}

#ifdef _WIN32
	unsigned long nonblock = 1;
	ioctlsocket(Sock, FIONBIO, &nonblock);
#else
	fcntl(Sock, F_SETFL, O_NONBLOCK);
#endif

	in_addr in;
	in.s_addr = Host;
	LogInfo(LOG_NET, 0, "Listening on %s:%u", inet_ntoa(in), ntohs(address.sin_port));
	return true;
}

bool UDPServer::Process() {
	fd_set readset;
	std::map<std::string, Stream*>::iterator stream_itr;
	int num;
	int length;
	unsigned char buffer[2048];
	sockaddr_in from;
	int socklen = sizeof(sockaddr_in);
	timeval sleep_time;

	FD_ZERO(&readset);
	FD_SET(Sock, &readset);

	sleep_time.tv_sec = 30;
	sleep_time.tv_usec = 0;

	if ((num = select(Sock + 1, &readset, NULL, NULL, &sleep_time)) < 0) {
		LogError(LOG_NET, 0, "select error");
		return false;
	}
	else if (num == 0)
		return true;

	if (FD_ISSET(Sock, &readset)) {
#ifdef _WIN32
		if ((length = recvfrom(Sock, (char*)buffer, 2048, 0, (struct sockaddr*)&from, (int*)&socklen)) < 0)
#else
		if ((length = recvfrom(Sock, buffer, 2048, 0, (sockaddr*)&from, (socklen_t*)&socklen)) < 0)
#endif // _WIN32
		{
			/*LogError(LOG_NET, 0, "recvfrom error (%i)", WSAGetLastError());
			return false;*/
		}
		else {
			LogError(LOG_NET, 0, "received %i", length);
			char temp[25];
			sprintf(temp, "%u.%d", ntohl(from.sin_addr.s_addr), ntohs(from.sin_port));
			LogError(LOG_NET, 0, "temp = %s", temp);
			if ((stream_itr = Streams.find(temp)) == Streams.end()) {
				LogError(LOG_NET, 0, "new stream");
				Stream* s = GetNewStream(from.sin_addr.s_addr, from.sin_port);
				s->SetServer(this);
				Streams[temp] = s;
				s->Process(buffer, length);
				//s->SetLastPacketTime();
			}
			else {
				LogError(LOG_NET, 0, "found stream");
				Stream* currentStream = stream_itr->second;

				currentStream->Process(buffer, length);
				//currentStream->SetLastPacketTime();
			}
		}
	}
	else {
		LogError(LOG_NET, 0, "FD_ISSET failed");
	}

	return true;
}

void UDPServer::StreamDisconnected(Stream* stream) {
	std::map<std::string, Stream*>::iterator stream_itr;
	char temp[25];
	sprintf(temp, "%u.%d", ntohl(stream->GetIP()), ntohs(stream->GetPort()));
	if ((stream_itr = Streams.find(temp)) != Streams.end()) {
		LogDebug(LOG_NET, 0, "Removing client.");
		Streams.erase(stream_itr);
		delete stream;
	}
	else {
		LogDebug(LOG_NET, 0, "Stream not found in disconnect.");
		if (stream)
			delete stream;
	}
}