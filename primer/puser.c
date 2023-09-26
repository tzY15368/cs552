#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GETCH _IOR('k', 7, char)

static char* get_char_poll(int fd, char* c_buf){
    char c;
    // Call the my_getchar ioctl to read a character
    if (ioctl(fd, IOCTL_GETCH, &c) == -1) {
        perror("ioctl failed");
        close(fd);
        return NULL;
    }
    c_buf[0] = c;
    return c_buf;
}

static char* get_char_interrupt(int fd, char* c_buf){
    char buf[10];
    int i;
	if(read(fd, buf, 10) == -1){
        perror("read failed");
        close(fd);
        return NULL;
    }
    for(i = 0; i < 10; i++){
        c_buf[i] = buf[i];
    }
    return c_buf;
}

int main(int argc, char *argv[]) {
    int fd;
    char _c[10];
    char* c = _c;
    int i;
    int use_poll = 0; // Default to using poll
    // Open the character device file
    

    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            if (strcmp(argv[i], "p") == 0) {
                use_poll = 1;
                break;
            } else if (strcmp(argv[i], "i") == 0) {
                use_poll = 0;
                break;
            }
        }
    }

	// char buf[100];
	// read(fd, buf, 100);
	// printf("read %s\n", buf);
    // fflush(stdout);
    // exit(0);

    printf("Using %s\n", use_poll ? "poll" : "interrupt");
    sleep(1);

    fd = open("/proc/ioctl_test", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open the device");
        return 1;
    }
    printf("---");
    fflush(stdout);


    while (1) {
        // read argv and see if it contains poll or interrupt
        // if poll, use poll
        // if interrupt, use interrupt
        // if nothing, use poll
        // c = get_char_poll(fd);
        
        if (use_poll) {
            c = get_char_poll(fd, c);
        } else {
            c = get_char_interrupt(fd, c);
        }
        if (c == NULL) {
            printf("Error reading from device\n");
            break;
        }

        if(c[0] == '\0'){
            continue;
        }
        if(c[1] == '\0'){
            if (c[0] == '\n' || c[0] == 'q') {
                break;
            }
            if (c[0] == 127) {
                printf("\b \b");
                fflush(stdout);
                continue;
            }
        }
        // Check if it's the Enter key to exit the program
        
        // Print the character
        // printf("Received: %c\n %d", c, c);
        printf("\033[32;1m%s\033[0m", c);
        fflush(stdout);
        // getch();
    }

    // Close the device file
    close(fd);

    return 0;
}

