#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_GETCH _IOR('k', 7, char)

int main() {
    int fd;
    char c;

    // Open the character device file
    fd = open("/proc/kbdev_test", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open the device");
        return 1;
    }
    printf("---");


	char buf[100];
	read(fd, buf, 100);
	printf("read %s\n", buf);
    fflush(stdout);
    exit(0);


    sleep(1);
    while (1) {
        // Call the my_getchar ioctl to read a character
        if (ioctl(fd, IOCTL_GETCH, &c) == -1) {
            perror("ioctl failed");
            close(fd);
            return 1;
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

