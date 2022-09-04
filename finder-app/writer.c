#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    // check if number of arguments is less than or greater than 3
    if (argc != 3)
    {
        printf("The number of arguments passed is less or greater than 3\n");
        printf("Refer /var/log/syslog for more info\n");
        openlog(NULL,0,LOG_USER);
        syslog(LOG_ERR,"Invalid number of arguments\n");
        syslog(LOG_ERR,"Usage: filename writefile writestr\n");
        syslog(LOG_ERR,"filename = writer\n");
        syslog(LOG_ERR,"writefile = full path to the file on filesystem\n");
        syslog(LOG_ERR,"writestr = text string to be written in writefile\n");
        closelog();
        return 1;
    }
    // Checking if the file or directory exist
    const char *writefile = argv[1];
    int fd = open(writefile, O_RDWR | O_CREAT, 0777);
    if (fd == -1)
    {
        openlog(NULL,0,LOG_USER);
        perror("perror returned");
        syslog(LOG_ERR,"Error opening file %s: %s\n", writefile, strerror(errno));
        closelog();
        return 1;
    }
    const char *writestr = argv[2];
    ssize_t nr;
    nr = write(fd, writestr, strlen(writestr));
    // Check if write is success
    if (nr == -1)
    {
        openlog(NULL,0,LOG_USER);
        syslog(LOG_ERR,"Unable to write to the file");
        closelog();
        return 1;
    }
    if (nr != strlen(writestr))
    {
        openlog(NULL,0,LOG_USER);
        syslog(LOG_ERR,"Partial write condition");
        closelog();
        return 1;
    }
    // writing the writestr to writefile
    openlog(NULL,0,LOG_USER);
    syslog(LOG_DEBUG,"Writing %s to %s",writestr, writefile);
    closelog();
    close(fd); // closing the opened file
}
