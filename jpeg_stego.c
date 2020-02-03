#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

typedef unsigned char uint8_t;

void print_usage();
void encode(int fd);
void decode(int fd);
int calc_encodable_characters(int fd);
int bpb_options(char *msg, int encodable_chars, int fd);
void bit_encode(int fd, uint8_t ch, uint8_t encode_pattern);

int main(int argc, char **argv) {
    if (argc != 3) {
        print_usage();
    }

    if (!(strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "-d") == 0)) {
        print_usage();
    }

    int fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		puts("Error opening file\n");
		exit(1);
	}

    if (strcmp(argv[1], "-e") == 0) {
        encode(fd);
    } else if (strcmp(argv[1], "-d") == 0) {
        decode(fd);
    }

}

void print_usage() {
    puts("Usage to encode\n");
    puts("jpeg_stego -e name_of_image\n");
    puts("Usage to decode\n");
    puts("jpeg_stego -d name_of_jpeg\n");
    exit(1);
}

void encode(int fd) {
    int encodable_chars = calc_encodable_characters(fd);
    if (encodable_chars == 0) {
        puts("Message cannot be encoded into file, the image is likely not a jpeg\n");
        exit(1);
    }

    char *msg = malloc(encodable_chars * sizeof(char) * 8);
    printf("For the least corruption, the message must be less than %d characters\n", encodable_chars);
    printf("For more characters but more corruption, the message must be less than %d characters\n", encodable_chars * 2);
    printf("For even more characters but even more corruption, the message must be less than %d characters\n", encodable_chars * 4);
    printf("For the most characters but the most corruption, the message must be less than %d characters\n", encodable_chars * 8);
	puts("Enter the message you wnat to enter:");
	int count = 0;
	char tmp;
	while ((tmp = fgetc(stdin)) != '\n' && count < (encodable_chars * 8)) {
		msg[count] = tmp;
		count++;
	}

    int bpb = bpb_options(msg, encodable_chars, fd);
    uint8_t encode_pattern = 0;
    int ipc = 0;
    uint8_t itr_val = 0;

    if (bpb == 1) {
        encode_pattern = 0b00000001;
        ipc = 8;
        itr_val = 0b10000000;
	} else if (bpb == 2) {
		encode_pattern = 0b00000011;
		ipc = 4;
		itr_val = 0b11000000;
		encodable_chars = encodable_chars * 2;
	} else if (bpb == 4) {
		encode_pattern = 0b00001111;
		ipc = 2;
		itr_val = 0b11110000;
		encodable_chars = encodable_chars * 4;
	} else if (bpb == 8) {
		encode_pattern = 0b11111111;
		ipc = 1;
		itr_val = 0b11111111;
		encodable_chars = encodable_chars * 8;
    }

    count = 64;
	unsigned char buf[1];
    for (int i = 0; i < strlen(msg); i++) {
		uint8_t itr = itr_val;
		for (int j = 0; j < ipc; j++) {
			if (count == 64) {
				//read over the quantisation table id
				read(fd, buf, 1);
				count = 0;
				j--;
			} else {
				uint8_t ch = itr & msg[i];
				if (ipc == 8) {
					if (ch > 0) {
						ch = 1;
					} else {
						ch = 0;
					}
					bit_encode(fd, ch, encode_pattern);
					itr = itr >> 1;
				} else if (ipc == 4) {
					if (j == 0) {
						ch = ch >> 6;
					} else if  (j == 1) {
						ch = ch >> 4;
					} else if (j == 2) {
						ch = ch >>2;
					} 
					itr = itr >> 2;
					bit_encode(fd, ch, encode_pattern);
				} else if (ipc == 2) {
					if (j == 0) {
						ch = ch>>4;
					}
					itr = itr >> 4;
					bit_encode(fd, ch, encode_pattern);
				} else if (ipc == 1) {
					bit_encode(fd, ch, encode_pattern);
				}
				count++;
			}
		}
	}

	close(fd);
	puts("Message encoded");
}

void decode(int fd) {
	int qt_size = calc_encodable_characters(fd);
	qt_size /= 8;
	qt_size *= 65;
	int decode_pattern = 0;
	int bpb = 0;
	int itr = 7;
	printf("How many bits per byte do you want? 1,2,4,8?\n");
	scanf("%d", &bpb);
	if (bpb == 1) {
		decode_pattern = 0b00000001;
		itr = 7;
	} else if (bpb == 2) {
		itr = 6;
		decode_pattern = 0b00000011;
	} else if (bpb == 4) {
		itr = 4;
		decode_pattern = 0b00001111;
	} else if (bpb == 8) {
		decode_pattern = 0b11111111;
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
					c = c | temp;
					printf("%c", c);
					c = 0b00000000;
					itr = 7;
				} else {
					char temp = r_buf[0] & decode_pattern;
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
	puts("");
	close(fd);
}

int calc_encodable_characters(int fd) {
	int qt_size = 0;
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
				uint8_t size[2];
				read(fd, size, 2);
				qt_size = size[0] << 8;
				qt_size = qt_size | size[1];
				qt_size = qt_size - 2;
				nqt = qt_size/65;
				break;
			}
		}
	}
	return nqt*8;
}

int bpb_options(char *msg, int encodable_chars, int fd) {
    if (strlen(msg) > encodable_chars * 8) {
        puts("Message is too long\n");
        encode(fd);
    } 

    if (strlen(msg) < encodable_chars) {
        puts("For 1 bit per byte encoding, enter 1 (least corruption)");
    }

    if (strlen(msg) < encodable_chars * 2) {
        puts("For 2 bits per byte encoding, enter 2 (2nd least corruption)");
    }

    if (strlen(msg) < encodable_chars * 4) {
        puts("For 4 bits per byte encoding, enter 4 (3rd least corruption)");
    }

    if (strlen(msg) < encodable_chars * 8) {
        puts("For 8 bits per byte encoding, enter 8 (4th least corruption)");
    }

    int bpb = 0;
    scanf("%d", &bpb);
    if (!(((bpb == 1) || (bpb == 2) || (bpb == 4) || (bpb == 8)) && (bpb * encodable_chars > strlen(msg)))) {
        puts("Invalid option\n");
        return bpb_options(msg, encodable_chars, fd);
    }

    return bpb;
}

void bit_encode(int fd, uint8_t ch, uint8_t encode_pattern) {
	char buf[1];
	read(fd, buf, 1);
	lseek(fd, -1, SEEK_CUR);
	buf[0] = buf[0] & (~encode_pattern);
	buf[0] = buf[0] | ch;
	write(fd, buf, 1);
}