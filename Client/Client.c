#include <stdio.h>

#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "implementation.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 9009

typedef struct __login_credentials{
	
	char username_email[101], password[101];

}login_credentials;

typedef struct __new_account_credentials{
	
	char first_name[56], last_name[201];
	char username[31], email[101];
	char password[101];	

}new_account_credentials;  

short int StartConnectionToServer(int *sockfd);

void menu(const int sockfd);

int SignIn(const int sockfd);
void SignUp(const int sockfd);

int main(void){
	
	int sockfd;

	if(StartConnectionToServer(&sockfd)==-1){

		printf("[X] - Connection...ERROR!\n\n");
	
		close(sockfd);

	}else{

		printf("[V] - Connection...OK!\n\n");

		menu(sockfd);
	}

	return 0;	
}

short int StartConnectionToServer(int *sockfd){
	
	struct sockaddr_in server_addr;

	*sockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(SERVER_PORT);
	server_addr.sin_addr.s_addr=inet_addr(SERVER_ADDR);

	socklen_t server_addr_len=sizeof(server_addr);

	short int return_value=connect(*sockfd, (struct sockaddr*)&server_addr, server_addr_len);

	return return_value;
}

void menu(const int sockfd){
	
	char opc;

	do{

		printf("+======================+\n");
		printf("| Sign In..........[1] |\n");
		printf("| Sign Up..........[2] |\n");
		printf("| Exit.............[3] |\n");
		printf("+======================+\n>");

		opc=getchar();

		__fpurge(stdin);

		switch(opc){

			case '1':

				SignIn(sockfd);
				break;

			case '2':

				SignUp(sockfd);
				break;

			case '3':

				break;

			default:

				printf("Opção incorreta\n\n");
				break;
		}

	}while(opc!='3');
}

int SignIn(const int sockfd){
	
	login_credentials credentials;

	long int userID=-1;
	
	const short int request=0;

	while(userID==-1){

		printf("Username/email >");
		GetString(credentials.username_email, 101);

		printf("Password >");
		GetString(credentials.password, 101);

		if(send(sockfd, (const void*)&request, sizeof(short int), 0)==-1){

			printf("\n\n[X] Connection error...\n\n");
			break;
		}

		if(send(sockfd, (const void*)&credentials, sizeof(login_credentials), 0)==-1){

			printf("\n\n[X] Connection error...\n\n");
			break;
		}

		if(recv(sockfd, (void*)&userID, sizeof(long int), 0)==-1){

			printf("\n\n[X] Connection error...\n\n");
			break;		
		}

		if(userID==-1){

			printf("\n\n[X] Atenção: 'username/email' ou senha incorretos!\n\n");
					
		}else{

			printf("\n\n[V] Login realizado com sucesso\n\n");
		}
			
	}

	close(sockfd);

	return 0;
}

void SignUp(const int sockfd){
	
}