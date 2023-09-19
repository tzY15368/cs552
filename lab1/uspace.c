#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IOCTL_TEST _IOW(0, 6, struct ioctl_test_t)

int main () {

  /* attribute structures */
  struct ioctl_test_t {
    int field1;
    char field2;
  } ioctl_test;

  int fd = open ("/proc/ioctl_test", O_RDONLY);

  ioctl_test.field1 = 10;
  ioctl_test.field2 = 'a';

  ioctl (fd, IOCTL_TEST, &ioctl_test);

	/* test read() */
	char buf[100];
	read(fd, buf, 100);
	printf("read %s\n", buf);

  return 0;
}