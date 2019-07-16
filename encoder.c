#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define LSB 0b00000001
#define LSB0_ENCODE 0b11111110

void lsb_encode(int fd, uint8_t ch);

void lsb_encode(int fd, uint8_t ch) {
	char buf[1];
	read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	if (ch == 1) {
		//make the LSB 1
		buf[0] = buf[0] | LSB;
	} else {
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

	//using uint8_t because they have a size of 1
	for (int i = 0; i < strlen(argv[1]); i++) {
		uint8_t itr = 0b10000000;
		for (int j = 0; j < 8; j++) {
			uint8_t ch = itr & argv[1][i];
			if (ch > 0) {
				ch = 1;
			} else {
				ch = 0;
			}
			lsb_encode(fd, ch);
			itr = itr >> 1;
		}
	}
	close(fd);
}