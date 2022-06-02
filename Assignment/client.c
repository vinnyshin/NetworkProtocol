#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BOARD_SIDE 5
#define WIN_BINGO_COUNT 3

int game_board[BOARD_SIDE][BOARD_SIDE];
int is_input = 0;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t sync_mutx = PTHREAD_MUTEX_INITIALIZER;

#define BUF_SIZE 100
#define NAME_SIZE 20

void* send_msg(void* msg);
void* recv_msg(void* msg);
void error_handling(char* msg);

void select_number(int num);
void swap(int* x, int* y);

void init_game_board();
void print_game_board();
int is_bingo();

char msg[BUF_SIZE];

// 컴파일 명령어: gcc client.c -D_REENTRANT -o client -lpthread
int main(int argc, char const* argv[])
{
    int sock;
    struct sockaddr_in serv_adr;    
    pthread_t snd_thread, rcv_thread;
    void* thread_return;

    if(argc != 3) {
        printf("Usage: %s <IP> <PORT> \n", argv[0]);
        exit(-1);
    }

    init_game_board(); 

    sock = socket(PF_INET, SOCK_STREAM, 0);
    pthread_cond_init(&cond, NULL);
    pthread_mutex_init(&sync_mutx, NULL);
    

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("connect() error");
    }

    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    close(sock);
    return 0;
}

void* recv_msg(void* arg) {
    int sock = *((int*)arg);
    char msg[BUF_SIZE];
    int str_len, recv_num;

    while (1) {
        if (is_input == 0) {
            
            str_len = read(sock, &recv_num, 4);

            if(str_len == -1) {
                return (void*)-1;
            }
            if(recv_num == 254) {
                printf("YOU LOSE!\n");
                close(sock);
                exit(0);
            }
            if(recv_num == 255) {
                close(sock);
                exit(0);
            } else {
                select_number(recv_num);
                print_game_board();
                int bingo_cnt = is_bingo();
                printf("bingo count: %d\n", bingo_cnt);
                
                if (bingo_cnt == WIN_BINGO_COUNT) {
                    printf("YOU WIN!!\n");
                    msg[0] = (char)254;
                    write(sock, &msg, 1);
                    close(sock);
                    exit(0);
                }
                
                is_input = 1;
                pthread_cond_signal(&cond);
            }
        }
    }
    return NULL;
}

void* send_msg(void *arg) {
    int str_len, input_num, sock = *((int*)arg);
    char msg[BUF_SIZE];

    pthread_mutex_lock(&sync_mutx);
    while (1) {
        if (is_input == 1) {
            int retry;
            do
            {
                retry = 0;
                printf("give me a number between 1 ~ 25 (-1 to end) >> ");
                scanf("%d", &input_num);
                if (input_num == -1) {
                    msg[0] = (char)255;
                    write(sock, &msg, 1);
                    close(sock);
                    exit(0);
                } else if(input_num < 1 || input_num > 25) {
                    retry = 1;
                }
            } while (retry == 1);

                    
            msg[0] = (char)input_num;
            write(sock, &msg, 1);
            is_input = 0;
        } else {
            pthread_cond_wait(&cond, &sync_mutx);
        }
    }
    pthread_mutex_unlock(&sync_mutx);
    return NULL;
}


void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void init_game_board() {
    srand((unsigned int)time(NULL));
    int* board_pointer = (int*)game_board;
    
    for(int i = 0; i < BOARD_SIDE * BOARD_SIDE; ++i) {
        board_pointer[i] = i + 1;
    }

    for(int i = 0; i < BOARD_SIDE * BOARD_SIDE; ++i) {
        swap(&board_pointer[i], &board_pointer[rand() % (BOARD_SIDE * BOARD_SIDE)]);
    }
}

void swap(int* x, int* y) {
    int temp;
    temp = *x;
    *x = *y;
    *y = temp;
}

void print_game_board() {
    for(int row = 0; row < BOARD_SIDE; ++row) {
        for(int column = 0; column < BOARD_SIDE; ++column) {
            printf("%d ", game_board[column][row]);
        }
        printf("\n");
    }
}


int is_bingo() {
    int sum = 0;
    int bingo_cnt = 0;

    // find a bingo by row == horizontally
    for(int row = 0; row < BOARD_SIDE; ++row) {
        for(int column = 0; column < BOARD_SIDE; ++column) {
            sum += game_board[column][row];
        }

        if (sum == 0) {
            ++bingo_cnt;
            if (bingo_cnt == WIN_BINGO_COUNT) {
                return bingo_cnt;
            }
        } else {
            sum = 0;
        }
    }

    // find a bingo by column == vertically
    for(int column = 0; column < BOARD_SIDE; ++column) {
        for(int row = 0; row < BOARD_SIDE; ++row) {
            sum += game_board[column][row];
        }

        if (sum == 0) {
            ++bingo_cnt;
            if (bingo_cnt == WIN_BINGO_COUNT) {
                return bingo_cnt;
            }
        } else {
            sum = 0;
        }
    }

    // find a bingo by diagonally (y = -x graph shape)
    for (int i = 0; i < BOARD_SIDE; ++i) {
        sum += game_board[i][i];
    }

    if (sum == 0) {
        ++bingo_cnt;
        if (bingo_cnt == WIN_BINGO_COUNT) {
            return bingo_cnt;
        }
    } else {
        sum = 0;
    }
    
    // find a bingo by diagonally (y = x graph shape)
    for (int i = 0; i < BOARD_SIDE; ++i) {
        sum += game_board[i][BOARD_SIDE - 1 - i];
    }

    if (sum == 0) {
        ++bingo_cnt;
        if (bingo_cnt == WIN_BINGO_COUNT) {
            return bingo_cnt;
        }
    }

    return bingo_cnt;
}

void select_number(int num) {
    int x, y;

    for (int column = 0; column < BOARD_SIDE; column++)
    {
        for (int row = 0; row < BOARD_SIDE; row++)
        {
            if (game_board[column][row] == num) {
                game_board[column][row] = 0;
            }
        }
    }
}