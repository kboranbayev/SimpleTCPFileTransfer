/* --------------------------------------------------------------------------------------------------------------------------------
 --     SOURCE FILE:    tcp_svr.c  -   A simple TCP Server File Transfer Program
 --
 --     PROGRAM:        tcp_svr.c
 --
 --     FUNCTIONS:      Berkeley Socket API, File IO
 --
 --     DATE:           October 2, 2019
 --
 --     DESIGNERS:      Aman Abdulla, Kuanysh Boranbayev, Parm Dhaliwal
 --
 --     PROGRAMMERS:    Aman Abdulla, Kuanysh Boranbayev, Parm Dhaliwal
 --
 --     NOTES:          The program will accept TCP connection from client machines.
 --                     The server will read the command sent by a client machine on default port.
 --                     There are only 2 commands: GET and SEND.
 --                     GET command represents a file transfer from the server to the client.
 --                     SEND command represents a file transfer from the client to the server.
 --                     Then, the server will prompt the client for the file name.
 --                     In case of GET command, if the server fails to find a specified file, it will send code 404 as a file content.
 --                     In case of SEND command, if the client fails to find a specified file, it will exit with code 1.
 --                     After a file transfer is completed, the server will stay open for the next client.
 ---------------------------------------------------------------------------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_TCP_PORT 	7005	// Default port
#define DATA_PORT 			7006 	// Starting port for file transfer
#define BUFLEN				80		//Buffer length
#define TRUE				1

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


/*
 Creates a socket
	return 		int 			socket
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
	int	n, bytes_to_read;
	int	sd, new_sd, client_len, port;
	struct	sockaddr_in server, client;
	char	*bp, *bp1, buf[BUFLEN], buf1[BUFLEN];

	switch(argc)
	{
		case 1:
			port = SERVER_TCP_PORT;	// Use the default port
		break;
		case 2:
			port = atoi(argv[1]);	// Get user specified port
		break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}
    sd = 0;
	// Create a stream socket
	sd = socket_create(sd);

	// Bind an address to the socket
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}

	// Listen for connections
	// queue up to 5 connect requests
	listen(sd, 5);
    printf("Server is listenning ...\n");
    
	while (TRUE)
	{
		client_len = sizeof(client);
		if ((new_sd = accept (sd, (struct sockaddr *)&client, (socklen_t *)&client_len)) == -1)
		{
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		printf(" Remote Address:  %s: %d\n", inet_ntoa(client.sin_addr), client.sin_port);
		
		bp = buf;
		bytes_to_read = BUFLEN;
		while ((n = recv (new_sd, bp, bytes_to_read, 0)) < BUFLEN)
		{
			bp += n;
			bytes_to_read -= n;
		}

		printf ("Command: %s\n", buf);
				
		if (!strcmp(buf, "GET\n")) {
			struct sockaddr_in new_server, new_client;
			int conn_fd, new_conn_fd, data_cl_len;

			// Create a stream socket
			conn_fd = socket_create(conn_fd);

			port = DATA_PORT;
			bzero((char *)&new_server, sizeof(struct sockaddr_in));
			new_server.sin_family = server.sin_family;
			new_server.sin_port = htons(port);
			new_server.sin_addr.s_addr = server.sin_addr.s_addr;
			while (bind(conn_fd, (struct sockaddr *)&new_server, sizeof(new_server)) == -1) 
			{
				printf("Port %d is busy. Trying for %d\n", port, port + 1);
				port++;
				new_server.sin_port = htons(port);
			}
			
			char selected_port[BUFLEN];
			sprintf(selected_port, "%d", port);
			printf("Server initiated port %s for file transfer\n", selected_port);
			send(new_sd, selected_port, BUFLEN, 0);
		
			printf("Client is requesting a file name ...\n");

			memset(bp1, 0, BUFLEN);
			memset(buf1, 0, BUFLEN);

			bp1 = buf1;
			bytes_to_read = BUFLEN;

			while ((n = recv (new_sd, bp1, bytes_to_read, 0)) < BUFLEN)
			{
				bp1 += n;
				bytes_to_read -= n;
			}
			printf("Filename: %s\n", buf1);

			// listen file transfer port
			if (listen(conn_fd, 5) == -1) {
				perror("ERROR: Failed to listen Port.\n");
			}
			printf("Accepting client ...\n");
			data_cl_len = sizeof(new_client);
			if ((new_conn_fd = accept(conn_fd, (struct sockaddr *)&new_client, (socklen_t *)&data_cl_len)) == -1)
			{
				fprintf(stderr, "Can't accept data connection client\n");
            	exit(1);
			};
			
			printf(" Remote Address:  %s: %d\n", inet_ntoa(new_client.sin_addr), new_client.sin_port);
			char *string = file_read(buf1);
			// send a file
			if(send(new_conn_fd, string, BUFLEN, 0)) {
				printf("File sent successfully.\n");
			}
			// closing
			close(new_conn_fd);
            close(conn_fd);
		} else if (!strcmp("SEND\n", buf)) {
			struct sockaddr_in new_server, new_client;
			int conn_fd, new_conn_fd, data_cl_len;
            conn_fd = 0;
			// Create a stream socket
			conn_fd = socket_create(conn_fd);

			port = DATA_PORT;
			bzero((char *)&new_server, sizeof(struct sockaddr_in));
			new_server.sin_family = server.sin_family;
			new_server.sin_port = htons(port);
			new_server.sin_addr.s_addr = server.sin_addr.s_addr;
			while (bind(conn_fd, (struct sockaddr *)&new_server, sizeof(new_server)) == -1) 
			{
				printf("Port %d is busy. Trying for %d\n", port, port + 1);
				port++;
				new_server.sin_port = htons(port);
			}
			
			char selected_port[BUFLEN], filename[BUFLEN];
  
			sprintf(selected_port, "%d", port);
			printf("Server initiated port %s for file transfer\n", selected_port);
			send(new_sd, selected_port, BUFLEN, 0);

			printf("Server is requesting a file name...\n");
			
			memset(bp, 0, BUFLEN);
			memset(buf, 0, BUFLEN);

			bp = filename;
			bytes_to_read = BUFLEN;

			while ((n = recv (new_sd, bp, bytes_to_read, 0)) < BUFLEN)
			{
				bp += n;
				bytes_to_read -= n;
			}

			printf("Filename: %s.\n", filename);

			// listen file transfer port
			if (listen(conn_fd, 5) == -1) {
				perror("ERROR: Failed to listen Port.\n");
			}

			data_cl_len = sizeof(new_client);
			printf("Accept a client\n");
			if ((new_conn_fd = accept(conn_fd, (struct sockaddr *)&new_client, (socklen_t *)&data_cl_len)) == -1)
			{
				fprintf(stderr, "Can't accept data connection client\n");
            	exit(1);
			};
			printf(" Remote Address:  %s: %d\n", inet_ntoa(new_client.sin_addr), new_client.sin_port);

			bp1 = buf1;
			bytes_to_read = BUFLEN;

			while ((n = recv (new_conn_fd, bp1, bytes_to_read, 0)) < BUFLEN)
     	   	{
				bp1 += n;
				bytes_to_read -= n;
        	}
			printf("File content: %s", buf1);

			file_write(filename, buf1);
			close(new_conn_fd);
			close(conn_fd);
		}
	}
    close (new_sd);
	close(sd);
	return(0);
}
