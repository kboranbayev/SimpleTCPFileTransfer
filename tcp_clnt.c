/* --------------------------------------------------------------------------------------------------------------------------------
 --     SOURCE FILE:    tcp_clnt.c  -   A simple TCP Client File Transfer Program
 --
 --     PROGRAM:        tcp_clnt.c
 --
 --     FUNCTIONS:      Berkeley Socket API, File IO
 --
 --     DATE:           October 2, 2019
 --
 --     DESIGNERS:      Aman Abdulla, Kuanysh Boranbayev, Parm Dhaliwal
 --
 --     PROGRAMMERS:    Aman Abdulla, Kuanysh Boranbayev, Parm Dhaliwal
 --
 --     NOTES:          The program will establish a TCP connection to a user specified server.
 --                     The server can be specified using a fully qualified domain name or and
 --                     IP address. After the connection has been established the user will be 
 --                     prompted for a command. There are only 2 commands: GET and SEND.
 --                     GET command represents a file transfer from the server to the client.
 --                     SEND command represents a file transfer from the client to the server.
 --                     If unrecongnized command is entered, the user will be prompted for a command again.
 --                     After command is sent to the server, the server will respond with a file name request.
 --                     The user will be prompted for a file name.
 --                     In case of GET command, if the server fails to find a specified file, it will send code 404 as a file content.
 --                     In case of SEND command, if the client fails to find a specified file, it will exit with code 1.
 --                     After a file transfer is completed, the client will close the connection with the server.
 ---------------------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_TCP_PORT		7005	// Default server port
#define BUFLEN				80  	// Buffer length


/**
 *  Writes a file
 */
void file_write(char *buf, char *string)
{
	buf[strcspn(buf, "\r\t\n")] = 0;
	FILE *fp;
	fp = fopen(buf, "w");
	fprintf(fp, "%s",string);
	fclose(fp);
}


/**
 *  Reads a file
 *  @return char *
 */
char *file_read(char *filename) 
{
	filename[strcspn(filename, "\r\t\n")] = 0;
	char *string = malloc(BUFLEN);
	FILE *fp;
	if((fp = fopen(filename, "r")) == NULL)
	{
		printf("File not found!\n");
		exit(1);
	}

	fread(string, sizeof(string), 1, fp);
	
	string[strcspn(string, "\r\t\n")] = 0;
	
	fclose(fp);
	return string;
}

/**
 *  Connects to a host
 */
void connect_to_host(int socket, struct sockaddr_in server)
{
	if (connect (socket, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
}

/**
 *  Gets host by IP
 *  @return hostent *   
 */
struct hostent *get_host(char *host)
{
	struct hostent *hp;

	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	return hp;
}

/**
 *  Creates a socket
 *  @return int
 */
int socket_create(int sd)
{
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Can't create a socket");
		exit(1);
	}
	return sd;
}

/**
 *  Program start
 */
int main (int argc, char **argv)
{
	int n, bytes_to_read;
	int sd, port;
	struct hostent	*hp, *new_hp;
	struct sockaddr_in server;
	char  *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], **pptr;
	char str[16];

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}
    sd = 0;
	// Create a socket
	sd = socket_create(sd);

	// initialize client-end
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	// get host info
	hp = get_host( host);

	// Save host address
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	connect_to_host(sd, server);

	printf("Connected:    Server Name: %s:%d\n", hp->h_name, port);

	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	
	for (;;){
		printf("Command (Enter 'GET' or 'SEND'): ");
		fgets (sbuf, BUFLEN, stdin);
			
		if (!strcmp(sbuf, "SEND\n")) { // Client wants to send a file to the server
            send (sd, sbuf, BUFLEN, 0);
            
			bp = rbuf;
			bytes_to_read = BUFLEN;

			while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFLEN)
			{
				bp += n;
				bytes_to_read -= n;
			}
			printf("Server opened port %s for file transfer\n", rbuf);

			char filename[BUFLEN];
			memset(filename, 0, BUFLEN);
			printf("Filename (e.g. 'file.txt'): ");
			
			int k = 0;
			while ((filename[k++] = getchar()) != '\n') 
				;
            // sending a filename in port 7005
			write(sd, filename, BUFLEN);
			// reading a file and storing its content as a string
			char *string = file_read(filename);
            
			// setting up a new connection
			struct sockaddr_in data_conn;
			int data_conn_fd = 0;

			// establish a client connection on port 7006
			// create a new socket for data file stream
			data_conn_fd = socket_create(data_conn_fd);
            // converts string to int
			port = atoi(rbuf);
            
			bzero((char *)&data_conn, sizeof(struct sockaddr_in));
			data_conn.sin_family = server.sin_family;
			data_conn.sin_port = htons(port);

			// get a new host address
			new_hp = get_host(host);
			
			// save a new host info
			bcopy(new_hp->h_addr, (char *)&data_conn.sin_addr, new_hp->h_length);
			
            sleep(1);
			// connect to a new hostdata_conn_fd = socket_create(data_conn_fd);
			connect_to_host(data_conn_fd, data_conn);

			printf("Connected to File Transfer Port %s:%d\n", new_hp->h_name, port);

			if(send(data_conn_fd, string, BUFLEN, 0)) {
				printf("File sent successfully.\n");
			}
			printf("Closing connection on port %d\n", port);
			close(data_conn_fd);
			break; // leave for loop and end the program
		} else if (!strcmp(sbuf, "GET\n")) { // Client wants to receive a file from the server
            send (sd, sbuf, BUFLEN, 0);
			
            bp = rbuf;
			bytes_to_read = BUFLEN;

			while ((n = recv (sd, bp, bytes_to_read, 0)) < BUFLEN)
			{
				bp += n;
				bytes_to_read -= n;
			}
			printf("Server opened port '%s' for file transfer\n", rbuf);
			char filename[BUFLEN], buff[BUFLEN];
			memset(filename, 0, BUFLEN);
			printf("Filename (e.g.'file.txt'): ");
			
			int k = 0;
			while ((filename[k++] = getchar()) != '\n') 
				;
			filename[strcspn(filename, "\r\n")] = 0;
			write(sd, filename, BUFLEN);

			// setting up a new connection
			struct sockaddr_in data_conn;
			int data_conn_fd = 0;

			// establish a client connection on requested port X
			// create a new socket for data file stream
			data_conn_fd = socket_create(data_conn_fd);

			port = atoi(rbuf);
			bzero((char *)&data_conn, sizeof(struct sockaddr_in));
			data_conn.sin_family = server.sin_family;
			data_conn.sin_port = htons(port);

			// get a new host address
			new_hp = get_host(host);
			
			// save a new host info
			bcopy(new_hp->h_addr, (char *)&data_conn.sin_addr, new_hp->h_length);
			sleep(1);
			// connect to a new host
			connect_to_host(data_conn_fd, data_conn);

			printf("Connected to File Transfer Port %s:%d\n", new_hp->h_name, port);

			printf("Server is sending the file ...\n");
			
			memset(bp, 0, BUFLEN);
			memset(buff, 0, BUFLEN);

			bp = buff;
			bytes_to_read = BUFLEN;

			while ((n = recv (data_conn_fd, bp, bytes_to_read, 0)) < BUFLEN)
			{
				bp += n;
				bytes_to_read -= n;
			}

			printf("File contents: %s\n", buff);
			printf("Writing data into the file...\n");
			file_write(filename, buff);
			printf("Data is written successfully...\n");
			printf("Closing connection on port %d\n", port);
			close(data_conn_fd);
			break; // leave for loop and end the program
		} else {
			printf("Try again...\n");
		}
	}

	fflush(stdout);
	printf("Closing the connection\n");
	close(sd);
	return (0);
}
