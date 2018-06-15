//For fprintf, time, malloc, strncat...
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//For MySQL functions
#include "mysql.h"

//For socket manipulation
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

//Socket atributes
#define PORT 9009
#define ADDR "127.0.0.1"
#define MAX_CONNECTIONS 1001

//Database atributes (address, user, password, database, port)
#define HOST "127.0.0.1"
#define USER "root"
#define PASS "jihjo"
#define DB	 "accounts"
#define MYSQL_PORT 3306

struct __mysql{

	MYSQL mysql;
	MYSQL_ROW row;
	MYSQL_RES *res;

}mysql;

typedef struct __login_credentials{

	char username_email[101], password[101];

}login_credentials;

typedef struct __new_account_credentials{

	char first_name[56], last_name[201];
	char username[31], email[101];
	char password[101];

}new_account_credentials;

char *GetTime(void);
void WriteServerLog(short int c_log_row, short int c_log_column);
void WriteConnectionLog(short int c_log_row, short int c_log_column, const char *client_addr);

short int StartMysqlConnection(void);
short int StartServer(int *local_sockfd);
void AcceptNewConnections(const int local_sockfd);

void SignInRequest(const int client_sockfd, const char *client_addr);
void SignUpRequest(const int client_sockfd, const char *client_addr);

int main(void){

	WriteServerLog(0, 0); //Starting server...
	WriteServerLog(0, 1); //Starting connection to the database...

	int local_sockfd;

	if(StartMysqlConnection()!=-1){

		WriteServerLog(1, 0); //Successful database connection

		if(StartServer(&local_sockfd)!=-1){

			WriteServerLog(2, 0); //Listening to new connections...
			AcceptNewConnections(local_sockfd);

		}else{

			WriteServerLog(2, 1); //Listening error!
		}

	}else{

		WriteServerLog(1, 1); //Error connecting to the database!
	}

	close(local_sockfd);
	mysql_close(&mysql.mysql);

	return 0;
}

char *GetTime(void){

	time_t timer;
	struct tm *local_time;

	time(&timer);

	local_time=localtime(&timer);

	char *time_info=(char*)malloc(101*sizeof(char));

	snprintf(time_info, 101, "[%d-%d-%d %d:%d:%d] ", (local_time->tm_year+1900), local_time->tm_mon, local_time->tm_mday,
		local_time->tm_hour, local_time->tm_min, local_time->tm_sec);

	return time_info;
}

void WriteServerLog(short int c_log_row, short int c_log_column){

	FILE *file=fopen("server_logs.txt", "a");

	const char *logs_index[3][2]={{"Starting server...", "Starting connection to the database..."},
								  {"Successful database connection", "Error connecting to the database!\n"},
								  {"Listening to new connections...\n", "Listening error!"}};

	char *log=GetTime();

	strncat(log, logs_index[c_log_row][c_log_column], 101);

	fprintf(file, "%s\n", log);

	free(log);
	fclose(file);
}

void WriteConnectionLog(short int c_log_row, short int c_log_column, const char *client_addr){

	FILE *file=fopen("connection_logs.txt", "a");

	const char *logs_index[5][2]={{"Connection accepted from", "Connection error from"},
								  {"Receiving login credentials from", "Sending userID to"},
								  {"Login request from", "New account request from"},
								  {"Seding response to", "Disconnect from"},
								  {"Error desconnecting from", "---"}};

	char *log=GetTime();
	char _log[50];

	snprintf(_log, 50, "%s %s", logs_index[c_log_row][c_log_column], client_addr);

	strncat(log, _log, 50);

	fprintf(file, "%s\n", log);

	free(log);
	fclose(file);
}

short int StartMysqlConnection(void){

	short int return_value;

	mysql_init(&mysql.mysql);

	if(mysql_real_connect(&mysql.mysql, HOST, USER, PASS, DB, MYSQL_PORT, NULL, 0)){

		return_value=0;

	}else{

		return_value=-1;
	}

	return return_value;
}

short int StartServer(int *local_sockfd){

	struct sockaddr_in local_addr;

	*local_sockfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	local_addr.sin_family=AF_INET;
	local_addr.sin_port=htons(PORT);
	local_addr.sin_addr.s_addr=inet_addr(ADDR);

	bind(*local_sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

	int return_value;

	return_value=listen(*local_sockfd, MAX_CONNECTIONS);

	return return_value;
}

void AcceptNewConnections(const int local_sockfd){

	struct sockaddr_in client_addr;
	socklen_t client_addr_len=sizeof(client_addr);

	short int request;

	int client_sockfd;

	while(1){

		client_sockfd=accept(local_sockfd, (struct sockaddr*)&client_addr, &client_addr_len);

		if(client_sockfd==-1){

			WriteConnectionLog(0, 1, inet_ntoa(client_addr.sin_addr)); //Connection error from (CLIENT IP)

		}else{

			WriteConnectionLog(0, 0, inet_ntoa(client_addr.sin_addr)); //Connection accepted from (CLIENT IP)

			if(recv(client_sockfd, (void*)&request, sizeof(short int), 0)==-1){

				WriteConnectionLog(0, 1, inet_ntoa(client_addr.sin_addr)); //Connection error from (CLIENT IP)

			}else{

				if(request==0){

					WriteConnectionLog(2, 0, inet_ntoa(client_addr.sin_addr)); //Login request from (CLIENT IP)
					SignInRequest(client_sockfd, inet_ntoa(client_addr.sin_addr));

				}else{

					WriteConnectionLog(2,1, inet_ntoa(client_addr.sin_addr)); //New account request from (CLIENT IP)
					SignUpRequest(client_sockfd, inet_ntoa(client_addr.sin_addr));
				}
			}
		}

		if(close(client_sockfd)==-1){

            WriteConnectionLog(4, 0, inet_ntoa(client_addr.sin_addr)); //Error desconnecting from

		}else{

            WriteConnectionLog(3, 1, inet_ntoa(client_addr.sin_addr)); //Desconnect from
		}
	}
}

void SignInRequest(const int client_sockfd, const char *client_addr){

	login_credentials credentials;

	if(recv(client_sockfd, (void*)&credentials, sizeof(login_credentials), 0)!=-1){

		WriteConnectionLog(1, 0, client_addr); //Receiving login credentials from (CLIENT IP)

		char query[256];

		snprintf(query, 256, "select userID from users where username or email='%s' and password='%s';",
			credentials.username_email, credentials.password);

        printf("%s - %s", credentials.username_email, credentials.password);

		mysql_query(&mysql.mysql, query);

		mysql.res=mysql_store_result(&mysql.mysql);

		long int userID;

		mysql.row=mysql_fetch_row(mysql.res);

		if(mysql.row==NULL){

			userID=-1;

		}else{

			userID=atol(mysql.row[0]);
		}

		if(send(client_sockfd, (const void*)&userID, sizeof(long int), 0)!=-1){

			WriteConnectionLog(1, 1, client_addr); //Sending userID to (CLIENT IP)

		}else{

            WriteConnectionLog(0, 1, client_addr);
		}

		mysql_free_result(mysql.res);

	}else{

        WriteConnectionLog(0, 1, client_addr);
	}
}

void SignUpRequest(const int client_sockfd, const char *client_addr){

	new_account_credentials new_account;

	if(recv(client_sockfd, (void*)&new_account, sizeof(new_account_credentials), 0)==-1){

		WriteConnectionLog(1, 0, client_addr); //Receiving new account credentials from (CLIENT IP)

	}else{

		char query[550];
		short int response;

		snprintf(query, 550, "select userID from users where email='%s' or username='%s';", new_account.email, new_account.username);

		mysql_query(&mysql.mysql, query);

		mysql.res=mysql_store_result(&mysql.mysql);

		mysql.row=mysql_fetch_row(mysql.res);

		if(mysql.row!=NULL){

			if(send(client_sockfd, (const void*)&response, sizeof(long int), 0)==-1){

				WriteConnectionLog(0, 1, client_addr); //Connection error from (CLIENT IP)

			}else{

				WriteConnectionLog(3, 0, client_addr); //Seding response to (CLIENT IP)
			}

		}else{

			snprintf(query, 550, "insert into users values(DEFAULT, '%s', '%s', '%s', '%s', '%s'", new_account.first_name,
				new_account.last_name, new_account.username, new_account.email, new_account.password);

			if(mysql_query(&mysql.mysql, query)!=0){

				response=-2;

				if(send(client_sockfd, (const void*)&response, sizeof(short int), 0)==-1){

					WriteServerLog(1, 0); //Error connecting to the database!
					WriteConnectionLog(3, 0, client_addr); //Seding response to (CLIENT IP)

				}else{

					WriteConnectionLog(3, 0, client_addr); //Seding response to (CLIENT IP)
				}

			}else{

				response=0;

				if(send(client_sockfd, (const void*)&response, sizeof(short int), 0)==-1){

					WriteConnectionLog(0, 1, client_addr); //Connection error from

				}else{

					WriteConnectionLog(3, 0, client_addr); //Seding response to (CLIENT IP)
				}
			}
		}
	}
}
