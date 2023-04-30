#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
struct client_info {
	int sockno;
	char ip[INET_ADDRSTRLEN];
};
// creation of the structure containing the socket number and username
struct info_client{
	char name[100];
	int sock_nb;
};
struct info_client clients[100];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void send_to_all(char *msg,int curr,char *recept)
{
	int i;
	pthread_mutex_lock(&mutex);
	for(i = 0; i < n; i++) {
		if(clients[i].sock_nb != curr && strcmp(clients[i].name, recept) == 0 ) {
			if(send(clients[i].sock_nb,msg,strlen(msg),0) < 0) {
				perror("sending failure");
				continue;
			}
		}
	}
	pthread_mutex_unlock(&mutex);
}
void * thread_recv_msg(void *sock)
{
	struct client_info cl = *((struct client_info *)sock);
	char msg[500];
	char receiver[100];
	int len;
	char res[600];
	int i;
	int j;
	while((len = recv(cl.sockno,msg,500,0)) > 0) {
		msg[len] = '\0';
        char *ptr1, *ptr2;
        ptr1 = strstr(msg, ":");
        if (ptr1 != NULL) {
            ptr1 += 1;
            ptr2 = strstr(ptr1, "-");
            if (ptr2 != NULL) {
                int word_len = ptr2 - ptr1;
                strncpy(receiver, ptr1, word_len);
                receiver[word_len] = '\0';
                strcpy(ptr1, ptr2+1);
            }
        }
		send_to_all(msg, cl.sockno,receiver);
		memset(msg,'\0',sizeof(msg));
	}
	pthread_mutex_lock(&mutex);
	printf("%s disconnected\n", cl.ip);
	for(i = 0; i < n; i++) {
		if(clients[i].sock_nb == cl.sockno) {
			j = i;
			while(j < n-1) {
				clients[j] = clients[j+1];
				j++;
			}
		}
	}
	n--;
	pthread_mutex_unlock(&mutex);
}
int main(int argc,char *argv[])
{
	struct sockaddr_in my_addr,their_addr;
	int server_socket;
	int client_socket;
	socklen_t their_addr_size;
	int portno;
	char username[100];
	int ulen;
	pthread_t  thread_recv_ID; /* Thread ID from pthread_create()*/

	
	char msg[500];
	int len;
	struct client_info cl;
	char ip[INET_ADDRSTRLEN];;
	;
	if(argc > 2) {
		printf("too many arguments");
		exit(1);
	}
	portno = atoi(argv[1]);
	server_socket = socket(AF_INET,SOCK_STREAM,0);
	memset(my_addr.sin_zero,'\0',sizeof(my_addr.sin_zero));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(portno);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	
	their_addr_size = sizeof(their_addr);

	if(bind(server_socket,(struct sockaddr *)&my_addr,sizeof(my_addr)) != 0) {
		perror("binding unsuccessful");
		exit(1);
	}

	if(listen(server_socket, 5) != 0) {
		perror("listening unsuccessful");
		exit(1);
	}

	while(1) {
		if((client_socket = accept(server_socket,(struct sockaddr *)&their_addr,&their_addr_size)) < 0) {
			perror("accept unsuccessful");
			exit(1);
		}
		pthread_mutex_lock(&mutex);
		inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);
		printf("%s connected\n",ip);
		printf("New Client connected from port# %d and IP %s\n", ntohs(their_addr.sin_port), inet_ntoa(their_addr.sin_addr));
		cl.sockno = client_socket;
		strcpy(cl.ip,ip);
		//assigning socket to client
		clients[n].sock_nb = client_socket;
		//username received
		ulen=recv(client_socket, username, sizeof(username),0);
		username[ulen] = '\0';
		// socket number and username into an array of structure
		strcpy(clients[n].name, username);
		
		n++;
		pthread_create(&thread_recv_ID, NULL, thread_recv_msg, &cl); /* Create server receiver thread */
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}
