/* file kernel object per la creazione del chardev personalizzato che su azione in scrittura simula pressione dei tasti*/
#define pr_fmt(fmt) "%s %s: " fmt, KBUILD_MODNAME, __func__
#include <linux/module.h> // module init e exit
#include <linux/init.h> 
#include <linux/input.h> // per gli input report key e alloc
#include <linux/input-event-codes.h> // per la lista dei vari tasti
#include <linux/fs.h> // per le file oparations
#include <linux/device.h> //per la crezione automatica dei device
#include <linux/cdev.h> //same
 
static ssize_t device_write(struct file * file, const char __user * buffer, size_t len, loff_t * offset); //taccia il compilatore

static struct input_dev* dev; // device simulazione pressione dei tasti
static struct file_operations fops = {.write = device_write}; // per le operazioni sul file

static struct class* chardev_class = NULL; 
static struct cdev cdev;
static dev_t d;
static int major;
static char kchar[1024];
/*parsing delle combinazioni di tasti 
<qualunque cosa contenuta qui dentro è una pressione unica>
non possono esserci cose nested (rende inutile il fatto di avere le parestesi angolate ma magari nel tempo migliorerà)
*/

// in questo modo possiamo accedere facilmente al tasto che andiamo a leggere
static unsigned char key[255] = {
    ['a'] = KEY_A,
    ['b'] = KEY_B,
    ['c'] = KEY_C,
    ['d'] = KEY_D,
    ['e'] = KEY_E,
    ['f'] = KEY_F,
    ['g'] = KEY_G,
    ['h'] = KEY_H,
    ['i'] = KEY_I,
    ['j'] = KEY_J,
    ['k'] = KEY_K,
    ['l'] = KEY_L,
    ['m'] = KEY_M,
    ['n'] = KEY_N,
    ['o'] = KEY_O,
    ['p'] = KEY_P,
    ['q'] = KEY_Q,
    ['r'] = KEY_R,
    ['s'] = KEY_S,
    ['t'] = KEY_T,
    ['u'] = KEY_U,
    ['v'] = KEY_V,
    ['w'] = KEY_W,
    ['x'] = KEY_X,
    ['y'] = KEY_Y,
    ['z'] = KEY_Z,

    ['1'] = KEY_1,
    ['2'] = KEY_2,
    ['3'] = KEY_3,
    ['4'] = KEY_4,
    ['5'] = KEY_5,
    ['6'] = KEY_6,
    ['7'] = KEY_7,
    ['8'] = KEY_8,
    ['9'] = KEY_9,
    ['0'] = KEY_0,

    [' ']  = KEY_SPACE,
    ['\t'] = KEY_TAB,
    [27]   = KEY_ESC,      // ESC in ASCII è 27
    ['-']  = KEY_MINUS,
    ['=']  = KEY_EQUAL,
    ['[']  = KEY_LEFTBRACE,
    [']']  = KEY_RIGHTBRACE,
    [';']  = KEY_SEMICOLON,
    ['\''] = KEY_APOSTROPHE,
    ['`']  = KEY_GRAVE,
    ['\\'] = KEY_BACKSLASH,
    [',']  = KEY_COMMA,
    ['.']  = KEY_DOT,
    ['/']  = KEY_SLASH,
    ['A']  = KEY_RIGHTALT,
    ['C']  = KEY_RIGHTCTRL,
    ['E']  = KEY_ENTER, // non lo mappo con \n
    ['S']  = KEY_RIGHTSHIFT,
    //ce ne sono molti di piu ma al momento ce li facciamo andare bene
};


//esempio di possibile mappatura
//ad un tasto premuto sull'arduino corrispone una combinazioni di tasti, definita in questo array

inline void premi_tasto(const char tasto){
    input_report_key(dev, key[(u_int8_t) tasto], 1);
    input_sync(dev);
}

inline void rilascia_tasto(const char tasto){
    input_report_key(dev, key[(u_int8_t) tasto], 0);
    input_sync(dev);
}

static int check_formato(const char* buf){
/* controllo del formato delle stringhe da parsare
    in questo modo controllo che sia tutto <..><..>..
    */
    int stato = 0;
    while(*buf != 0){
        if(*buf == '<'){
            stato++;
        }
        else if(*buf == '>'){
            stato--;
        }
        if(stato < 0 || stato > 1){
            return -1;
        }
        buf++;
    }
    if(stato == 1)
        return -1;
    return 0;
}

static void parser(const char* buf){
    /*fa traduce la stringa chiamata dal device negli input da tastiera*/
    int err = check_formato(buf);
    if(err){
        pr_warn("errore di formato");
        return;
    }
    u_int8_t stato = 0; //se è 0 siamo in attesa di trovare una stringa in <>, se 1 siamo nel ciclo di pressione e rilascio
    int idx = 0;
    while(*buf != '\0'){
        if(*buf == '<'){
            stato = 1; //segnala che lo stato è in pressione
        }
        else if(*buf == '>' && stato == 1){
            //ciclo di rilascio
            while(*(--buf) != '<'){
                rilascia_tasto(*buf);
                idx += 1;
            }
            //routine di rilascio
            //solo nel caso era stato gia incontrato qualcosa
            buf += idx+1; //torno allo stato precendente il piu uno perche viene conteggiato anche il --buf del while
            idx = 0;
            stato = 0;
        }
        else if(stato == 1){
            //simulazione pressione usando dizionario key
            premi_tasto(*buf);
        }
        else{
            pr_notice("presenza di caratteri esterni, rivedere regole di composizione macro");
            //in questo modo mandiamo avanti comunque la funzione, altrimenti avremmo un caso in cui si crea un loop
            //siamo in uno stato non corretto
        }
        buf++;
    }
}

static ssize_t device_write(struct file * file, const char __user * buffer, size_t len, loff_t * offset){
    /*funzione di scrittura per le fops*/
    int ret = copy_from_user(&kchar, buffer, len); //kchar dichiarato fuori perche superava lo stack locale
    if(ret < 0) return -EFAULT;

    kchar[len-1]='\0';//device_write prende in input una line feed dopo la stringa, viene eliminata
    pr_info("tentativo di esecuzione macro %s, di lunghezza :%d", kchar, len-1); 
    parser(kchar);
    for(int i = 0; i < 1024; i++){
        kchar[i] = '\0';
    }
    // printk("ho letto: %s\n che corrisponde al valore %u", kbuf, pressed);
    return len; 
}

static int my_init(void)
{
	pr_info("Modulo per tastiera macro inserito\n");
	
    //crezione di input device 
    dev = input_allocate_device();
    if(!dev) {
        pr_err( "errore nell'inizializzazione");
        return -1;
    }    
    // inizializzazione
    dev->name = "tiziano_dev";
    dev->id.bustype = BUS_VIRTUAL;
    dev->id.vendor = 0;
    dev->id.product = 0;
    dev->id.version = 0;
    //dovrebbe abilitare i bit ad essere inviati
    dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    for (int i = 0; i < KEY_CNT; i++)
        set_bit(i, dev->keybit);

    //registrazione
    int error = input_register_device(dev);
    
    if(error) {
        pr_err("errore nella registrazione");
        input_free_device(dev);
        return -1;
    }
    
    /* 
    // creazione charachter device
    // lo zero sta ad indicare che assegna lui il major number
    int major = register_chrdev(0, "tiziano_chardev", &fops);
    
    if(major < 0){
        printk("errore registrando character device \n");
        input_unregister_device(dev); // la pulizia è necessaria
    }*/
    //ALTERNATIVA CON ALLOC
    //necessaria poichè il vecchio metodo era deprecato, lo lascio per completezza
    int res = alloc_chrdev_region(&d, 0, 1, "tiziano_chardev");
    if(res < 0){
        pr_err("errore in alloc chardev region: %d\n", res);
        return -res;
    }
    major = MAJOR(d);
    cdev_init(&cdev, &fops);
    res = cdev_add(&cdev, MKDEV(major, 0), 1);
    if(res < 0){
        pr_err("errore in cdev_add %d\n", res);
        unregister_chrdev_region(d, 1);
        return -res;
    }
    chardev_class = class_create("tiziano_chardev");
    if(IS_ERR(chardev_class)){
        pr_err("errore nella creazione della classe\n");	
        unregister_chrdev_region(d, 1);
        return -1;
    }
    void* ret;
    ret = device_create(chardev_class, NULL, MKDEV(major, 0), NULL, "tiziano_chardev0");
    if(IS_ERR(ret)){
        pr_err("errore nella creazione del device\n");
        class_destroy(chardev_class);
        unregister_chrdev_region(d, 1);
        return -1;
    }
    pr_info("device registrato con major %d\n", major);


    return 0;
}

static void my_exit(void)
{
    device_destroy(chardev_class, MKDEV(major, 0));
    pr_info("device distrutta\n");
    //class_unregister(chardev_class);
    class_destroy(chardev_class);
    pr_info("class distrutta\n");

    unregister_chrdev_region(d, 1);
    pr_info("chardev region distrutta\n");
    // la free non è necessaria se si fa unregister
    input_unregister_device(dev);
    pr_info("modulo macro deregistrato correttamente\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
