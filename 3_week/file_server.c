#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 30 // 서로 메세지를 주고 받을 때 사용할 메세지크기

void error_handling(char* message);

int main(int argc, char const *argv[])
{
    int serv_sd, clnt_sd;
    FILE* fp;
    char buf[BUF_SIZE];
    int read_cnt;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if (argc != 2)
    {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }

    fp = fopen("test.txt", "rb");

    serv_sd = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sd == -1)
    {
        error_handling("sockt() error\n");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    // 대기열 5개 만들기
    if(listen(serv_sd, 5) == -1) {
        error_handling("listen() error");
    }

    clnt_adr_sz = sizeof(clnt_adr);
    clnt_sd = accept(serv_sd, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

    while (1)
    {
        read_cnt = fread((void*)buf, 1, BUF_SIZE, fp);

        if(read_cnt < BUF_SIZE) { // 파일의 마지막 부분에서는 BUF SIZE만큼 못 읽어 올 것이니까.
            write(clnt_sd, buf, read_cnt);
            break;
        }
        write(clnt_sd, buf, BUF_SIZE);
    }

    shutdown(clnt_sd, SHUT_WR);
    read(clnt_sd, buf, BUF_SIZE);
    printf("Message: %s \n", buf);

    fclose(fp);
    close(clnt_sd);
    close(serv_sd);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}
