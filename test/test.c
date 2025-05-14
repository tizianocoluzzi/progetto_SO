#include <linux/module.h> // module init e exit
#include <linux/init.h>
#include <linux/usb.h>


// ottenuta da lsusb: Bus 001 Device 010: ID 2341:0042 Arduino SA Mega 2560 R3 (CDC ACM)

#define VENDOR_ID 2341
#define PRODUCT_ID 0042

//dichiarazione di quali dispositvi 
static struct usb_device_id usb_table[] = {
    {USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    {},
};
//per attaccare effettivamente questo driver
MODULE_DEVICE_TABLE(usb, usb_table);

static int probe(struct usb_interface *interface, const struct usb_device_id *id){
    printk("dispositivo riconosciuto");
    return 0;
}

static void disconnect(struct usb_interface *interface){
    printk("disconnetto");
}

static struct usb_driver driver = {
    .name = "macro_keyboard",
    .id_table = usb_table,
    .probe = probe,
    .disconnect = disconnect
};

static int my_init(void){
    int res = usb_register(&driver);
	printk("Init function\n");
    if(res){
        printk("errore durante la registrazione del device");
        return (-res);
    }
    printk("dispositivo registrato");
    return 0;
}

static void my_exit(void)
{
    usb_deregister(&driver);
    printk("hello - Goodbye, Kernel!\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
