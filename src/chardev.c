/* file kernel object per la creazione del chardev personalizzato che su azione in scrittura simula pressione dei tasti*/
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

/*idea per il parsing delle combinazioni di tasti 
<qualunque cosa contenuta qui dentro è una pressione unica>
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
    ['E'] = KEY_ENTER,
    ['C'] = KEY_LEFTCTRL,
    ['A'] = KEY_LEFTALT
    //... da aggiungere gli altri map

};

//esempio di possibile mappatura
//ad un tasto premuto sull'arduino corrispone una combinazioni di tasti, definita in questo array
static unsigned char* map[16] = {
    "<abc>","<Cs>", "Abcd", "", "", "", "","","","","","","","","",""
};

static void parser(const char* buf){
    u_int8_t stato = 0; //se è 0 siamo in attesa, se 1 siamo in rilascio
    while(*buf != '\0'){
        if(*buf == '<'){
            if(stato == 1 ) { //c'è un errore 
            }
            buf++;
            stato = 1; //segnala che lo stato è in pressione
        }
        else if(*buf == '>' && stato == 1){
            //routine di rilascio, svuotare una eventuale linked listo o un array
            //solo nel caso era stato gia incontrato qualcosa
            stato = 0;
        }
        else if(stato == 1){
            //switch case per la mappatuda dei dati, forse potrei creare un dizionario per l'efficenza
            //esempio, le lettere minuscole corrispondono tutte a KEY_<lettera>, C corrisponde a control, A corrisponde ad alt, T corrisponde a tab, E corrisponde a enter ...
            //è uno switch case bello lungo in realtà
        }
        else{
            //siamo in uno stato non corretto
        }

    }
}

static ssize_t device_write(struct file * file, const char __user * buffer, size_t len, loff_t * offset){
    char kbuf[100]; //allochiamo memoria come se non ci fosse un domani
    int ret = copy_from_user(kbuf+1, buffer, 1);
    if(ret < 0) return -EFAULT;
    //per la formattazione della stringa per strtol  
    kbuf[0] = '+';
    kbuf[2] = '\n';
    kbuf[3] = '\0';
    //test per key in realta questa cosa non deve essere effettuata qui ma in parser, quello che leggon dal dev è l'indice dell'array map
    unsigned char pressed = key[kbuf[0]];
    long res;
    ret = kstrtol(kbuf, 16, &res);
    if(ret == 0){
        printk("il valore letto è: %d\n", ret);
    }
    else if (ret == -EINVAL){
        printk("conversione fallita per errore di conversione\n");
    }
    /*input_report_key(dev, key[kbuf[0]], 1);
    input_sync(dev);
    input_report_key(dev, key[kbuf[0]], 0);
    input_sync(dev);*/

    printk("ho letto: %s\n che corrisponde al valore %u", kbuf, pressed);
    // TODO modo di mappare ogni possibile lettera con una combinazione di tasti
    // TODO chiamata alla funzione parser che invia la pressione
    return 1; 
}

static int my_init(void)
{
	printk("hello - Hello, Kernel!\n");
	
    //crezione di input device 
    dev = input_allocate_device();
    if(!dev) printk("errore nell'inizializzazione");
    
    // inizializzazione
    dev->name = "tiziano_dev";
    dev->id.bustype = BUS_VIRTUAL;
    dev->id.vendor = 0;
    dev->id.product = 0;
    dev->id.version = 0;
    //in teoria dovrebbe abilitare i bit ad essere inviati
    dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REP);
    for (int i = 0; i < KEY_CNT; i++)
        set_bit(i, dev->keybit);

    //registrazione
    int error = input_register_device(dev);
    
    if(error) printk("errore nella registrazione");
    
    /* 
    // creazione charachter device
    // lo zero sta ad indicare che assegna lui il major number
    int major = register_chrdev(0, "tiziano_chardev", &fops);
    
    if(major < 0){
        printk("errore registrando character device \n");
        input_unregister_device(dev); // la pulizia è necessaria
    }*/
    //ALTERNATIVA CON ALLOC
    int res = alloc_chrdev_region(&d, 0, 1, "tiziano_chardev");
    if(res < 0){
        printk("errore in alloc chardev region: %d\n", res);
        return -res;
    }
    major = MAJOR(d);
    cdev_init(&cdev, &fops);
    res = cdev_add(&cdev, MKDEV(major, 0), 1);
    if(res < 0){
        printk("errore in cdev_add %d\n", res);
        unregister_chrdev_region(d, 1);
        return -res;
    }
    chardev_class = class_create("tiziano_chardev");
    if(IS_ERR(chardev_class)){
        printk("errore nella creazione della classe\n");	
        unregister_chrdev_region(d, 1);
        return res;
    }
    res = device_create(chardev_class, NULL, MKDEV(major, 0), NULL, "tiziano_chardev0");
    if(IS_ERR(res)){
        printk("errore nella creazione del device\n");
        class_destroy(chardev_class);
        unregister_chrdev_region(d, 1);
        return res;
    }
    printk("device registrato con major %d\n", major);


    return 0;
}

static void my_exit(void)
{
    // lo lascio al momento come verifica
    /*input_report_key(dev, KEY_A, 1);
    input_sync(dev);
    input_report_key(dev, KEY_A, 0);
    input_sync(dev);*/
	
    device_destroy(chardev_class, MKDEV(major, 0));
    printk("device distrutta\n");
    //class_unregister(chardev_class);
    class_destroy(chardev_class);
    printk("class distrutta\n");

    unregister_chrdev_region(d, 1);
    printk("chardev region distrutta\n");
    // la free non è necessaria se si fa unregister
    input_unregister_device(dev);
    printk("hello - Goodbye, Kernel!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
