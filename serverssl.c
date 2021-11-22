#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>	//strlen
#include <errno.h>    /*USING THE ERROR LIBRARY FOR FINDING ERRORS*/
#include <malloc.h> /*FOR MEMORY ALLOCATION */
#include <sys/socket.h> /*for creating sockets*/
#include <sys/types.h> /*for using sockets*/
#include <netinet/in.h> /* network to asii bit */
#include <resolv.h> /*server to find out the runner's IP address*/
#include "openssl/ssl.h" /*using openssl function's and certificates and configuring them*/
#include "openssl/err.h" /* helps in finding out openssl errors*/
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>	//write

#define FAIL -1 /*for error output == -1 */
#define BUFFER 1024 /*buffer for reading messages*/
#define PORT 6000

int OpenListener(int port)
{
    int socket_server;
    struct sockaddr_in server; /*creating the sockets*/
    
    //Create socket
    socket_server = socket(AF_INET, SOCK_STREAM, 0); /* setting the connection as tcp it creates endpoint for connection */
    if (socket_server == -1)
	{
		printf("Creating Socket Failed");
	}
    bzero(&server, sizeof(server));
    
    server.sin_family = AF_INET;
    server.sin_port = htons( PORT );
    server.sin_addr.s_addr = inet_addr("192.168.56.111"); //Please enter the ip address of your Server VM
    
    //Bind
    if (bind(socket_server, (struct sockaddr *)&server, sizeof(server)) != 0) /* assiging the ip address and port*/
    {
        
        puts("Bind Failed");
        close(socket_server);
        //perror("can't bind port"); /* reporting error using errno.h library */
        //abort(); /*if error will be there then abort the process */
        return 1;
    }

    //Listen
    if (listen(socket_server, 10) != 0) /*for listening to max of 10 clients in the queue*/
    {
        puts("Listen Failed");
        close(socket_server);
        //perror("Can't configure listening port"); /* reporting error using errno.h library */
        //abort(); /*if erroor will be there then abort the process */
        return 1;
    }
    return socket_server;
}

int isRoot() /*for checking if the root user is executing the server*/
{
    if (getuid() != 0)
    {
        return 0;
    }
    else
    {
        return 1; /* if root user is not executing report must be user */
    }
}

SSL_CTX *InitServerCTX(void) /*creating and setting up ssl context structure*/
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
    SSL_load_error_strings(); /* load all error messages */
    method = SSLv23_client_method(); /* create new server-method instance */
    ctx = SSL_CTX_new(method); /* create new context from method */
    if (ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void LoadCertificates(SSL_CTX *ctx, char *CertFile, char *KeyFile) /* to load a certificate into an SSL_CTX structure*/
{
    /* set the local certificate from CertFile */
    if (SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if (SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if (!SSL_CTX_check_private_key(ctx))
    {
        fprintf(stderr, "Private key does not match the public certificaten");
        abort();
    }
}

void ShowCerts(SSL *ssl) /*show the ceritficates to client and match them*/
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if (cert != NULL)
    {
        printf("Server certificates:n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Server: %sn", line); /*server certifcates*/
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("client: %sn", line); /*client certificates*/
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.n");
}

void Servlet(SSL *ssl) /* Serve the connection -- threadable */
{
    char buf[1024];
    int socket_server, bytes;
    char input[BUFFER];
    pid_t cpid;
    if (SSL_accept(ssl) == FAIL) /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else
    {
        ShowCerts(ssl); /* get any certificates */
        /*Fork system call is used to create a new process*/
        cpid = fork();
        if (cpid == 0)
        {
            while (1)
            {
                bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request and read message from server*/
                if (bytes > 0)
                {
                    buf[bytes] = 0;
                    printf("nMESSAGE FROM SERVER:%sn", buf);
                }
                else
                    ERR_print_errors_fp(stderr);
            }
        }
        else
        {
            while (1)
            {
                printf("nMESSAGE TO CLIENT:");
                fgets(input, BUFFER, stdin); /* get request and reply to client*/
                SSL_write(ssl, input, strlen(input));
            }
        }
    }
    socket_server = SSL_get_fd(ssl); /* get socket connection */
    SSL_free(ssl); /* release SSL state */
    close(socket_server); /* close connection */
}

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

int main(int count, char *strings[])
{
    SSL_CTX *ctx;
    SSL *ssl;
    struct sockaddr_in server , client;
    socklen_t sin_len = sizeof(client);
    int socket_server, socket_client;
    char *portnum;
    char buf[2048];
    //int fdimg;
    //int on = 1;

    if (!isRoot()) /* if root user is not executing server report must be root user*/
    {
        printf("This program must be run as root/sudo user!!");
        exit(0);
    }

    if (count != 2)
    {
        printf("Usage: %s n", strings[0]); /*send the usage guide if syntax of setting port is different*/
        exit(0);
    }

    /*//Create socket
    socket_server = socket(AF_INET , SOCK_STREAM , 0);
    if(socket_server < 0)
    {
        printf("Creating Socket Failed");
        return 1;
    }*/

    //setsockopt(socket_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
    SSL_library_init(); /*load encryption and hash algo's in ssl*/

    portnum = strings[1];
    ctx = InitServerCTX(); /* initialize SSL */

    LoadCertificates(ctx, "mycert.pem", "mycert.pem"); /* load certs */
    socket_server = OpenListener(atoi(portnum)); /* create server socket */

    //Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( PORT );

    /*//Bind
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
    }*/
	
    //Accept and incoming connection
	puts("------Waiting for incoming connections------");
    while(1)
    {
        socket_client = accept(socket_server, (struct sockaddr *)&client , &sin_len);
        printf("Connection: %s:%dn", inet_ntoa(server.sin_addr), ntohs(server.sin_port)); /*printing connected client information*/

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
                //close(fdimg);
            }
            else if(!strncmp(buf, "Get /doctest.jpg", 16))
            {
                //fdimg = open("doctest.jpg", O_RDONLY);
                write(socket_client, html, sizeof(html) - 1);
                //sendfile(socket_client, fdimg, NULL, 6000);
                //close(fdimg);
            }
            else
                write(socket_client, html, sizeof(html) - 1);

            close(socket_client);
            printf("\n------Client Disconnected------\n\n");
            return 0;
        }

        //parents process
        ssl = SSL_new(ctx); /* get new SSL state with context */
        SSL_set_fd(ssl, socket_client); /* set connection socket to SSL state */
        Servlet(ssl); /* service connection */
        //close(socket_server); /* close server socket */
        SSL_CTX_free(ctx); /* release context */
        close(socket_client);
    }

    return 0;
}
