/* file kernel object per la creazione del chardev personalizzato che su azione in scrittura simula pressione dei tasti*/
#include <linux/module.h> // module init e exit
#include <linux/init.h> 
#include <linux/input.h> // per gli input report key e alloc
#include <linux/input-event-codes.h> // per la lista dei vari tasti
#include <linux/fs.h> // per le file oparations

static ssize_t device_read(struct file * file, const char __user * buffer, size_t len, loff_t * offset); //taccia il compilatore

static struct input_dev* dev; // device simulazione pressione dei tasti
static struct file_operations fops = {.write = device_read }; // per le operazioni sul file

static ssize_t device_read(struct file * file, const char __user * buffer, size_t len, loff_t * offset){
    char kbuf[100]; //allochiamo memoria come se non ci fosse un domani
    int ret = copy_from_user(kbuf, buffer, 1);
    if(ret < 0) return -EFAULT;
    kbuf[1] = '\0';
    printk("ho letto: %s\n", kbuf);
    // TODO pressione dei tatsi con switch case
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
    
    // creazione charachter device
    // lo zero sta ad indicare che assegna lui il major number
    int major = register_chrdev(0, "tiziano_chardev", &fops);
    if(major < 0){
        printk("errore registrando character device \n");
        input_unregister_device(dev); // la pulizia è necessaria
    }
    printk("device registrato con major %d\n", major);


    return 0;
}

static void my_exit(void)
{
    // lo lascio al momento come verifica
    input_report_key(dev, KEY_A, 1);
    input_sync(dev);
    input_report_key(dev, KEY_A, 0);
    input_sync(dev);
    // la free non è necessaria se si fa unregister
	input_unregister_device(dev);
    printk("hello - Goodbye, Kernel!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
