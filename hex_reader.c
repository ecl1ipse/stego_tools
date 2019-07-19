#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: file_name\n");
		exit(0);
	}
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Error opening file\n");
		exit(0);
	}
	unsigned char buf[1];
	while (read(fd, buf, 1)) {
		printf("0x%02X ", (unsigned int)(buf[0] & 0xFF));
	}
}