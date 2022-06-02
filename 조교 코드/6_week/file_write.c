#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]){
  int fd, ret;
  fd = open("select_file.txt",O_CREAT|O_WRONLY|O_APPEND);//없으면 만들고 write를 하게하며, append까지 가능하도록 한다.

  if(fd < 0){
        perror("file erorr ");
        exit(0);
  }

  if(argc != 2){
        printf(" usage : ./f_write <msg> ");
        exit(0);
  }

  strcat(argv[1],"\n");
  ret = write(fd, argv[1], strlen(argv[1]));
  if(ret < 0){
        perror("write error ");
        exit(0);
  }

  close(fd);
  return 0;
}