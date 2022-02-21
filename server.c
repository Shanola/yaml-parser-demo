#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <yaml.h>
#include "parse.h"

int main(int argc , char *argv[])

{
    /* Parse yaml file */
	parse_state_t state;
	yaml_parser_t parser;
	yaml_event_t event;

	memset(&state, 0, sizeof(state));
    state.isclient = 0;
	state.addr = strdup("172.16.100.230");
	state.state = START;
    yaml_parser_initialize(&parser);

	FILE *fp = fopen(/*"query.yaml"*/"test.yaml", "rb");

    yaml_parser_set_input_file(&parser, fp);
	do {
	    if (yaml_parser_parse(&parser, &event) == 0) {
		    printf("yaml_parser_parse failed\n");
			return -1;
		}

		if (consume_event(&state, &event) == 0) {
		    printf("consume_event failed\n");
			return -1;
		}
		yaml_event_delete(&event);
	} while (state.state != STOP);
	
	uint16_t buf[14];
	
	for (query_t *q = state.qlist; q; q = q->next) {
	    printf("len: %1ld, %s\n",strlen(q->ip), q->ip);
	    printf("len: %1ld, %d\n", sizeof(q->port), q->port);
	    printf("%d\n", q->protocolId);
	    printf("%d\n", q->serverId);
	    printf("%d\n", q->fc);
	    printf("%d\n", q->startRegAddr);
	    printf("%d\n", q->command);

		memcpy(buf, q->ip, 16);
		memcpy(buf+8, &q->port, 2);
		memcpy(buf+9, &q->protocolId, 2);
		memcpy(buf+10, &q->serverId, 2);
		memcpy(buf+11, &q->fc, 2);
		memcpy(buf+12, &q->startRegAddr, 2);
		memcpy(buf+13, &q->command, 2);
	}


    //socket的建立
    uint16_t inputBuffer[256] = {};
    //char message[] = {0x1000};
    int sockfd = 0,forClientSockfd = 0;
    sockfd = socket(AF_INET , SOCK_STREAM , 0);

    if (sockfd == -1){
        printf("Fail to create a socket.");
    }

    //socket的連線
    struct sockaddr_in serverInfo,clientInfo;
    int addrlen = sizeof(clientInfo);
    bzero(&serverInfo,sizeof(serverInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(50000);
    bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
    listen(sockfd,5);

    while(1){
        forClientSockfd = accept(sockfd,(struct sockaddr*) &clientInfo, &addrlen);
        char *clientip = inet_ntoa(clientInfo.sin_addr);
		printf("client's ip: %s\n", clientip);
		send(forClientSockfd, buf, sizeof(buf), 0);
		//send(forClientSockfd,message,sizeof(message),0);
        /*int ret = recv(forClientSockfd, inputBuffer, sizeof(inputBuffer), 0);
        for (int i=0; i<ret; i++) {
		    printf("Get[%d]: %d\n", i, inputBuffer[i]);
		}*/
    }
    return 0;
}
