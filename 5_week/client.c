#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define NAME_SIZE 20

void* send_msg(void* msg);
void* recv_msg(void* msg);
void error_handling(char* msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

// 컴파일 명령어: gcc client.c -D_REENTRANT -o client -lpthread
int main(int argc, char const* argv[])
{
    int sock;
    struct sockaddr_in serv_adr;    
    pthread_t snd_thread, rcv_thread;
    void* thread_return;

    if(argc != 4) {
        printf("Usage: %s <IP> <PORT> <name> \n", argv[0]);
        exit(-1);
    }

    sprintf(name, "[%s]", argv[3]);
    sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET; // PF INET과 같이 TCP를 쓰겠다. 이건 나 자신이 어떻게 통신하겠다고 정의하는 것
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]); // 서버 어드레스 설정
    serv_adr.sin_port = htons(atoi(argv[2])); // 포트 설정

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
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;

    while (1) {
        // 마지막에 EOF하고 이스케이프 문자하고 구별이 안가서 -1 해준대
        str_len = read(sock, name_msg, NAME_SIZE + BUF_SIZE - 1);

        if(str_len == -1) {
                return (void*)-1;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);
    }
    return NULL;
}

void* send_msg(void *arg) {
    int sock = *((int*)arg);
    char name_msg[NAME_SIZE + BUF_SIZE];
    int str_len;

    while (1) {
        fgets(msg, BUF_SIZE, stdin);
        if(!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
                close(sock);
                exit(0);
        }

        sprintf(name_msg, "%s %s", name, msg);
        write(sock, name_msg, strlen(name_msg));    
    }
    
    return NULL;
}


void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}