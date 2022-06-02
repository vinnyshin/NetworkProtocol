#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

// 감시를 하면서 변화를 감지하고 파일에 적어준다.

int main() {
    int fd, n, ret;

    char buf[128];
    struct timeval tv;

    fd_set readfs;
    fd = open("write.txt", O_RDONLY);

    if(fd < 0) {
        perror("file open error");
        exit(-1);
    }

    // 버퍼 초기화
    memset(buf, 0x00, 128);

    // 감시자
    FD_ZERO(&readfs);

    while(1) {
        FD_SET(fd, &readfs);
        // 몇개를 감시할거냐를 select에서 요구함, fd는 index기 때문에 + 1 하면 file descripter 배열 전체의 크기
        ret = select(fd + 1, &readfs, NULL, NULL, NULL);

        if(ret == -1) {
            perror("select error");
            exit(-1);
        }

        if(FD_ISSET(fd, &readfs)) {
            while((n = read(fd, buf, 128)) > 0) {
                // 계속 읽기.
                printf("%s", buf);
            }
        }

        memset(buf, 0x00, 128); // 이거 없어도 동작할 것 같은데? 어차피 128씩 다시 읽어서 덮어씌우니까 나중에 test
        usleep(10000);
    }
}   