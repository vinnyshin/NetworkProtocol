#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 1024 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기
#define OPSZ 4

void error_handling(char* message);
int calculate(int opnum, int opnds[], char oprator);

int main (int argc, char* argv[]) {
    int serv_sock, clnt_sock;
    char opinfo[BUF_SIZE];
    int result, opnd_cnt, i;
    int recv_cnt, recv_len;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if (argc != 2)
    {
        printf("usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1)
    {
        error_handling("sockt() error\n");
    }

    memset(&serv_adr, 0, sizeof(serv_adr));

    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }
    
    if(listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    clnt_adr_sz = sizeof(clnt_adr);
    
    for (int i = 0; i < 5; i++)
    {
        opnd_cnt = 0;
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        read(clnt_sock, &opnd_cnt, 1); // 1 OFFSET씩 읽기

        recv_len = 0;
        // 마지막 operator를 위해 + 1
        while ((opnd_cnt * OPSZ + 1) > recv_len)
        {
            recv_cnt = read(clnt_sock, &opinfo[recv_len], BUF_SIZE - 1);
            // char로 읽어서 나중에 int*로 캐스팅
            recv_len += recv_cnt;
        }
        
        result = calculate(opnd_cnt, (int*)opinfo, opinfo[recv_len - 1]);
        write(clnt_sock, (char*)&result, sizeof(result));
        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

int calculate(int opnum, int opnds[], char op) {
    int result = opnds[0], i;

    switch (op)
    {
    case '+':
        for (int i = 0; i < opnum; i++)
        {
            result += opnds[i];
        }
        break;
    case '-':
        for (int i = 0; i < opnum; i++)
        {
            result -= opnds[i];
        }
        break;
    case '*':
        for (int i = 0; i < opnum; i++)
        {
            result *= opnds[i];
        }
        break;
    case '/':
        for (int i = 0; i < opnum; i++)
        {
            result /= opnds[i];
        }
        break;
    

    default:
        break;
    }

    return result;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}
