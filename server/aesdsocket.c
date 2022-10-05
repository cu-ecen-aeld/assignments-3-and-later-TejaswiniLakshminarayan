/*
 * Author: Tejaswini Lakshminarayan
 * Socket based program for Assignment 5 Part 1
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>
#include <net/if.h>
#include <sys/stat.h>


#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL 0
#define PORT 9000
#define BUFFER_SIZE 1024


int file_fd;
int sockfd;

void handle_signal(int signum)
{
    printf("Caught signal %d", signum);
    if ((signum == SIGINT) || (signum == SIGTERM)) {
        close(file_fd);
        close(sockfd);
        remove("/var/tmp/aesdsocketdata");
        exit(0);
    }
}

int main(int argc, char **argv)
{

   // Register signal handling for SIGINT and SIGTERM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    // opens a stream socket bound to port 9000, 
    // failing and returning -1 if any of the
    // socket connection steps failed
    int receive_bytes;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len;
    int clientfd;
    
    // Open socket
    sockfd = socket(DOMAIN, TYPE, PROTOCOL);
    if (sockfd == -1)
    {
        perror("socket creation failed\n");
        return -1;
    }
    
    // Ref : Socket Programming by Eduonix and beej.us
    memset((void *)&server_address, 0, sizeof(server_address));
    server_address.sin_family = DOMAIN;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int yes=1;
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) 
    {
    	perror("setsockopt error");
    	exit(1);
    }


    // Bind socket
    int sockbd = bind(sockfd, (struct sockaddr *)&server_address, sizeof(struct sockaddr));
    if (sockbd == -1)
    {
        close(sockfd);
        perror("Error binding \n");
        return -1;
    }

    //daemon creation if -d is present
    //daemon kill handling is pending
    if(argc > 1)
    {
        if(!strcmp(argv[1],"-d"))
        {
            //create_deamon();
            pid_t pid = fork();

    		if (pid < 0) 
    		{
    			printf("error:pid<0");
        		exit (-1);
    		}
    		else if (pid == 0)
    		{
    			if (setsid() < 0) 
    			{
        			perror("Cannot set SID");
        			exit (-1);
        		}
    		
    			if(chdir("/") == -1)
    			{
    				perror("error changing directory");
    				exit (-1);
    			} 
    			open("/dev/null", O_RDWR);
    			dup(0);
    			dup(0);
    			printf("Daemon created successfully\n");
    			//exit (0);
    		}
            else {
                exit (0);
            }
        }
    }

    printf("Listening\n");
    int sockls;
    sockls = listen(sockfd, 5);
    printf("Listening successful\n");

    if (sockls == -1)
    {
        perror("Error Listening \n");
        return -1;
    }

    printf("opening /var/tmp/aesdsocketdata\n");
    file_fd = open("/var/tmp/aesdsocketdata",O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP |S_IWGRP | S_IROTH | S_IWOTH);
    
    if (file_fd == -1)
    {
        perror("error opening the file");
    }
    printf("file successfully opened\n");
    int file_size = 0;
    
    char receive_buffer[BUFFER_SIZE];
    
    while(1)
    {
        client_len = sizeof(client_address);
        clientfd = accept(sockfd, (struct sockaddr *)&client_address, &client_len);
        if (clientfd == -1)
        {
            perror("error accepting\n");
            return -1;
        }
        printf("Accepted connection from %s\n",inet_ntoa(client_address.sin_addr));
        syslog(LOG_DEBUG,"Accepted connection from %s\n",inet_ntoa(client_address.sin_addr));  // log to syslog

        bool packet_complete_status = false;
        int buffer = BUFFER_SIZE;
        int running_buffer_size = 0;
        char *write_buffer = malloc(sizeof(char) * buffer);

        while(!packet_complete_status)
        {
            receive_bytes = recv(clientfd, receive_buffer, BUFFER_SIZE, 0);
            if (receive_bytes == 0)
            {
                packet_complete_status = true;
            } 
            for (int i = 0; i < receive_bytes; i++)
            {
                //printf("recieved byte is %d",receive_buffer[i]);
                if (receive_buffer[i] == '\n')
                {
                    packet_complete_status = true;
                }
            }
            if ((buffer - running_buffer_size) < receive_bytes)
            {
                buffer += receive_bytes;
                write_buffer = (char *) realloc(write_buffer, sizeof(char) * buffer);
            }
            memcpy(write_buffer + running_buffer_size, receive_buffer, receive_bytes);
            running_buffer_size += receive_bytes;
        }
        
        if(packet_complete_status)
        {
            packet_complete_status = false;
            // write to file
            int write_bytes = write(file_fd, write_buffer, running_buffer_size);
            if (write_bytes == -1)
            {
                perror("error writing to file");
                return -1;
            }
            file_size += write_bytes;

            lseek(file_fd, 0, SEEK_SET);

            char *read_buffer = (char *) malloc(sizeof(char) * file_size);

            // read
            ssize_t read_bytes = read(file_fd, read_buffer, file_size);
            if (read_bytes == -1)
            {
                perror("error reading from file");
                return -1;
            }

            int send_bytes = send(clientfd, read_buffer, read_bytes, 0);
            if (send_bytes == -1)
            {
                perror("error sending to client");
                return -1;
            }

            // free the malloc buffer
            free(read_buffer);
            free(write_buffer);

        }
        close(clientfd);
        syslog(LOG_DEBUG,"Closed connection from %s\n",inet_ntoa(client_address.sin_addr));  // log to syslog
    }
    
    close(sockbd);
    return 0;
}
