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
	int qt_size = 0;
	int nqt = 0; //number of quantisation tables
	unsigned char buf[1];
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
				qt_size = qt_size | (unsigned char) size[1];
				qt_size = qt_size - 2;
				nqt = qt_size/65;
				break;
			}
		}
	}
	int decode_pattern = 0;
	int bit_num = 0;
	int bpb = 0;
	int itr = 7;
	printf("How many bits per byte do you want? 1,2,4,8?\n");
	scanf("%d", &bpb);
	if (bpb == 1) {
		printf("Which bit do you want to decode in\n");
		scanf("%d", &bit_num);
		if (bit_num >= 1 && bit_num <= 8) {
			if (bit_num == 1) {
				decode_pattern = 0b00000001;
			} else if (bit_num == 2) {
				decode_pattern = 0b00000010;
			} else if (bit_num == 3) {
				decode_pattern = 0b00000100;
			} else if (bit_num == 4) {
				decode_pattern = 0b00001000;
			} else if (bit_num == 5) {
				decode_pattern = 0b00010000;
			} else if (bit_num == 6) {
				decode_pattern = 0b00100000;
			} else if (bit_num == 7) {
				decode_pattern = 0b01000000;
			} else if (bit_num == 8) {
				decode_pattern = 0b10000000;
			}
		} else {
			printf("Invalid bit number, must be between 1 and 8\n");
			exit(1);
		} 
	} else if (bpb == 2) {
		itr = 6;
		decode_pattern = 0b00000011;
	} else if (bpb == 4) {
		itr = 4;
		decode_pattern = 0b00001111;
	} else if (bpb == 8) {
		//itr is not important for 8 bpb
		itr = 0;
	} else {
		printf("invalid bits per byte. It must be either 1,2,4 or 8\n");
		exit(1);
	}
	int count = 64;
	char c = 0b00000000;
	char r_buf[1];
	int counter = 0;
	while (read(fd, r_buf, 1)) {
		if (count == 64) {
			count = 0;
			continue;
		} else {
			if (bpb == 1) {
				if (itr == 0) {
					//Should have gotten a character
					char temp = r_buf[0] & decode_pattern;
					temp = temp>>(bit_num - 1);
					c = c | temp;
					printf("%c", c);
					c = 0b00000000;
					itr = 7;
				} else {
					char temp = r_buf[0] & decode_pattern;
					temp = temp>>(bit_num - 1);
					temp = temp<<itr;
					c = c | temp;
					itr--;
				}
			} else if (bpb == 8) {
				printf("%c", r_buf[0]);
			} else if (bpb == 4) {
				if (itr == 0) {
					char temp = r_buf[0] & decode_pattern;
					c = c | temp;
					printf("%c", c);
					c = 0b00000000;
					itr = 4;
				} else {
					char temp = r_buf[0] & decode_pattern;
					temp = temp<<itr;
					itr -= 4;
					c = c | temp;
				}
			} else if (bpb == 2) {
				if (itr == 0) {
					char temp = r_buf[0] & decode_pattern;
					c = c | temp;
					printf("%c", c);
					c = 0b00000000;
					itr = 6;
				} else {
					char temp = r_buf[0] & decode_pattern;
					temp = temp<<itr;
					itr -= 2;
					c = c | temp;
				}
			}
			count++;
		}
		counter++;
		if (counter >= qt_size) {
			break;
		}
	}
	close(fd);

}