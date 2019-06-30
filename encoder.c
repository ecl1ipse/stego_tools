#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define LSB 0b00000001
#define LSB0_ENCODE 0b11111110

void lsb_encode(int fd, char e);

void lsb_encode(int fd, char e) {
	char buf[1];
	read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	if ((int) e != 0) {
		//Make LSB 1
		buf[0] = buf[0] | LSB;
	} else {
		//Make LSB 0
		buf[0] = buf[0] & LSB0_ENCODE;
	}
	write(fd, buf, 1);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Usage: message, file_name\n");
		exit(0);
	}
	int fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		printf("Error opening file\n");
		exit(0);
	}

	//using chars because they have a size of 1
	char buf[1];
	char temp;
	for (int i = 0; i < strlen(argv[1]); i++) {
		char c = 0b10000000;
		for (int j = 0; j < 8; j++) {
			temp = argv[1][i] & c;
			lsb_encode(fd, temp);
			c = c>>1;
		}
	}
	close(fd);
}