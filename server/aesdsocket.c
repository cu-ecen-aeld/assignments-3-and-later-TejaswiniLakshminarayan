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
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>
#include <net/if.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/queue.h>
#include <pthread.h>
#include <sys/time.h>


#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define PROTOCOL 0
#define PORT 9000

#define USE_AESD_CHAR_DEVICE 1
#if USE_AESD_CHAR_DEVICE
#define AESDCHARDEVICE "/dev/aesdchar"
#else
#define AESDCHARDEVICE "/var/tmp/aesdsocketdata"
#endif

int file_fd;
int sockfd;
pthread_mutex_t lock;

typedef struct node
{
    pthread_t thread;
    int clientfd;
    bool thread_complete_status;
    char client_ip[INET6_ADDRSTRLEN];
    TAILQ_ENTRY(node) entries;
}thread_data_t;

typedef TAILQ_HEAD(head_s, node) head_t;
head_t head;

/* appended timestamp to the /var/tmp/aesdsocketdata file
*  every 10s using RFC 2822 complaint strftime format,
*  followed by a newline
*/
void time_stamp()
{
    printf("starting timestamp\n");
    time_t ct;
    char time_buffer[100];
    time(&ct);
    //RFC 2822 complaint strftime format,followed by a newline.
    strftime(time_buffer, sizeof(time_buffer), "timestamp: %Y-%m-%d %H:%M:%S\r\n", localtime(&ct));
    //char *timestamp = "timestamp: ";
    // appending timestamp to the file
    //strcat(timestamp, time_buffer);
    printf("%s", time_buffer);
    lseek(file_fd, 0, SEEK_END);
    int count_bytes = strlen(time_buffer);
    //writing to the file
    int write_bytes_count = write(file_fd, time_buffer, count_bytes);
    if (write_bytes_count != count_bytes){
        printf("write unsuccessful\n");
        //perror("write unsuccessful\n");
    }
    printf("exiting timestamp\n");
}

// free memory
void free_memory()
{
    thread_data_t *temp;
    while(!TAILQ_EMPTY(&head))
    {
        temp = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, temp, entries);
        free(temp);
    }
    pthread_mutex_destroy(&lock);
    exit(0);
}

void handle_signal(int signum)
{
    printf("Caught signal %d\n", signum);
    if (signum == SIGALRM)
    {
        printf("caught SIGALRM\n");
        time_stamp();
        //unsigned int alarm (unsigned int seconds);
        alarm(10);
    }
    if ((signum == SIGINT) || (signum == SIGTERM)) {
        printf("Caught signal %d\n", signum);
        close(file_fd);
        close(sockfd);
        remove(AESDCHARDEVICE);
        free_memory();
    }
}

void * thread_function(void* thread_param)
{
    thread_data_t* data = (thread_data_t *)thread_param;
    bool packet_complete_status = false;
    bool realloc_status = false;
    char read_data, write_data;
    char *write_buffer = (char*)malloc(sizeof(char));
    int incoming_bytes = 0;
    data->thread_complete_status=false;
    
    file_fd = open(AESDCHARDEVICE, O_RDWR | O_APPEND | O_CREAT, 0777);
    if (file_fd < 0)
    {
        printf("Error opening file\n");
    }

    while(!packet_complete_status)
    {
        if(realloc_status)
        {
            write_buffer = realloc(write_buffer, incoming_bytes+1);
            realloc_status = false;
        }
        //recieve bytes
        int receive_bytes = recv(data->clientfd, &write_data, 1, 0);
        if(receive_bytes < 1)
        {
            printf("Error receiving bytes\n");
            perror("recv error\n");
        }
        if(receive_bytes == 1)
        {
            realloc_status = true;
            *(write_buffer + incoming_bytes) = write_data;
            incoming_bytes++;
        }
        if(write_data == '\n'){
            packet_complete_status = true;
        }
    }

    //lock 
    pthread_mutex_lock(&lock);

    //write    
    int write_bytes = write(file_fd, write_buffer, incoming_bytes);
    if(write_bytes != incoming_bytes)
    {
        printf("write failed\n");
        perror("write failed\n");
    } 
    printf("write success\n");       
         
    lseek(file_fd, 0, SEEK_SET);

    //read
    while(read(file_fd, &read_data, 1) != 0)
    {
        int sent_status = send(data->clientfd, &read_data, 1, 0);
        if (sent_status == 0)
        {
            printf("send failed\n");
        }
    }
    printf("send complete\n");
    packet_complete_status = false;

    //unlock
    pthread_mutex_unlock(&lock);

    //close connection
    int close_fd = close(data->clientfd);
    if(close_fd == 0){
        syslog(LOG_DEBUG, "Closed connection from %s\n", data->client_ip);
    }

    // reset and free memory
    incoming_bytes = 0;
    free(write_buffer);
    data->thread_complete_status=true;
    close(file_fd);
    return thread_param;
}

int main(int argc, char **argv) {

    // Register signal handling for SIGINT, SIGTERM and SIGALRM
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGALRM, handle_signal);
    // opens a stream socket bound to port 9000, 
    // failing and returning -1 if any of the
    // socket connection steps failed

    struct sockaddr_in server_address, client_address;
    socklen_t client_len;
    //int clientfd;

    // Open socket
    sockfd = socket(DOMAIN, TYPE, PROTOCOL);
    if (sockfd == -1) {
        perror("socket creation failed\n");
        //return -1;
    }

    // Ref : Socket Programming by Eduonix and beej.us
    memset((void *) &server_address, 0, sizeof(server_address));
    server_address.sin_family = DOMAIN;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // socket reuse
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
        perror("setsockopt error");
        exit(1);
    }

    // Bind socket
    int sockbd = bind(sockfd, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    if (sockbd == -1) {
        close(sockfd);
        perror("Error binding \n");
        //return -1;
    }

    //daemon creation if -d is present
    //daemon kill handling is pending
    if (argc > 1) {
        if (!strcmp(argv[1], "-d")) {
            //create_deamon();
            pid_t pid = fork();

            if (pid < 0) {
                printf("error:pid<0");
                exit(-1);
            } else if (pid == 0) {
                if (setsid() < 0) {
                    perror("Cannot set SID");
                    exit(-1);
                }

                if (chdir("/") == -1) {
                    perror("error changing directory");
                    exit(-1);
                }
                open("/dev/null", O_RDWR);
                dup(0);
                dup(0);
                printf("Daemon created successfully\n");
            } else {
                exit(0);
            }
        }
    }

    // file_fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_TRUNC,
    //                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

    // if (file_fd == -1) {
    //     perror("error opening the file");
    // }
    // printf("file successfully opened\n");
    
    //mutex init
    pthread_mutex_init(&lock, NULL);

    TAILQ_INIT(&head);

    int sockls;
    bool alarm_flag = false;

    printf("Listening\n");
    sockls = listen(sockfd, 5);
    printf("sockls value is %d\n", sockls);
    printf("sockfd value is %d\n", sockfd);

    if (sockls == -1) {
        perror("Error Listening \n");
        //return -1;
    }
    printf("Listening successful\n");

    while (1) {
        thread_data_t *data = (thread_data_t *) malloc(sizeof(thread_data_t));
        client_len = sizeof(client_address);
        data->clientfd = accept(sockfd, (struct sockaddr *) &client_address, &client_len);

        if (data->clientfd == -1) {
            perror("error accepting\n");
            return -1;
        }
        printf("accept success\n");
        inet_ntop(client_address.sin_family,(struct sockaddr *) &client_address, data->client_ip, sizeof(data->client_ip));

        printf("Accepted connection from %s\n", data->client_ip);
        syslog(LOG_DEBUG, "Accepted connection from %s\n", data->client_ip);  // log to syslog

        // thread create
        pthread_create(&(data->thread), NULL, &thread_function, (void *)data);

        TAILQ_INSERT_TAIL(&head, data, entries);
        data = NULL;
        free(data);

        if(!AESDCHARDEVICE) {
            if (!alarm_flag) {
                alarm_flag = true;
                printf("alarm set\n");
                alarm(10);
            }
        }

        thread_data_t *entry = NULL;
        TAILQ_FOREACH(entry, &head, entries)
        {
            printf("check for join\n");
            pthread_join(entry->thread, NULL);
            if (entry->thread_complete_status) {
                TAILQ_REMOVE(&head, entry, entries);
                free(entry);
                break;
            }
        }
        
    }
    // closing..
    close(sockfd);
    close(file_fd);
}
