#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>	//strlen
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write

#define PORT 8080

char html[] = 
"HTTP/1.1 200 OK \r\n"
"Server: PGWebServ v0.1 \n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><body><h1>  ITT440 Individual Assignment  </h1>\n"
"<p>  Puteri Aira Aifa binti Kamarulsahar (2021155847)  </p>"
"\n<p>  CS2554A  </p>\n"
"Hi, my name is Airara, nice to meet you!\n\n"
"</body></html>"
"-----Connection: CLOSED-----\r\n\r\n";

int main(int argc , char *argv[])
{
    struct sockaddr_in server , client;
    socklen_t sin_len = sizeof(client);
    int socket_server, socket_client;
    char buf[2048];
    int fdimg;
    int on = 1;

    //Create socket
    socket_server = socket(AF_INET , SOCK_STREAM , 0);
    if(socket_server < 0)
    {
        printf("Creating Socket Failed");
        return 1;
    }

    setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    //Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

    //Bind
	if( bind(socket_server,(struct sockaddr *)&server , sizeof(server)) == -1)
	{
		puts("Bind Failed");
        close(socket_server);
        return 1;
	}
	puts("Bind SUCCESS\n");

    //Listen
    if( listen(socket_server , 10) == -1)
    {
        puts("Listen Failed");
        close(socket_server);
        return 1;
    }
	
    //Accept and incoming connection
	puts("------Waiting for incoming connections------");
    while(1)
    {
        socket_client = accept(socket_server, (struct sockaddr *)&client , &sin_len);
        if(socket_client == -1)
        {
            perror("Accepting Connection Failed");
        }  
	    
        puts("\nConnection Accepted -server-\n");

        if(!fork())
        {
            //child process
            close(socket_server);
            memset(buf, 0, 2048);
            read(socket_client, buf, 2047);

            printf("%s\n", buf);

            if(!strncmp(buf, "Get /favicon.ico", 16))
            {
                //fdimg = open("favicon.ico", O_RDONLY);
                write(socket_client, html, sizeof(html) - 1);
                //sendfile(socket_client, fdimg, NULL, 4000);
                close(fdimg);
            }
            else if(!strncmp(buf, "Get /doctest.jpg", 16))
            {
                //fdimg = open("doctest.jpg", O_RDONLY);
                write(socket_client, html, sizeof(html) - 1);
                //sendfile(socket_client, fdimg, NULL, 6000);
                close(fdimg);
            }
            else
                write(socket_client, html, sizeof(html) - 1);

            close(socket_client);
            printf("\n------Client Disconnected------\n\n");
            return 0;
        }

        //parents process
        close(socket_client);
    }

    return 0;
}
