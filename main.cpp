#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <ctime>
#include <vector>
#include <cstring>
#include <string>
#include <fstream>

//Массив цветов
const int arrSize = 40;
int Flowers[arrSize];
//Вектор индексов увядающих цветов
std::vector<int> FadingFlowers;
//двоичный семафор
pthread_mutex_t mutex;
//Увядание
void *Fading(void *param) {
    srand(time(NULL));
    while (true) {
        std::string sf;
        pthread_mutex_lock(&mutex); //протокол входа в КС: закрыть двоичный семафор
        for (int i = 0; i < arrSize; i++) {
            if (Flowers[i] == 1) {
                bool to_fade_or_not_to_fade = rand() % 5 == 0;
                if (to_fade_or_not_to_fade) {
                    Flowers[i] = 0;
                    FadingFlowers.push_back(i);
                }
            }
            sf += std::to_string(Flowers[i]);
        }
        std::cout << "time = " << clock() << " | Flowers: " << sf << "\n";
        pthread_mutex_unlock(&mutex); //протокол выхода из КС:
        sleep(4);
    }
    return nullptr;
}
//Полив
void *Watering(void *param) {
    srand(time(NULL));
    int MyNum = *((int *) param);
    while (true) {
        pthread_mutex_lock(&mutex); //протокол входа в КС: закрыть двоичный семафор
        if (FadingFlowers.size() > 0) {
            int NumberOfMyFlower = rand() % FadingFlowers.size();
            int MyFlower = FadingFlowers[NumberOfMyFlower];
            FadingFlowers.erase(FadingFlowers.begin() + NumberOfMyFlower);
            if (Flowers[MyFlower] == 1) {
                std::cout << "Error!!!";
            } else {
                Flowers[MyFlower] = 1;
            }
            std::cout << "time = " << clock() << " | Gardener #" << MyNum + 1 << " watered flower #" << MyFlower + 1
                      << "\n";
        } else {
            std::cout << "All flowers are beautiful\n";
        }
        pthread_mutex_unlock(&mutex); //протокол выхода из КС:
        sleep(1);
    }
    return nullptr;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    //инициализация двоичного семафора
    pthread_mutex_init(&mutex, NULL);
    if (argc != 1) {
        if (strcmp(argv[1], "random") == 0) {
            for (int i = 0; i < arrSize; i++) {
                Flowers[i] = rand() % 2;
                if (Flowers[i] == 0) {
                    FadingFlowers.push_back(i);
                }
            }
        } else if (strcmp(argv[1], "direct") == 0) {
            for (int i = 0; i < argc - 2 && i < arrSize; i++) {
                Flowers[i] = std::stoi(argv[i + 2]);
                if (Flowers[i] != 0 && Flowers[i] != 1) {
                    std::cout << "Input Error";
                    return -1;
                }
            }
            for (int i = 0; i < arrSize; i++) {
                if (Flowers[i] == 0) {
                    FadingFlowers.push_back(i);
                }
            }

        } else if (strcmp(argv[1], "file") == 0) {
            if (argc < 3) {
                std::cout << "Input Error";
                return -1;
            }
            std::ifstream in(argv[2]);
            if (in.is_open()) {
                int counter = 0;
                std::string x;
                while (in >> x && counter < 40) {
                    if (x != "0" && x != "1") {
                        std::cout << "Input Error";
                        return -1;
                    }
                    Flowers[counter] = std::stoi(x);
                    counter++;
                }
                for (int i = 0; i < arrSize; i++) {
                    if (Flowers[i] == 0) {
                        FadingFlowers.push_back(i);
                    }
                }
            } else {
                std::cout << "Input Error";
                return -1;
            }
            in.close();
        } else {
            std::cout << "Input Error";
            return -1;
        }
    } else {
        std::cout << "Enter \"random\", \"direct\" or \"file\": ";
        std::string str;
        while (true) {
            std::cin >> str;
            if (str == "random" || str == "direct" || str == "file") {
                break;
            }
        }
        if (str == "random") {
            for (int i = 0; i < arrSize; i++) {
                Flowers[i] = rand() % 2;
                if (Flowers[i] == 0) {
                    FadingFlowers.push_back(i);
                }
            }
        } else if (str == "direct") {
            for (int i = 0; i < arrSize; i++) {
                std::cout << "Enter Flower[" << i << "], 0 - fading, 1 - beautiful: ";
                while (true) {
                    int x;
                    std::cin >> x;
                    if (x == 0 || x == 1) {
                        Flowers[i] = x;
                        break;
                    }
                }
                for (int i = 0; i < arrSize; i++) {
                    Flowers[i] = rand() % 2;
                    if (Flowers[i] == 0) {
                        FadingFlowers.push_back(i);
                    }
                }
            }
        } else if (str == "file") {
            std::string path;
            std::cin >> path;
            std::ifstream in(path);
            if (in.is_open()) {
                int counter = 0;
                std::string x;
                while (in >> x && counter < 40) {
                    if (x != "0" && x != "1") {
                        std::cout << "Input Error";
                        return -1;
                    }
                    Flowers[counter] = std::stoi(x);
                    counter++;
                }
                for (int i = 0; i < arrSize; i++) {
                    Flowers[i] = rand() % 2;
                    if (Flowers[i] == 0) {
                        FadingFlowers.push_back(i);
                    }
                }
            } else {
                std::cout << "Input Error";
                return -1;
            }
            in.close();
        }
    }

    int num[2];
    for (int i = 0; i < 2; i++) {
        num[i] = i;
    }
    //идентификаторы для дочерних потоков
    pthread_t GardenerThread1;
    pthread_t GardenerThread2;
    //создание дочерних потоков
    pthread_create(&GardenerThread1, NULL, Watering, (void *) (num + 0));
    pthread_create(&GardenerThread2, NULL, Watering, (void *) (num + 1));
    //Вызов
    Fading(NULL);
    pthread_join(GardenerThread1, NULL);
    pthread_join(GardenerThread2, NULL);

    pthread_mutex_destroy(&mutex);
}

