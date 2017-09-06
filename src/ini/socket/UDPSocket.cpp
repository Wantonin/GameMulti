#include "UDPSocket.hpp"

UDPSocket::UDPSocket() {

}

UDPSocket::~UDPSocket() {
	Sockets::CloseSocket(sock);
	Sockets::Release();
}

bool UDPSocket::init() {
	if (!Sockets::Start()) 
	{
		std::cout << "Erreur initialisation WinSock : " << Sockets::GetError();
		return false;
	}

	// init socket
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // ipv4, UDP
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Erreur initialisation socket : " << Sockets::GetError();
		return false;
	}

	if (!Sockets::SetNonBlocking(sock))
	{
		std::cout << "Erreur settings non bloquant : " << Sockets::GetError();
		return false;
	}

	return true;
}

bool UDPSocket::bind() {
	SOCKADDR_IN server;
	server.sin_addr.s_addr = INADDR_ANY;  // indique que toutes les sources seront accept�es
										  // UTILE: si le port est 0 il est assign� automatiquement
	server.sin_port = htons(PORT); // toujours penser � traduire le port en r�seau
	server.sin_family = AF_INET; // notre socket est UDP

	if (bind(sock, (SOCKADDR *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Erreur bind : " << Sockets::GetError();
		return false;
	}

	return true;
}

bool UDPSocket::wait() {
	fd_set rdfs;
	timeval timeout = { 0 };
	int selectRedy;

	FD_ZERO(&rdfs);

	FD_SET(sock, &rdfs);
	selectRedy = select(sock + 1, &rdfs, nullptr, nullptr, &timeout);
	if (selectRedy == -1)
	{
		std::cout << "Erreur select : " << Sockets::GetError() << std::endl;
		return -4;
	}
	else if (selectRedy > 0)
	{
		if (FD_ISSET(sock, &rdfs))
		{
			return true;
		}
	}

	return false;
}

int UDPSocket::recv(Address &sender, void *data, int size) {
	if (!this.wait())
		return -1;
	SOCKADDR_IN address = { 0 };

	/* a client is talking */
	int n = 0;
	int sinsize = sizeof *sin;

	if ((n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (SOCKADDR *)address, &sinsize)) < 0)
	{
		return -1;
	}
	buffer[n] = 0;

	uint32_t ip = ntohl(address.sin_addr.s_addr);
	uint16_t port = ntohs(address.sin_port);

	return n;
}

void UDPSocket::send(SOCKET sock, SOCKADDR_IN *sin, const char *buffer)
{
	if (sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)sin, sizeof *sin) < 0)
	{
		std::cout << "Erreur send()" << std::endl;
		return;
	}
}