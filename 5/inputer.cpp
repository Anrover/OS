#include <stdio.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <vector>
#include <stdlib.h> 
#include <string.h> 
#include <iostream>
#include <fstream>
#include <syslog.h>
#include <fcntl.h>
#include <sys/stat.h>
using namespace std;


//на случай непредвиденных обстоятельств храним число процессов
int pid_count = 0;
vector<string> command_list;
vector<int> command_type_list;
vector<pid_t> pid_list;
ifstream infile;

// запуск нашей программы в фоновом режиме
void skeleton_daemon() {
    pid_t pid;

    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    
    umask(0);
    chdir("/");

    if (int fdnull = open("/dev/null", O_RDWR)) {   
        dup2 (fdnull, STDIN_FILENO);
        dup2 (fdnull, STDOUT_FILENO);
        dup2 (fdnull, STDERR_FILENO);
        close(fdnull);
    }
    else {
        syslog(LOG_ERR, "Failed to open /dev/null");
        exit(EXIT_FAILURE);
    }
}

// считываем команды и файла конфигурации
void read_config_file() {
    if(!infile.is_open()) {
        syslog(LOG_ERR, "Failed open");
        exit(EXIT_FAILURE); 
    }
    infile.clear();
    infile.seekg(0, ios::beg);
    string line;
    while (getline(infile, line)) {
        //ищем индекс последнего пробела, чтобы разделить строку
        size_t last_space = line.rfind(" ");
        string command = line.substr(0, last_space);
        string type = line.substr(last_space + 1, line.size() - last_space + 1);

        command_list.push_back(command);

        if (type == "wait") 
            command_type_list.push_back(0);
        else if (type == "respawn")
            command_type_list.push_back(1);
        else {
            syslog(LOG_ERR, "Invalid option");
            exit(EXIT_FAILURE); 
        }

    }
}

//стартуем нашу команду
int run_command(string command) {
    double duration = 0; 
    struct timespec start, end;
    int attempts = 0;

    clock_gettime(CLOCK_MONOTONIC, &start); 

    //пытаемся запустить до тех пор, пока не будет превышено время в 5 секунд, либо пока не совершим 50 попыток
    while (duration < 5 && attempts++ < 50) {
        if (system(command.c_str()) == 0) return EXIT_SUCCESS;

        clock_gettime(CLOCK_MONOTONIC, &end);
        duration = (end.tv_sec - start.tv_sec) * 1e9; 
        duration = (duration + (end.tv_nsec - start.tv_nsec)) * 1e-9;
    }
    return EXIT_FAILURE;
}

char* generate_pidfile_name(int num) {
    char* filename = new char[30];
    sprintf(filename, "/tmp/fproc%d.pid", num);
    return filename;
}

//создаем и записываем pid в файл
void write_pid_to_file(int num) {
    ofstream outfile;
    outfile.open(generate_pidfile_name(num));
    outfile << pid_list[num] << endl;
}

//запуск нового процесса + старт команды
void launch_process(int num_command) {
    pid_t cpid = fork(); 

    switch (cpid) {
        case -1:
            syslog(LOG_ERR, "Fork failed; cpid == -1");
            exit(EXIT_FAILURE); 
        case 0:
            exit(run_command(command_list[num_command]));
        default:
            pid_list[num_command] = cpid;
            pid_count++;
            write_pid_to_file(num_command);
    }
}

//удаление pid-файла
void remove_pid_file(int num) {
    char* pidfile = generate_pidfile_name(num);
    if (access(pidfile, F_OK) != -1 && remove(pidfile) != 0)
        syslog(LOG_ERR, "Error deleting file: %s", pidfile);
}

//ожидаем завершения процессов. Если требуется, то перезапускаем
void wait_processes() {
    int status;
    pid_t cpid;

    while (pid_count) {
        cpid = waitpid(-1, &status, 0);
        for (int p = 0; p < pid_list.size(); p++) {
            if(pid_list[p] == cpid) {
                //если команда типа "wait"
                if (command_type_list[p] == 0) {
                    remove_pid_file(p);
                    if (WEXITSTATUS(status) == 0)
                        syslog(LOG_INFO, "Completed: '%s', pid = %d", command_list[p].c_str(), cpid);
                    else 
                        syslog(LOG_INFO, "Failed: '%s', pid = %d", command_list[p].c_str(), cpid);
                    pid_count--;
                } 
                else {//иначе если команда типа "respawn"
                    remove_pid_file(p);
                    //если запуск провалился, то просто уменьшаем число pid'ов
                    if (WEXITSTATUS(status) != 0) {
                        syslog(LOG_INFO, "Failed: '%s', pid = %d", command_list[p].c_str(), cpid);
                        pid_count--;
                    } else {//если же запуск был успешным, то запускаем процесс с командой по новой
                        syslog(LOG_INFO, "Completed: '%s', pid = %d. And respawn!", command_list[p].c_str(), cpid);
                        launch_process(p);
                    }
                }
            }
        }
    }
}

void init() {
    read_config_file();
    
    pid_list.resize(command_list.size());
    for (int p = 0; p < pid_list.size(); p++) {
        launch_process(p);
    }
}

//ожидаем сигнал hup
void hup_handler(int s) {
    signal(SIGHUP, hup_handler);
    syslog(LOG_INFO, "Read updated config file!");
    
    //производим остановку процессов и очистку
    for (int p = 0; p < pid_list.size(); p++) {
        kill(pid_list[p], SIGKILL);
        remove_pid_file(p);
    }
    command_list.clear();
    pid_list.clear();
    command_type_list.clear();
<<<<<<< c6bbb3a2f2d6dabf05b4cef862e3c20165434eb7
	pid_count = 0;
=======
    pid_count = 0;
>>>>>>> change daemonize

    //по новой инициализируем
    init();
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        cout << "Invalid arguments" << endl;
        return 0;
    }
    infile.open(argv[1]);

    skeleton_daemon();
    syslog(LOG_INFO, "Start");
<<<<<<< c6bbb3a2f2d6dabf05b4cef862e3c20165434eb7
	signal(SIGHUP, hup_handler);
	init();
	wait_processes();
	infile.close();
	syslog(LOG_INFO, "Successful complete!");
	return 0; 
}
=======
    signal(SIGHUP, hup_handler);
    init();
    wait_processes();
    infile.close();
    syslog(LOG_INFO, "Successful complete!");
    return 0; 
}
>>>>>>> change daemonize
