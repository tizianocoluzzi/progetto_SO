/* file per la lettura in input del driver /ttyAC0 e la scrittura su un chardev personalizzato creato nel file ./chardev.c*/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> 
#include <termios.h>

#define SERIAL_PATH "/dev/ttyACM0" 
#define DEVICE_PATH "/dev/tiziano_chardev0" //da aggiungere
int main(){
    //N.B. per aprire il file ho dovuto dare i permessi al device /dev/ttyACM0 il che potrebbe essere un problema per la scalabilità
    int fd = open(SERIAL_PATH, O_RDONLY | O_NOCTTY);
    // struttura per la gestione delle opzioni del file
    struct termios opts;
    int device_fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("il dispositivo non è collegato o non è in /dev/ttyACM0\n");
        return -1;
    }
    if(device_fd < 0){
        perror("il kernel module non è stato caricato");
        return -1;
    }
    
    opts.c_cflag = B19200 | CS8 | CLOCAL | CREAD; // baudrate, bit per messaggio, controllo del segnale, abilitazione dispositivo
    // le setto a zero per vedere poi che fanno
    // le flag le sto usando praticamente solo in controllo
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
        //printf("line: %s\n", buf);
        write(device_fd, buf, 1);
    }
    return 0;
}
