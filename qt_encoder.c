#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void lsb_encode(int fd, uint8_t ch, uint8_t encode_pattern);
void bit2_encode(int fd, uint8_t ch);
void bit4_encode(int fd, uint8_t ch);
void bit8_encode(int fd, uint8_t ch);

void lsb_encode(int fd, uint8_t ch, uint8_t encode_pattern) {
	char buf[1];
	int temp = read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	if (ch == 1) {
		//make the LSB 1
		buf[0] = buf[0] | encode_pattern;
	} else {
		buf[0] = buf[0] & (~encode_pattern);
	}
	write(fd, buf, 1);
}

void bit2_encode(int fd, uint8_t ch) {
	char buf[1];
	int temp = read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	buf[0] = buf[0] & 0b11111100;
	buf[0] = buf[0] | ch;
	write(fd, buf, 1);
}

void bit4_encode(int fd, uint8_t ch) {
	char buf[1];
	int temp = read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	buf[0] = buf[0] & 0b11110000;
	buf[0] = buf[0] | ch;
	write(fd, buf, 1);
}

void bit8_encode(int fd, uint8_t ch) {
	char buf[1];
	int temp = read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	buf[0] = (char) ch
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
	printf("How many bits per byte do you want encoded? 1,2,4 or 8?\n");
	printf("The more bits per byte the more image corruption but more characters can be encoded\n");
	int bpb = 0
	scanf("%d", &bpb);

	int bit_num = 0;
	scanf("%d", &bit_num);
	int encode_pattern = 0;
	if (bpb == 1) {
		printf("Which bit do you want to encode in, 1 is the most secure and 8 is the least secure\n");
		if (bit_num >= 1 && bit_num <= 8) {
			if (bit_num == 1) {
				encode_pattern = 0b00000001;
			} else if (bit_num == 2) {
				encode_pattern = 0b00000010;
			} else if (bit_num == 3) {
				encode_pattern = 0b00000100;
			} else if (bit_num == 4) {
				encode_pattern = 0b00001000;
			} else if (bit_num == 5) {
				encode_pattern = 0b00010000;
			} else if (bit_num == 6) {
				encode_pattern = 0b00100000;
			} else if (bit_num == 7) {
				encode_pattern = 0b01000000;
			} else if (bit_num == 8) {
				encode_pattern = 0b10000000;
			}
		} else {
			printf("Invalid bit number, must be between 1 and 8\n");
			exit(1);
		}
	}
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
				j--;
			} else {
				uint8_t ch = itr & argv[1][i];
				if (ch > 0) {
					ch = 1;
				} else {
					ch = 0;
				}
				lsb_encode(fd, ch, encode_pattern);
				itr = itr >> 1;
				count++;
			}
		}
	}
	close(fd);

}