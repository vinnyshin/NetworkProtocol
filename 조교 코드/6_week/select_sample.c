#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> //파일제어

int main(){
  int fd, n, ret;

  char buf[128];
  struct timeval tv;

  fd_set readfds; //파일디스크립터를 정의
  fd = open("select_file.txt",O_RDONLY); //open해서 read only로 열기 때문에 수정할 수 없음
  if(fd < 0){
        perror("file open error ");
        exit(0);
  }
  memset(buf, 0x00, 128); //버퍼 128바이트를 0으로 초기화
  
  FD_ZERO(&readfds);//파일디스크립터 초기화
  while(1){
        FD_SET(fd,&readfds); //감시할 파일을 디스크립터에 할당
	
	    //fd+1을 해주는 이유: 만약에 내가 3번을 할당을 받았으면 감시해야 될 게 4개가 됨
        ret = select(fd+1, &readfds, NULL, NULL, NULL);//세번째는 파일에 쓸 수 있는지 검사, 네번째는 예외사항 검사, 다섯번째는 timeout 여기서는 NULL 이기 때문에 무기한 대기임

        if(ret == -1){
                perror("select error ");
                exit(0);
        }

        if(FD_ISSET(fd, &readfds)){//setting이 되었는지 확인
                while(( n = read(fd, buf, 128)) > 0)
                        printf("%s",buf);
        }

        memset(buf, 0x00, 128);
        usleep(1000);
  }
}