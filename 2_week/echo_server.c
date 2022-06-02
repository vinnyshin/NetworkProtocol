#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리

#define BUF_SIZE 1024 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기

void error_handling(char* message);

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock; // 서버 나 자신과 클라이언트 소켓 정의
    char message[BUF_SIZE];
    char message2[BUF_SIZE] = "Hi hello it's me";
    int str_len, i;

    struct sockaddr_in serv_adr;
    struct sockaddr_in clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc != 2) {
        printf("Usage: %s <PORT>\n", argv[0]);
        exit(-1);
    }

    // PF_INET, SOCK_STREAM 은 프로토콜을 tcp 통신을 하겠다는 의미
    // 0 은 기본값 다른 값을 쓸 수 있어
    // 이건 서버가 어떤 통신을 하느냐
    serv_sock = socket(PF_INET, SOCK_STREAM, 0); // socket을 정의하는 중
    if (serv_sock == -1) {
        error_handling("socket() ERROR");
    }

    // 레이어간 규약 맞출거야
    // serv_adr structure을 0으로 초기화 시키기
    memset(&serv_adr, 0, sizeof(serv_adr));
    // 새로 정의하기
    serv_adr.sin_family = AF_INET; // PF INET과 같이 TCP를 쓰겠다. 이건 나 자신이 어떻게 통신하겠다고 정의하는 것
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY); // 내 어드레스를 (IP를) 자동으로 가져오는 함수
    serv_adr.sin_port = htons(atoi(argv[1])); // 포트 설정

    // 서버가 클라이언트를 받을 준비하기
    if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
        error_handling("bind() error");
    }

    // 클라이언트를 받을 준비가 되었으니, listen을 통해 내용 받기
    // 클라이언트를 5명 받겠다는 의미
    if(listen(serv_sock, 5) == -1) {
        error_handling("listen() error");
    }

    clnt_adr_sz = sizeof(clnt_adr);

    for (int i = 0; i < 5; i++)
    {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
        if(clnt_sock == -1) {
            error_handling("accept() error");
        } else {
            printf("connected client %d \n", i + 1);
        }
        while ((str_len = read(clnt_sock, message, BUF_SIZE)) != 0)
        {
            // 이게 정답이 아닌 코드라는데..?
            // write(clnt_sock, "hi", str_len);
            // 이거는 response가 안가
            // 잘린대, 다음주에 자세히
            write(clnt_sock, message2, BUF_SIZE);
        }
        close(clnt_sock);
    }

    close(serv_sock);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}