#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc , char *argv[])
{

    //socket的建立
    int sockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線

    struct sockaddr_in info;
    bzero(&info,sizeof(info));
    info.sin_family = PF_INET;

    //localhost test
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(50000);


    int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
    if(err==-1){
        printf("Connection error");
    }


    //Send a message to server
    uint16_t message[] = {0x0001, 0x0010, 0x0100, 0x1000};
    uint16_t receiveMessage[100] = {};
    //send(sockfd,message,sizeof(message),0);
    int n = recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
	printf("recv %d bytes\n", n);
	char ip[16];
	memcpy(ip, receiveMessage, 16);
	printf("%s\n", ip);

	uint16_t port;
	memcpy(&port, receiveMessage+8, 2);
	printf("%d\n", port);

	uint16_t protocolId;
	memcpy(&protocolId, receiveMessage+9, 2);
	printf("%d\n", protocolId);

	uint16_t serverId;
	memcpy(&serverId, receiveMessage+10, 2);
	printf("%d\n", serverId);

	uint16_t fc;
	memcpy(&fc, receiveMessage+11, 2);
	printf("%d\n", fc);

	uint16_t startRegAddr;
	memcpy(&startRegAddr, receiveMessage+12, 2);
	printf("%d\n", startRegAddr);

	uint16_t command;
	memcpy(&command, receiveMessage+13, 2);
	printf("%d\n", command);


    //printf("%s",receiveMessage);
    printf("close Socket\n");
    close(sockfd);
    return 0;
}
