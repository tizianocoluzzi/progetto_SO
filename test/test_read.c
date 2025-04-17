#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> 
#include <termios.h>

#define PATH "/dev/ttyACM0" 

int main(){
    //N.B. per aprire il file ho dovuto dare i permessi al device /dev/ttyACM0 il che potrebbe essere un problema per la scalabilità
    int fd = open(PATH, O_RDONLY | O_NOCTTY);
    // struttura per la gestione delle opzioni del file
    struct termios opts;

    if (fd < 0) {
        perror("error opening file\n");
        return -1;
    }
    
    opts.c_cflag = B19200 | CS8 | CLOCAL | CREAD; // baudrate, bit per messaggio, le altre boh
    // le setto a zero per vedere poi che fanno
    opts.c_iflag = 0;
    opts.c_oflag = 0;
    opts.c_lflag = 0;

    // metto i parametri
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &opts);

    char buf[1024];
    memset(buf, 0, 1024);
    while(1){
        int ret = read(fd, buf, 1023);
        if( ret < 0){ 
            perror("error reading");
            return -1;
        }
        if (ret == 0) continue;
        printf("line: %s\n", buf);
    }
    return 0;
}
