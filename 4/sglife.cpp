#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <iostream>
#include <fstream>
#include <pthread.h>
using namespace std;
#define PORT 8080

//класс gameOfLife
class g_life { 
public: 
    int width;
    int height;
    int **map;
  

    g_life(int w, int h, int **m) { 
        width = w;
        height = h;
        map = m;
        nmap = new int * [height];
        for (int i = 0; i < height; i++) {
            nmap[i] = new int [width];
        }
    }

    // очередная итерация для изменения карты
    void update_generation() {
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) {
                nmap[y][x] = is_alive(x, y) ? 1 : 0;
            }
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++) 
                map[y][x] = nmap[y][x];
    }

    //возвращает строковое представление карты
    string get_str_map() {
        string str_map = "";
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                str_map += map[y][x] == 0 ? " " : "*";
            }
            str_map += "\n";
        }
        return str_map;
    }

private:
    int **nmap;

    //проверка на то, что клетка жива, либо станет живой
    bool is_alive(int x, int y) {
        int alive_count = 0;
        for (int dy = -1; dy < 2; dy++)
            for (int dx = -1; dx < 2; dx++) {
                if (dx == 0 && dy == 0)
                    continue;
                int nx = (x + dx + width) % width;
                int ny = (y + dy + height) % height;
                alive_count += map[ny][nx];
            }
        return alive_count == 3 || map[y][x] == 1 && alive_count == 2;
    }
}; 

//cчитаем карту из файла 
g_life read_gmap(ifstream &file) {
    string line;
    int width;
    int height;
    file >> height;
    file >> width;
    getline(file, line);
    int **map;

    map = new int * [height];
    for (int i = 0; i < height; i++) {
        map[i] = new int [width];
    }

    for (int y = 0; y < height; y++) {
        getline(file, line);
        for (int x = 0; x < width; x++)
            map[y][x] = line[x] == '0' ? 0 : 1;
    }
    g_life life(width, height, map);
    return life;
}

//основной метод-поток для генерации карты игры Жизнь. Также определяет превышение лимита времени
void *game_thread(void *arg) {
    g_life glife = *(g_life*)arg;
    double duration; 
    struct timespec start, end; 
    while (true) {
        // system ("clear");
        // printf("-----\n%s-----\n", glife.get_str_map().c_str());
        clock_gettime(CLOCK_MONOTONIC, &start); 

        //генерируем новое состояние
        glife.update_generation();

        clock_gettime(CLOCK_MONOTONIC, &end);
        duration = (end.tv_sec - start.tv_sec) * 1e9; 
        duration = (duration + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        if (duration > 1)
            cout << "Slow generation. Duration = " << duration << endl;
        else
            usleep((1 - duration) * 1e6);
    }
}

//основной метод-поток, который исполняет роль сервера. Передаёт текущее состояние игры клиентам при подключении.
void *server_thread(void *arg) {
    g_life glife = *(g_life*)arg;
    int server_fd, sock, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    if (listen(server_fd, 3) < 0) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    
    while (true) {
        if ((sock = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) {
            perror("accept"); 
            exit(EXIT_FAILURE); 
        }
        string str_map = glife.get_str_map();
        send(sock, str_map.c_str(), str_map.size(), 0);
        close(sock);
    }
}

int main(int argc, char const *argv[]) {
    // аргументом передаём файл с картой
    // в первой строке задаётся: height width
    if (argc != 2) {
        cout << "Invalid arguments" << endl;
        return 0;
    }
    ifstream infile;
    infile.open(argv[1]);
    if(!infile.is_open()) {
        perror("Failed open"); 
        exit(EXIT_FAILURE); 
    }
    g_life glife = read_gmap(infile);
    infile.close();

    pthread_t thr_id[2];
    pthread_create(&thr_id[0], NULL, game_thread, &glife);
    pthread_create(&thr_id[1], NULL, server_thread, &glife);
    pthread_join(thr_id[0], NULL);
    pthread_join(thr_id[1], NULL);
} 
