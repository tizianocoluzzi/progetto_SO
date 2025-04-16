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

#### finti di informazione
- chatgpt per le domande anche piu stupide che mi venivano in mente, sempre sottovalutato ma molto di aiuto
- youtube nei giorni precedenti ne avevo guardato abbastanza per farmi un idea di tutti i concetti necessari
- classici stackoverflow
- kernel linux, in particolare il file `linux/driver/input/keyboard/atacbd.c` per vedere come inizializzava i vari valori

#### prossimi obiettivi:
- capire come leggere un input dell'arduino
