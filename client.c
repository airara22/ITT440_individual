#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <string.h>

#define PORT 8080

int main(int argc , char *argv[])
{
	int socket_server;
	struct sockaddr_in server;
	//unsigned char buffer[4096];
	char *msg , server_reply[2000];
	
	//Create socket
	socket_server = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_server == -1)
	{
		printf("Creating Socket Failed");
	}
		
	server.sin_addr.s_addr = inet_addr("192.168.56.111"); //Please enter the ip address of your Server VM
	server.sin_family = AF_INET;
	server.sin_port = htons( PORT );
	//server.sin_socktype = SOCK_STREAM;

	//Connect to remote server
	if (connect(socket_server , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("Accepting Connection Failed");
		return 1;
	}

	puts("Connection from Client Accepted -client-\n");

	//Send some data
	msg = "connect";
	if( send(socket_server , "HEAD / HTTP/1.0\r\n\r\n" , strlen(msg) , 0) < 0)
	{
		puts("Send failed");
		return 1;
	}
	puts("Data Send Successfully\n");

    //Receive a reply from the server
	if( recv(socket_server, server_reply , 2000 , 0) < 0)
	{
		puts("recv failed");
	}
	puts("Reply Received Successfully!\n");
	puts(server_reply);

	return 0;
}