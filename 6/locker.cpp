#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

char* generate_command(char* path, char* action) {
	const char* program = strcmp(action, "read") == 0 ? "/bin/cat " : "/bin/nano ";
	
	char* command = (char*)malloc(strlen(path) + strlen(program));
	if (command == NULL) {
        cout << "Unable to allocate memory!" << endl;
        exit(EXIT_FAILURE);
	}

	strcpy(command, program);
	strcat(command, path);
	return command;
}

char* generate_lock_filename(char* path) {
	char* name = (char*)malloc(strlen(path) + 4);
	if (name == NULL) {
        cout << "Unable to allocate memory!" << endl;
        exit(EXIT_FAILURE);
	}

	sprintf(name, "%s.lck", path);
	return name;
}

void grab_access(char* filename, char* action) {
	FILE* lockfile = fopen(filename, "w");
	fprintf(lockfile, "pid=%d	action=%s", getpid(), action);
	fclose(lockfile);
}

void unlock(char* lockfile) {
	if (access(lockfile, F_OK) != -1)
		remove(lockfile);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cout << "Must be two arguments." << endl;
		return 0;
	}
	if (strcmp(argv[1], "read") != 0 && strcmp(argv[1], "write") != 0) {
		cout << "First argument is read/write. Second argument is file name."  << endl;
		return 0;
	}
	char* command = generate_command(argv[2], argv[1]);
	char* lockfile = generate_lock_filename(argv[2]);

	while (access(lockfile, F_OK) != -1) {
		sleep(1);
	}

	grab_access(lockfile, argv[1]);
	system(command);
	unlock(lockfile);
}