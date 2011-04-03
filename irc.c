#include <winsock2.h>
SOCKET ircsock;

int irc_connect(char* nick) {
	ircsock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

void irc_send(char* message) {
	
}

irc_disconnect() {
	closesocket(ircsock);
}
