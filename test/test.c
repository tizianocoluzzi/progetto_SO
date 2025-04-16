#include <linux/module.h> // module init e exit
#include <linux/init.h> 
#include <linux/input.h> // per gli input report key e alloc
#include <linux/input-event-codes.h> // per la lista dei vari tasti

struct input_dev* dev;
static int my_init(void)
{
	printk("hello - Hello, Kernel!\n");
	
    //crezione di input device 
    dev = input_allocate_device();
    if(!dev) printk("errore nell'inizializzazione");
    
    // inizializzazione
    dev->name = "test_dev";
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
    // in teoria dovrebbe inviare il tasto A
    // non viene inviato perche il dispositivo non è ancora effettivamente inizializzato
    input_report_key(dev, KEY_A, 1);
    input_sync(dev);
    input_report_key(dev, KEY_A, 0);
    input_sync(dev);
    return 0;
}

static void my_exit(void)
{
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
