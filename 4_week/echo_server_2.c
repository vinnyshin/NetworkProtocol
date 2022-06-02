// https://plummmm.tistory.com/67

// echo_server.c는 file descriptor로 번호 붙혀서 사용
// echo_sever_2.c는 pid를 받을 수 있게 되어서 관리하기가 더 편해..
// clown14@hanyang.ac.kr 김시은

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리
#include <sys/time.h>
#include <sys/select.h>
#include <sys/signal.h>

#define BUF_SIZE 100 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기

void error_handling(char* message);
void read_childproc(int sig);

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock; // 서버 나 자신과 클라이언트 소켓 정의
    struct sockaddr_in serv_adr, clnt_adr;

    pid_t pid;
    struct sigaction act; // signal을 핸들링 하겠다는 의미
    socklen_t adr_sz;
    int str_len, state;
    char buf[BUF_SIZE];

    if(argc != 2) {
        printf("Usage: %s <PORT>\n", argv[0]);
        exit(-1);
    }

    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    state = sigaction(SIGCHLD, &act, 0);

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

    while(1) {
        adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
        if (clnt_sock == -1) {
            continue;
        } else {
            puts("new client connect...");
        }

        pid = fork();
        if(pid == -1) {
            close(clnt_sock);
            continue;
        } else if (pid == 0) { // 정상 이는 child 가 받는 것
            close(serv_sock);
            while((str_len = read(clnt_sock, buf, BUF_SIZE)) != 0) {
                write(clnt_sock, buf, str_len);
            }
            close(clnt_sock);
            puts("client disconnect");
            return 0;
        } else {
            close(clnt_sock);
        }
    }
    
    close(serv_sock);
    return 0;
}

void error_handling(char* message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void read_childproc(int sig) {
    pid_t pid;
    int status;
    
    // https://codetravel.tistory.com/42
    // waitpid의 첫 항이 -1일 경우 임의의 자식 프로세스를 기다린다
    // 기다리는 PID가 종료되지 않아서 즉시 종료 상태를 회수 할 수 없는 상황에서 호출자는 차단되지 않고 반환값으로 0을 받음
    // https://codingdog.tistory.com/entry/%EB%A6%AC%EB%88%85%EC%8A%A4-waitpid-%ED%95%A8%EC%88%98-%EC%9E%90%EC%8B%9D-%ED%94%84%EB%A1%9C%EC%84%B8%EC%8A%A4%EB%A5%BC-%EA%B8%B0%EB%8B%A4%EB%A6%B0%EB%8B%A4
    pid = waitpid(-1, &status, WNOHANG);
    printf("remove prod id %d \n", pid);
}