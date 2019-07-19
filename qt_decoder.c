#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define LSB 0b0000001

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: file_name\n");
		exit(1);
	}
	int fd = open(argv[1], O_RDONLY);
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
	int count = 0;
	int itr = 7;
	char c = 0b00000000;
	char r_buf[1];
	int counter = 0;
	while (read(fd, r_buf, 1)) {
		if (count == 64) {
			read(fd, r_buf, 1);
			count = 0;
		} else {
			if (itr == 0) {
				//Should have gotten a character
				char temp = r_buf[0] & LSB;
				c = c | temp;
				printf("%c", c);
				c = 0b00000000;
				itr = 7;
			} else {
				char temp = r_buf[0] & LSB;
				temp = temp<<itr;
				c = c | temp;
				itr--;
				count++;
			}
		}

		counter++;
		if (counter >= qt_size) {
			break;
		}
	}
	close(fd);

}