#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 1024 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기
#define RLT_SIZE 4
#define OPSZ 4

void error_handling(char* message);
int calculate(int opnum, int opnds[], char oprator);

int main(int argc, char* argv[]) {
    int sock;
    char opmsg[BUF_SIZE];
    int result, opnd_cnt, i;
    struct sockaddr_in serv_adr; // 서버 주소 저장할 변수

    if(argc != 3) {
        printf("usage: %s <IP> <PORT>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        error_handling("socket() error");
    }

    memset(&serv_adr, 0 ,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)  {
        error_handling("connect() error");
    } else {
        puts("CONNECT ----> ");
    }

    fputs("Operand count: ", stdout);
    scanf("%d", &opnd_cnt);
    opmsg[0] = (char)opnd_cnt;
    // 이것도 256 이상 안되는 코드인거네

    for (int i = 0; i < opnd_cnt; i++)
    {
        printf("Operand %d:", i + 1);
        scanf("%d", (int*)&opmsg[i * OPSZ + 1]);
    }

    fgetc(stdin);
    fputs("Operator: ", stdout);
    scanf("%c", &opmsg[opnd_cnt * OPSZ + 1]);
    
    write(sock, opmsg, opnd_cnt*OPSZ + 2);
    read(sock, &result, RLT_SIZE);
    
    printf("Operation result: %d \n", result);
    close(sock);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}
