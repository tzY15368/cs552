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
    fflush(stdout);
    sleep(1);
    int i = 5;
    while (i--) {
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

        // Print the character
        printf("Received: %c\n", c);
        fflush(stdout);
    }

    // Close the device file
    close(fd);

    return 0;
}

