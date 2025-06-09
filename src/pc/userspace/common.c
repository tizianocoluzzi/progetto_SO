#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h> 

#include "common.h"


int inizializza(){
    int ret;
    int macro_fd = open(MACRO_PATH, O_WRONLY | O_CREAT | O_EXCL, 0666);
        for(int i = 0; i < MAX_MACRO; i++){
            char term = '\n';
            ret = write(macro_fd, &term, 1);
            if(ret == -1){
                perror("errore inizializzazione file\n");
                return -1;
            }
        }
        close(macro_fd);

    return 0;
}
