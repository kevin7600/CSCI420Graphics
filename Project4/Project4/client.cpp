#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>
using namespace std;
struct sockaddr_in{
	int sin_family;
	int sin_port;
	int sin_adrr;
	int sin_zero[8];
};


int socket(int family, int type, int protocal){

}

int connect(int socket, int server_addr, int length){

}

int send(int socket, string message, int length,bool flag){

}
int recv(int socket, buffr, length, flag){

}

int close(int socket){

}

int main{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		ERR_EXIT("socket");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT_NUMBER);
	servaddr.sin_addr.s_addr = inet_addr(IP_ADDR);

	if (connect(sock, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0){
		ERR_EXIT("connect");
	}
	while (true){
		cout << "HI" << endl;
	}
	//communicate with server
	send(sock, sendbuf, strlen(sendbuf), 0);
	recv(sock, recvbuf, sizeof(recvbuf), 0);

	close(sock);
	return 0;
}