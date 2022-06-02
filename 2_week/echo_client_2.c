#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 1024 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기

void error_handling(char* message);

int main(int argc, char* argv[]) {
    int sock; // 소켓이 잘 만들어졌는지 확인하기 위함
    char message[BUF_SIZE];
    int str_len, recv_len, recv_cnt; // recv_len, recv_cnt 추가
    struct sockaddr_in serv_adr;

    if(argc != 3) {
        printf("Usage: %s <IP> <PORT>\n", argv[0]);
        exit(-1);
    }

    // PF_INET, SOCK_STREAM 은 tcp 통신을 하겠다는 의미
    // 0 은 기본값
    // 이건 서버가 어떤 통신을 하느냐
    sock = socket(PF_INET, SOCK_STREAM, 0); // socket을 정의하는 중
    if (sock == -1) {
        error_handling("socket() ERROR");
    }

    // 레이어간 규약 맞추기
    // serv_adr structure을 0으로 초기화 시키기
    memset(&serv_adr, 0, sizeof(serv_adr));
    // 새로 정의하기
    serv_adr.sin_family = AF_INET; // PF INET과 같이 TCP를 쓰겠다. 이건 나 자신이 어떻게 통신하겠다고 정의하는 것
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]); // 서버 어드레스 설정
    serv_adr.sin_port = htons(atoi(argv[2])); // 포트 설정

    // 서버랑 연결
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("connect() error");
    } else {
        puts("connected..");
    }

    while (1)
    {
        fputs("Input message(Q, q to end):", stdout);
        fgets(message, BUF_SIZE, stdin);

        if(!strcmp(message, "q\n") || !strcmp(message, "Q\n")) {
            break;
        }

        str_len = write(sock, message, strlen(message));
        recv_len = 0;
        
        while (recv_len < str_len)
        {
            // NAK 보내는 것처럼 잘 도착하지 못했으면 계속 다시 보내달라고 요청해야 하는 것 같은데?
            // Q) read 의 마지막 BUF_SIZE는 잘못 들어간듯, BUF_SIZE - recv_len 해야 하는거 아닌가?
            recv_cnt = read(sock, &message[recv_len], BUF_SIZE - 1);
            if (recv_cnt == -1) { // 실패했다는 의미
                error_handling("read() error!\n");
            }
            recv_len += recv_cnt;
        }

        message[recv_len] = 0;
        
        // 소켓에 내가 적은 메세지 보내기
        // write(sock, message, strlen(message));
        // 서버에서 온 메세지 받기
        //str_len = read(sock, message, BUF_SIZE - 1);
        // 마지막 \n 없애주기
        //message[str_len] = 0;
        printf("message from server %s\n", message);
    }
    
    close(sock);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(-1);
}