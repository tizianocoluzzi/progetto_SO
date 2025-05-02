# DIARIO DI BORDO
## introduzione 
Il diario di bordo serve per annoatre ciò che è successo nello sviluppo di questo progetto, cioe le varie situaizoni in cui mi sono trovato e in che modo le ho risolte

## giornate 

### 16.04.2025

#### descrizione
inizio i primi test per il progetto, la prima giornata di test effettivi risale a qualche giorno fa in cui avevo effettivamente provato a creare e caricare nel kernel dei kernel object
il test odierno riguardava semplicemente la funzione `input_report_key()`, per capire se effettivamente si puo utilizzare per simulare la pressione di un tasto, la risposta è positiva 
nostante la sofferenza per capire le funzioni di inizializzazione del dispositivo e come questo viene registrato all'interno del sistema operativo,
alla fine la pressione simulata del tasto a ha riscontro positivo solop nella `__my_exit()` e non nelle `__my_init()`, non so come mai, credo sia un problema da poco poichè non dovrebbe 
essere queste le funzioni che gestiscono la simulazione dei tasti, da come dice chatgpt: il dispositivo nella funzione di init non è ancora stato registrato e quindi il sistema non è ancora pronto a ricevere il suo input

#### tool usati
- `evtest /dev/input/eventX` per vedere quello che succede con l'input, non ho ancora ben capito come si usa
- `insmod` e `rmmod` per inserimento nel kernel
- `cat /proc/bus/input/devices | tail` per vedere i vari devices, in particolare gli ultimi
- `dmesg | tail` per vedere le `printk` del device

#### fonti di informazione
- chatgpt per le domande anche piu stupide che mi venivano in mente, sempre sottovalutato ma molto di aiuto
- youtube nei giorni precedenti ne avevo guardato abbastanza per farmi un idea di tutti i concetti necessari
- classici stackoverflow
- kernel linux, in particolare il file `linux/driver/input/keyboard/atacbd.c` per vedere come inizializzava i vari valori

#### prossimi obiettivi:
- capire come leggere un input dell'arduino

### 17.04.2025

#### descrizione
test obiettivo di oggi superato alla grande, creazione del mio cutecom personale, piu che altro per vedere come funziona la lettura di un file dev, l'unico problema è stato quello
di settare termios, piu che altro perche mi mette il dubbio su come si possa effettivamente implementare in un kernel object, in questo casa si tratta si una cosa che fa polling
suall seriale praticamente, nel caso del driver vero si tratta di creare la funzione che reagisce all'evento di scrittura quindi non so bene in che modo termios possa aiutare e come
implementarlo effettivamente
Ho chiaramente dovuto anche fare un mini script di arduino che inviasse il carattere 'a' a ripetizione, ho usato tutto il materiale del professore

#### tool usati 
- `dmesg` per vedere la connessione di arduino
- `cutecom` per testare il banale funzionamento dell'arduino e vedere se il mio programma mostrava i risultati correttamente
- `avr-toolkit` per il programma di arduino 

#### fonti di informazione
- youtube per vedere la parte di termios
- google per la documentazione di termios
- manuale di linux perche non mi ricordavo nulla 
- slide e source del prof per la parte di avr

#### prossimi obiettivi
indeciso se iniziare effettivamente il progetto oppure fare un test con un kernel object in lettura (creazione quindi del devchar e le fopts) 

### 19.04.2025

#### descrizione 
prima di iniziare lo studio presento il problema: non credo si possa fare la mmap sul `dev/ttyACM0`, device che comunque non esiste sempre quindi andrebbe sicuramente a creare errori nel server, possiamo percorre due strade: 
- creaiamo il driver personalizzato USB, il che vuol dire utilizzare il protocollo USB
- mettiamo il dispositivo arduino nella blacklist del driver dcd-acm standard e creiamo il nostro personale, probabilmente la strada piu semplicema prevede cose meno professionali
- opzione brutta, potrebbe essere la prima da percorrere, creare un demon user-space che copita i dati della tty in un chardev che fa partire poi la simulazione dei tasti
all fine oggi giornata di sole ricerche, il problema grosso di usare il protocollo USB sta nel fatto di dover riprogrammare la atmega16u2, il chip che fa da tramite tra l'arduino e il pc,
si potrebbe fare in modo di leggerlo come un dispositivo HID, è un'operazione molto costosa

#### decrizione pt. 2
in realtà ho iniziato lo sviluppo del progetto nella modalita col demone in userspace, al momento ho iniziato a creare un chardev, ho scopertu successivamente che è un metodo obsoleto
quello che ho ustilizzato adesso, cioe la funzione `register_chardev` perchè prevede che si debba poi allocate il dispositivo in `/dev` manualmente con `insnode`, 
questo può essere risolto con `alloc_chardev_area` (tipo, non ricordo bene), ho creato anche la funzione di read che reagisce a quando si scrive qualcosa sul device con il suo major

#### obiettivi futuri
- finire di scrivere la funzione read, cioe quella che invia la pressione del tasto
- scrivere tutto in maniera sensata e corretta e ordinata
- scrivere il codice di arduino

### 28.04.2025
ho riscritto la creazione del chardev utilizzando le funzioni di allocazione, in questo modo /dev/tiziano_chardev0 si crea in automatico, ancora da controllare bene come funzione la free
di questi dispositivi

#### obiettivi futuri
- gli stessi dell'altra volta
- correggere bene la free del dispositivo

#### 29.04.2025
corretta la funzione di exit, semplicemente avevo commentato la creazione del device di simulazione dei tasti e andandolo ad eliminare dava problemi, 
ho scritto la struttura della funzione parser che ha lo scopo di tradurre una stringa in una pressione di tasti seguendo determinate regole, in modo che possa essere piu modulare
il problema sta nell'associare un tasto a una simulazione di pressione, nel driver atakbd usa un semplice array che mappa un numero a un determinato KEY_<qualcosa>, 
potrebbe essere una buona soluzione per evitare di impazzire
aggiunto il vettore key, con indice dei caratteri, sembra funzionare, ho eseguito diversi test:
```
$ echo "a" > /dev/tiziano_chardev0
--- 
$ echo "b" > /dev/tiziano_chardev0
---
$ echo "e" > /dev/tiziano_chardev0
---
$ echo "E" > /dev/tiziano_chardev0
```
ai quali sono seguite le seguenti riposte dalle printk 
```
[ 4418.659421] ho letto: a
                che corrisponde al valore 30
[ 4418.659429] ho letto: 
               
                che corrisponde al valore 0
[ 4429.383546] ho letto: b
                che corrisponde al valore 48
[ 4429.383550] ho letto: 
               
                che corrisponde al valore 0
[ 4459.409220] ho letto: a
                che corrisponde al valore 30
[ 4459.409234] ho letto: 
               
                che corrisponde al valore 0
[ 4468.032280] ho letto: e
                che corrisponde al valore 18
[ 4468.032291] ho letto: 
               
                che corrisponde al valore 0
[ 4475.003979] ho letto: E
                che corrisponde al valore 28
[ 4475.003984] ho letto: 
               
                che corrisponde al valore 0
``` 
si evidenzia un problema perche legge anche un carattere vuoto dopo, ma si risolverà, potrebbe essere semplicemente legato all'uso delle doppie virgolette
anche aggiungendo l'effettiva simulazione del tasto si riscontra che simula la pressione del tasto corretta, in questo modo abbiamo praticamente un dizionario con accesso in O(1)
ho messo anche una conversione da stringa letta a int in base 16 ma a quanto pare non va, non capisco bene perche

#### obiettivi futuri
- finire il parser
- quelli precedenti
(si accumulano)

### 02.05.2025
giornatina produttiva, finita la parte di codice del chardev, almeno per questa prima versione, finito il parser, corretta la conversione degli indici (non serviva a nulla la kstrol), 
il parser ha alcuni problemi e imprecisioni ma alla fine va bene perche potrei renderlo piu complesso, devo solo trovare un modo sensato per farlo, 
ho creato uno script .sh per compilare,
inserire il kernel module e dare i permessi al device
ho anche aggiornato la divisione in cartelle della src
##### aggiornamento delle 0:05
ho fatto anche la parte della tastiera e il demone che fa da tramite, il codice della tastiera è quasi interamente copiato il codice del professore, 
ho modificato l'invio del segnale solo alla pressione del tasto invece che anche al rilascio, il demone è molto base si tratta di leggere e scrivere in modo molto banale,
a livello fisico ho realizzato la tastiera con soli 8 tasti perche lo spazio sulle breadboard è quello che è, prima o poi la realizzerò interamente

#### obiettivi futuri
- controllare tutto e fare tutti i test necessari
- scrivere lo schema della tastiera e documentare tutto
- provare a trovare la soluzione unica senza demone
