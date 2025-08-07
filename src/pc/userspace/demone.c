/* file per la lettura in input del driver /ttyAC0 e la scrittura su un chardev personalizzato creato nel file ./chardev.c*/
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/inotify.h>
#include <unistd.h> 
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "common.h"

#define DEVICE_PATH "/dev/tiziano_chardev0" 
//descrittori file a -1 per controllo durante la pulizia
int fd = -1;
int device_fd = -1;

char macro_map[MAX_MACRO][MAX_MACRO_LEN];

pthread_mutex_t mutex;

int copia_macro_map(){
    // lettura del file di configurazione, se non esiste ne crea uno vuoto
    int macro_fd = open(MACRO_PATH, O_RDONLY, 0666);
    int ret;
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
    pthread_mutex_lock(&mutex);    
    //copia del file in memoria
    for (int i = 0; i < MAX_MACRO; i++){
        int j = 0;
        while(j < MAX_MACRO_LEN-1){
            ret = read(macro_fd,&(macro_map[i][j]), 1);
            if(macro_map[i][j++] == '\n') break;
            if(ret == 0) break;
            if(ret < 0){ 
                perror("errore nella lettura del file delle macro");
                return -1;
            }
            
        }
        //aggiunta terminatore stringa
        //printf("macro[%d]: %s", i, macro_map[i]);
    }
    pthread_mutex_unlock(&mutex);
    close(macro_fd);
    //printf("aquisizione file effettuata\n"); 
    return 0;
}


void* inotify_thread(void* arg){
    /*  thread per l'aggiornamento di macro_map, attende con inotify */
    //inizializzazione inotify
    int ifd = inotify_init();
    int ret;
    if(ifd < 0){
        perror("errore nell'inizializzazione di inotify");
        return NULL;
    }
    int wd1 = inotify_add_watch(ifd, MACRO_PATH,IN_MODIFY);
    if(wd1 < 0){
        perror("errore nell'aggiunta di macro path in inotify");
        return NULL;
    }
    char ev_buf[sizeof(struct inotify_event)];
    //loop lettura evento, termina quando termina il processo
    while(1){
        ret = read(ifd, &ev_buf, sizeof(struct inotify_event));
        printf("letti %d bytes\n", ret);
        struct inotify_event* ev = (struct inotify_event *)ev_buf;
        if(ev->wd == wd1){
            //printf("modificate macro, TODO aggiornare\n");
            copia_macro_map();
        }
    }
    
    return NULL;
}

void cleanup(int signo){
    if(fd > -1){
        close(fd);
    }
    if(device_fd > -1){
        close(fd);
    }
    exit(0);
}

int main(int argc, char* argv[]){
    int ret;
    char* serial_path;
     
    //il percorso dell'arduino è variabile per cui puo essere inserito da utente
    if(argc == 1){
        serial_path = "/dev/ttyACM0";
    }
    else{
        serial_path = argv[1];
    }
    //gestione delle interruzioni, va in cleanup
    struct sigaction ign;
    memset((void*) &ign, 0, sizeof(ign));
    ign.sa_handler = SIG_IGN;
    struct sigaction sig;
    memset((void*) &sig, 0, sizeof(sig)); 
    sig.sa_handler = cleanup;
    sigaction(SIGTERM, &sig, NULL);
    sigaction(SIGHUP, &sig, NULL);
    sigaction(SIGINT, &ign, NULL); //non si interrompe sul control c, in questo modo non ho problemi per farlo partire da shell
    
    
    
    fd = open(serial_path, O_RDONLY | O_NOCTTY);
    if (fd < 0) {
        if(errno == ENOENT){
            printf("il file non si trova in %s, potrebbe non essere inserito\ncompilare con $ ./demone <percorso>\n", serial_path);
            return -1;
        }
        perror("errore nell'apertura del file\n");
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
    opts.c_iflag = 0;
    opts.c_oflag = 0;
    opts.c_lflag = 0;

    // metto i parametri
    tcflush(fd, TCIFLUSH);
    tcsetattr(fd, TCSANOW, &opts);
    
    ret = copia_macro_map();
    if(ret == -1){
        printf("errore nell'inizializzazione delle macro");
        close(fd);
        return -1;
    }
    //inizializzazione thread
    pthread_t thread;
    pthread_create(&thread, NULL, inotify_thread, NULL);
    pthread_detach(thread);
    u_int8_t buf = 0;
    struct timeval tv;
    fd_set readfds;
    while(1){
        ret = 0;
        while(ret == 0){
            
            tv.tv_sec = 5;
            tv.tv_usec = 0;
       
            FD_ZERO(&readfds); //inzializo a zero i bit 
            FD_SET(fd, &readfds); //fd deve essere controllato se è pronto per la lettura
            select(1024, &readfds, NULL, NULL, &tv);
            ret = FD_ISSET(fd, &readfds);
            if(ret < 0){
                ret = -1;
                perror("errore nell'attesa del'input");
                goto CLEANUP;
            }
//            printf("here\n");
        }
        
        
        ret = read(fd, &buf, 1);
        //non gestisco errno==EINTR perche ret=0 vuol dire EOF
        if(ret < 0){
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
        pthread_mutex_lock(&mutex);
        while(scritti < len){
            ret = write(device_fd, macro_map[buf]+scritti, len-scritti);
            if(ret < 0 && errno != EINTR){
                perror("errore nella scrittura sul device\n");
                ret = -1;
                goto CLEANUP;
            }
            else if(errno == EINTR){
                break;
            }
            scritti += ret;
        }
        pthread_mutex_unlock(&mutex);
    }
    //sezione di ritorno per la gestione del cleanup
    ret = 0;
CLEANUP:
    close(fd);            
    close(device_fd);            
    exit(ret);
}
