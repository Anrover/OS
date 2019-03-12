#include <iostream>
#include <fcntl.h>
#include <unistd.h>
using namespace std;

#define BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
	int in = STDIN_FILENO;
	if (argc != 2) {
		cout << "Invalid arguments" << endl;
		return 0;
	}
	int out = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
	if (out < 0) {
		cout << "Cannot open file to write" << endl;
		return 1;
	}
	char buffer[BUFFER_SIZE];
	bool last_zero = false;

	int len;
	do {
		len = read(in, buffer, BUFFER_SIZE);
		int from = 0;
		int read_bytes = 0;

		for (int i = 0; i < len; i++, read_bytes++) {
			if (buffer[i] != 0 && read_bytes != 0 && last_zero) { //если встретили не 0 и уже прочитали 1 байт и
				lseek(out, read_bytes, SEEK_CUR);  				  //байт до этого был 0, тогда пропускаем прочитанные байты
				from = i;
				read_bytes = 0;
				last_zero = false;
			}
			else if (buffer[i] == 0 && read_bytes != 0 && !last_zero) { //если встретили 0 и уже прочитали 1 байт и
				write(out, buffer + from, read_bytes);			  //байт до этого был не 0, тогда записываем прочитанные байты
				read_bytes = 0;
				last_zero = true;
			}
		}
		if (read_bytes != 0 && last_zero)
			lseek(out, read_bytes, SEEK_CUR);
		else if (read_bytes != 0 && !last_zero)
			write(out, buffer + from, read_bytes);
	} while (len != 0);
	
	close(out);
}
