#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
void *thread_recv_msg(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	while((len = recv(their_sock, msg, 500, 0)) > 0) {
		msg[len] = '\0';
		fputs(msg,stdout);
		memset(msg,'\0',sizeof(msg));
	}
}
int main(int argc, char *argv[])
{
	struct sockaddr_in their_addr;
	int client_socket;
	int portno;
	pthread_t thread_recv_ID;
	char msg[500];
	char username[100];
	char res[600];
	char ip[INET_ADDRSTRLEN];
	int len;

	if(argc > 3) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[2]);
	strcpy(username,argv[1]);
	client_socket = socket(AF_INET,SOCK_STREAM,0);
	memset(their_addr.sin_zero,'\0', sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(client_socket,(struct sockaddr *)&their_addr, sizeof(their_addr)) < 0) {
		perror("connection not esatablished");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
	printf("connected to %s, start chatting\n",ip);
	printf("\n *****Before writting a message, write the name of the recipient followed by an hiphen. Exemple: recipient- message ***** \n\n");

	// send the username of the new client 
	send(client_socket,username,strlen(username),0);
	
	
	pthread_create(&thread_recv_ID, NULL,thread_recv_msg,&client_socket); /* Create server receiver thread */
	while(fgets(msg,500,stdin) > 0) {
		strcpy(res,username);
		strcat(res,":");
		strcat(res,msg);
		len = write(client_socket,res,strlen(res));
		if(len < 0) {
			perror("message not sent");
			exit(1);
		}
		memset(msg,'\0', sizeof(msg)); //memset() is used to fill a block of memory with a particular value.
		memset(res,'\0', sizeof(res));
	}
	pthread_join(thread_recv_ID, NULL);//wait for thread termination.
	close(client_socket);

}