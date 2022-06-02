// 참고할만한 사이트
// https://pony11.tistory.com/15
// https://blog.naver.com/tipsware/220810795410

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // 통신을 하기 위한 라이브러리
#include <sys/socket.h> // 통신을 하기 위한 라이브러리
#include <sys/time.h>
#include <sys/select.h>

#define BUF_SIZE 100 // 서로 메세지를 주고 받을 때 사용할 메세지의 크기

void error_handling(char* message);

int main(int argc, char* argv[]) {
    int serv_sock, clnt_sock; // 서버 나 자신과 클라이언트 소켓 정의
    struct sockaddr_in serv_adr, clnt_adr;
    struct timeval timeout;
    fd_set reads, cpy_reads;
    
    socklen_t adr_sz;
    int fd_max, str_len, fd_num, i;
    char buf[BUF_SIZE];

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

    FD_ZERO(&reads); // read 초기화
    FD_SET(serv_sock, &reads); // serv_sock를 reads fd set에 넣어서 관리하겠다
    // 등록이 되었는지 안되었는지 확인하려면 FD_ISSET(serv_sock, &reads) == 1 이 True가 돼 >> 시험에 안나옴
    fd_max = serv_sock;

    while(1) {
        cpy_reads = reads;
        timeout.tv_sec = 5;
        timeout.tv_usec = 5000;

        // select : 여러개의 소켓을 하나의 스레드로 처리한다.
        // select 함수는 기본적으로 fd에 변화가 생길때까지 대기하는 블로킹 방식으로 처리한다.
		// 한 곳에 모아놓은 여러 개의 파일 디스크립터를 동시에 관찰할 수 있게 한다.
		// 여기서는 여러 개의 소켓 중 읽기, 쓰게, 예외 소켓을 fd_set에 넣어서
		// 변화를 감지하고, 변화가 발생했을 때 I/O를 하게 되는 방식이다.
		// 1) int nfds : fd 개수 범위(유닉스 호환용. 윈도우에서는 사용하지 않는다고 한다)
		// 2) fd_set* readfds : 수신된 데이터가 있는지 확인할 fd.
		// 3) fd_set* writefds : 보낼 준비가 된 데이터가 있는지 확인할 fd.
		// 4) fd_set* exceptfds : 예외발생이 있는지 확인할 fd.
		// 5) const timeval* timeout : seconds, microseconds 단위로 타임 아웃을 나타냄.
		// NULL을 전해주면 하나라도 만족할 때 까지 대기한다. 양수값이면 그 시간까지 대기.
		// 리턴값(int) : -1(오류 발생), 0(타임 아웃), 0보다 큰수(변화 발생 fd 갯수)
        if((fd_num = select(fd_max + 1, &cpy_reads, 0, 0, &timeout)) == -1) {
            break; 
        }
        if(fd_num == 0) {
            continue; // time out
        }
        for (int i = 0; i < fd_max + 1; i++) {
            if(FD_ISSET(i, &cpy_reads)) {
                if(i == serv_sock) {
                    adr_sz = sizeof(clnt_adr);
                    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    if(fd_max < clnt_sock) {
                        fd_max = clnt_sock;
                    }
                    printf("connected client: %d\n", clnt_sock);
                } else {
                    str_len = read(i, buf, BUF_SIZE);
                    if(str_len == 0) {
                        FD_CLR(i, &reads); // 관찰을 종료하겠다는 의미, 정보 삭제
                        close(i);
                        printf("close client: %d\n", i);
                    } else {
                        write(i, buf, str_len);
                    }
                }
            }
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