#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "common.h"

static char macro_map[MAX_MACRO][MAX_MACRO_LEN];

int leggi_regole(){
    char c;
    int fd = open("docs.txt", O_RDONLY, 0);
    if(fd == -1){
        perror("errore nell'apertura della documentazione\n");
        return -1;
    }
    while(read(fd,&c,1 ) == 1){
        putc(c, stdout);
    }
    close(fd);
    return 0;
}

int popola(){
    int ret;
    int fd = open(MACRO_PATH, O_RDONLY, 0);
    if(fd < 0){
        if(errno == ENOENT){
            ret = inizializza();
            if(ret < 0){
                perror("errore nell'inizializzazione del file");
                return -1;
            }           
        }
        else{
            perror("non è stato possibile aprire il file");
            return -1;
        }
    }
    
    for (int i = 0; i < MAX_MACRO; i++){
        int j = 0;
        while(j < MAX_MACRO_LEN-1){
            ret = read(fd, &(macro_map[i][j]), 1);
            if(macro_map[i][j++] == '\n') break; //da rivedere l'efficienza
            if(ret == 0) break;
            if(ret < 0){ //la gestione di EINTR è da fare
                perror("errore nella lettura del file delle macro");
                close(fd);
                return -1; 
            }
            
        }
        //aggiunta terminatore stringa
        macro_map[i][j] = '\0';
    }
    close(fd);
    return 0;

}

void elenco_macro(){
    int ret;
    for (int i = 0; i < MAX_MACRO; i++){
        printf("macro[%d]: %s", i, macro_map[i]);
    }
}

void modifica_macro(){
    int ret;
    
    //stampa macro_map    
    for (int i = 0; i < MAX_MACRO; i++){
        printf("macro[%d]: %s", i, macro_map[i]);
    }
    // TODO renderlo interattivo con la tastiera
    printf("\ninserisci il numero della macro che vuoi modificare [0-15]:\n");
    int i;
INPUT2:
    scanf("%d", &i);
    if(i > 15 || i < 0){
        printf("inserisci un numero da 0 a 15\n");
        while(getchar() != '\n'); //svuoto il buffer dello stdin
        goto INPUT2;
    }
    printf("inserisci una stringa, formato: <...><...>\n");
    scanf("%1022s", macro_map[i]);
    int len = strlen(macro_map[i]);
    macro_map[i][len] = '\n';
    macro_map[i][len+1] = '\0';
    printf("hai inserito %s\n", macro_map[i]);
    
    int fd = open(MACRO_PATH, O_WRONLY | O_TRUNC, 0);
    if(fd < 0){
        perror("non è stato possibile aprire il file:");
        return;
    }
    for(int j = 0; j < MAX_MACRO; j++){
        //printf("provo a scrivere %s", macro_map[j]);
        len = strlen(macro_map[j]);
        ret = 0;
        int scritti = 0;
        while(scritti < len){
            ret = write(fd, macro_map[j] + scritti, len-scritti);
            if(ret < 1){
                if(errno == EINTR){
                    continue;
                }
                perror("errore in scrittura");
                close(fd);
                return;
            }
            scritti += ret;
        }
        //printf("ho scrito %d byte\n", scritti);
        //printf("ho scritto: %s", macro_map[j]);
    }

    close(fd);
    printf("modifica avvenuta con successo, per vedere gli effetti ricorda di riavviare il demone\n");
}

int main(){    
    int ret = popola();
    if(ret == -1){
        printf("errore nella lettura del file\n");
        return -1;
    }
    while(1){
        printf("-----personalizzazione macro-----\n");
        printf("seleziona una delle seguenti opzioni:\n[1]vedi le macro attuali\n[2]registra una macro\n[3]vedi le regole di formattazione\n[4]esci\n");
        int i = 0;
        char input[16]; //nel caso voglio inserire altre opzioni
INPUT:
        if( fgets(input, 16, stdin) == NULL){
            perror("errore nell'inserimento del numero");
            return -1;
        }
        sscanf(input, "%d", &i);
        if(i == 1){
            printf("lista delle macro\n");
            elenco_macro();
        }
        else if(i == 2){
            printf("personalizzazione\n");
            modifica_macro();
        }
        else if(i == 3){
            leggi_regole();
        }
        else if(i == 4){
            return 0;
        }
        else{
            printf("inserisci un argomento valido\n");
            goto INPUT;
        }

    }   
    }
        
