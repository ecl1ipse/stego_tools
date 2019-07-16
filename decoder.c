#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define LSB 0b0000001

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

	char buf[1];
	int itr = 7;
	char c = 0b00000000;
	while (read(fd, buf, 1)) {
		if (itr == 0) {
			//Should have gotten a character
			char temp = buf[0] & LSB;
			c = c | temp;
			printf("%c", c);
			c = 0b00000000;
			itr = 7;
		} else {
			char temp = buf[0] & LSB;
			temp = temp<<itr;
			c = c | temp;
			itr--;
		}
	}
}