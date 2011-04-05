#include <winsock2.h>
#include <ws2tcpip.h>
#include "klog.h"
SOCKET ircsock;
struct sockaddr_in ircaddr;
//struct addrinfo ircaddrinfo;
//struct addrinfo *p_ircaddrinfo=&ircaddrinfo;
struct fd_set select_sock={
	1,
	{0},
};
struct timeval select_time={
	0,
	5000,
};
int sendlen;

void reset_structures() {
	select_sock.fd_count=1;
	select_sock.fd_array[0]=ircsock;
	select_time.tv_sec=0;
	select_time.tv_usec=5000;
}

void irc_connect(char* server, unsigned short port, char* nick) {
	struct WSAData data;
	WSAStartup(2, &data);
	ircsock=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//getaddrinfo("efnet.port80.se",NULL,NULL,&p_ircaddrinfo);
	//memcpy(&ircaddr.sin_addr, &p_ircaddrinfo->ai_addr, p_ircaddrinfo->ai_addrlen);
	ircaddr.sin_family=AF_INET;
	ircaddr.sin_addr.s_addr=inet_addr(server);
	ircaddr.sin_port=htons(port);
	//freeaddrinfo(p_ircaddrinfo);
	
	void* p_ircaddr = &ircaddr;
	connect(ircsock, p_ircaddr, sizeof(ircaddr));
	sendlen=sprintf(buf, "NICK %s\nUSER %s 0 * :%s\nNICK %s\n", nick, nick, nick, nick);
	send(ircsock, buf, sendlen, 0);
}

void irc_join(char* channel, char* key) {
	sendlen=sprintf(buf, "JOIN %s %s\n", channel, key);
	send(ircsock, buf, sendlen, 0);
}

void irc_send(char* channel, char* message) {
	sendlen=sprintf(buf, "PRIVMSG %s :Log: %s\n", channel, message);
	send(ircsock, buf, sendlen, 0);
}

void irc_check_incomming() {
	char *buf_offset, *offset, *nl;
	char pingmsg[]="PING ";
	char sendbuf[256];
	int i;
	reset_structures();
	//check if there is anything on the line
	while(select(0, &select_sock, NULL, NULL, &select_time)>0) {
		//clear buffer
		for(i=0;i<sizeof(buf);i++)
			buf[i]=(char)0;
		recv(ircsock, buf, sizeof(buf), 0);
		buf_offset=buf;
		offset=strstr(buf_offset, pingmsg);
		if(offset!=NULL) {
			offset+=sizeof(pingmsg)-1;
			//should have a NULL check...
			nl=strstr(offset, "\n");
			*nl=0;
			sendlen=sprintf(sendbuf, "PONG %s\n", offset);
			send(ircsock, sendbuf, sendlen, 0);
			buf_offset=nl+1;
		}
		reset_structures();
	}
}

void irc_disconnect() {
	char sendbuf[]="QUIT\n";
	send(ircsock, sendbuf, sizeof(sendbuf), 0);
	irc_check_incomming();
	closesocket(ircsock);
	WSACleanup();
}
