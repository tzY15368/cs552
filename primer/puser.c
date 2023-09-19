#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GETCH _IOR('k', 7, char)

static char get_char_poll(int fd){
    char c;
    // Call the my_getchar ioctl to read a character
    if (ioctl(fd, IOCTL_GETCH, &c) == -1) {
        perror("ioctl failed");
        close(fd);
        return -1;
    }
    return c;
}

static char get_char_interrupt(int fd){
    char buf[2];
	if(read(fd, buf, 2) == -1){
        perror("read failed");
        close(fd);
        return -1;
    }
    return buf[0];
}

int main(int argc, char *argv[]) {
    int fd;
    char c;
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
            c = get_char_poll(fd);
        } else {
            c = get_char_interrupt(fd);
        }
        if (c == -1) {
            printf("Error reading from device\n");
            break;
        }

        // Check if it's the Enter key to exit the program
        if (c == '\n' || c == 'q') {
            break;
        }
        if (c == 127) {
            printf("\b \b");
            fflush(stdout);
            continue;
        }
        // Print the character
        // printf("Received: %c\n %d", c, c);
        printf("%c", c);
        fflush(stdout);
    }

    // Close the device file
    close(fd);

    return 0;
}

