#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
int main(){
    int fd = open("test.txt", O_RDWR | O_CREAT, 0666);
    int ret;
    if(fd < 0){
        perror("errore nell'apertura del file");
        return -1;
    }
    //lettura di una linea
    char* buf = malloc(1024);
    while(*buf++ != '\n'){
        ret = read(fd, buf, 1);
        printf("%c", *buf);
        if(ret < 0 && ret != EINTR){
            perror("errore nella lettura del file");
            return -1;
        }
        
    }
    
    free(buf);
   
    close(fd);
    return 0;
}
