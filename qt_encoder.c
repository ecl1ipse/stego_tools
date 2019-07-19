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
	int temp = read(fd, buf, 1);
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
		exit(1);
	}
	int fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		printf("Error opening file\n");
		exit(1);
	}
	uint16_t qt_size = 0;
	int nqt = 0; //number of quantisation tables
	uint8_t buf[1];
	while (read(fd, buf, 1)) {
		if (buf[0] == 0xFF) {
			int brk = read(fd, buf, 1);
			if (brk == -1) {
				printf("error\n");
				exit(1);
			}
			if (buf[0] == 0xDB) {
				//found the quantisation table 
				//figure out how many characters can be encoded
				char size[2];
				read(fd, size, 2);
				qt_size = size[0] << 8;
				qt_size = qt_size | (uint8_t) size[1];
				qt_size = qt_size - 2;
				nqt = qt_size/65;
				break;
			}
	}
	}

	int encodable_characters = nqt*8;
	int count = 64;
	if (strlen(argv[1]) > encodable_characters) {
		printf("Too many characters to securely encode\n");
		exit(1);
	}
	for (int i = 0; i < strlen(argv[1]); i++) {
		uint8_t itr = 0b10000000;
		for (int j = 0; j < 8; j++) {
			if (count == 64) {
				//read over the quantisation table id
				read(fd, buf, 1);
				count = 0;
			} else {
				uint8_t ch = itr & argv[1][i];
				if (ch > 0) {
					ch = 1;
				} else {
					ch = 0;
				}
				lsb_encode(fd, ch);
				itr = itr >> 1;
				count++;
			}
		}
	}
	close(fd);

}