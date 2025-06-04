/* file per la lettura in input del driver /ttyAC0 e la scrittura su un chardev personalizzato creato nel file ./chardev.c*/
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> 
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "common.h"

#define SERIAL_PATH "/dev/ttyACM0" 
#define DEVICE_PATH "/dev/tiziano_chardev0" //da aggiungere
int fd = -1;
int device_fd = -1;

void cleanup(int signo){
    if(fd > -1){
        close(fd);
    }
    if(device_fd > -1){
        close(fd);
    }
    exit(0);
}

//attenzione, ho scoperto la possibilità di usare flock sul file in modo da evitare accessi concorrenti
int main(){
    // lettura del file di configurazione, se non esiste ne crea uno vuoto
    struct sigaction sig;
    memset((void*) &sig, 0, sizeof(sig)); 
    sig.sa_handler = cleanup;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGHUP, &sig, NULL);
    sigaction(SIGINT, &sig, NULL);
    int ret;
    int macro_fd = open(MACRO_PATH, O_RDONLY, 0666);
    if(macro_fd < 0){
        if(errno == ENOENT){
            ret = inizializza();
            if(ret < 0){
                perror("errore nell'inizializzazione del file");
                return -1;
            }
            macro_fd = open(MACRO_PATH, O_RDONLY, 0666);
       }
        else{
            perror("errore apertura file macro");
            return -1;
        }
    }
    //copia del file in memoria
    char macro_map[MAX_MACRO][MAX_MACRO_LEN];
    for (int i = 0; i < MAX_MACRO; i++){
        int j = 0;
        while(j < MAX_MACRO_LEN-1){
            ret = read(macro_fd, &(macro_map[i][j]), 1);
            if(macro_map[i][j++] == '\n') break; //da rivedere l'efficienza
            if(ret == 0) break;
            if(ret < 0){ //la gestione di EINTR è da fare
                perror("errore nella lettura del file delle macro");
                return -1;
            }
            
        }
        //aggiunta terminatore stringa
        macro_map[i][j] = '\0';
        printf("macro[%d]: %s", i, macro_map[i]);
    } 
    close(macro_fd);
    printf("aquisizione file effettuata\n"); 
    
    fd = open(SERIAL_PATH, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        perror("il dispositivo non è collegato o non è in /dev/ttyACM0\n");
        return -1;
    }
    // struttura per la gestione delle opzioni del file
    struct termios opts;
    memset((void*) &opts, 0, sizeof(opts)); 
    device_fd = open(DEVICE_PATH, O_WRONLY);
    
    if(device_fd < 0){
        perror("il kernel module non è stato caricato");
        close(fd);
        return -1;
    }
    
    opts.c_cflag = B19200 | CS8 | CLOCAL | CREAD; // baudrate, bit per messaggio, controllo del segnale, abilitazione dispositivo
    // le flag le sto usando praticamente solo in controllo
    opts.c_iflag = 0;
    opts.c_oflag = 0;
    opts.c_lflag = 0;

    // metto i parametri
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &opts);

    u_int8_t buf = 0;
    while(1){

    READ:
        ret = read(fd, &buf, 1);
        if( ret < 0){
            if(errno == EINTR) goto READ;
            perror("error reading");
            ret = -1;
            goto CLEANUP;
        }
        if (ret == 0) continue;
        if(buf > MAX_MACRO -1){
            printf("numero della macro non valido\n");
            continue; //passo semplicemente alla prossima lettura
        }    
        int len = strlen(macro_map[buf]);
        //printf("line: %s\n", buf);
        int scritti = 0;
        while(scritti < len){
            ret = write(device_fd, macro_map[buf]+scritti, len-scritti);
            if(ret < 0){
                perror("errore nella scrittura sul device\n");
                break;
            }
            scritti += ret;
        }
    }
    ret = 0;
CLEANUP:
    close(fd);            
    close(device_fd);            
    return ret;
}
