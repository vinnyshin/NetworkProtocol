#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 30 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기

void error_handling(char* message);

int main(int argc, char const *argv[])
{
    int sd;
    FILE* fp;
    
    char buf[BUF_SIZE];
    int read_cnt;
    struct sockaddr_in serv_adr;
    
    if(argc != 3) {
        printf("usage: %s <IP> <PORT>\n", argv[0]);
        exit(1);
    }

    fp = fopen("receive.txt", "wb");
    sd = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&serv_adr, 0 ,sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    
    if(connect(sd, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)  {
        error_handling("connect() error");
    } else {
        puts("CONNECT ----> ");
    }

    while ((read_cnt = read(sd, buf, BUF_SIZE)) != 0)
    {
        fwrite((void*)buf, 1, read_cnt, fp);
    }

    puts("Receive file Data \n");
    write(sd, "BYE\n", 10);
    
    fclose(fp);
    close(sd);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}
