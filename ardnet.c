#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/kernel.h>
#include <linux/etherdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/buffer_head.h>

#define NO_ERROR_CODE (0)
#define UNKNOWN_ERROR (-1)

struct net_device *virtualNIC;


struct file *serial_filep;

int serial_open(void) {
    struct file *flip = NULL;
    // mm_segment_t oldfs;

    // oldfs = get_fs();
    // set_fs(get_ds());
    flip = filp_open("/dev/ttyACM0", O_RDWR, 0644);
    // set_fs(oldfs);
    if (IS_ERR(flip)) {
        return UNKNOWN_ERROR;
    }
    serial_filep = flip;
    return NO_ERROR_CODE;
}

void serial_close(void) {
    filp_close(serial_filep, NULL);
}

int serial_read(unsigned char *data, unsigned int size) {
    // mm_segment_t oldfs;
    int ret;
    unsigned long long offset = 0;

    // oldfs = get_fs();
    // set_fs(get_ds());

    // ret = vfs_read(serial_filep, data, size, &offset);
    ret = kernel_read(serial_filep, data, size, &offset);

    // set_fs(oldfs);
    return ret;
}  

int serial_write(unsigned char *data, unsigned int size) {
    // mm_segment_t oldfs;
    unsigned long long offset = 0;

    // oldfs = get_fs();
    // set_fs(get_ds());

    // ret = vfs_write(serial_filep, data, size, &offset);
    kernel_write(serial_filep, data, size, &offset);

    // set_fs(oldfs);
    return NO_ERROR_CODE;
}



int virtualNIC_open(struct net_device *dev) {
    int err = NO_ERROR_CODE;

    printk("virtualNIC_open called\n");
    
    err = serial_open();
    if(err != NO_ERROR_CODE) {
        goto cleanup;
    }
    
cleanup:
    return err;
}

int virtualNIC_release(struct net_device *dev) {
    int err = NO_ERROR_CODE;
    printk("virtualNIC_release called\n");
    netif_stop_queue(dev);

    serial_close();
    
    return err;
}

int virtualNIC_xmit(struct sk_buff *skb, struct net_device *dev) {
    int err = NO_ERROR_CODE;
    printk("dummy xmit function called...\n");
    
    err = serial_write(skb->data, skb->truesize);

    dev_kfree_skb(skb);
    return err;
}

int virtualNIC_init(struct net_device *dev) {
    printk("virtualNIC device initialized\n");
    return NO_ERROR_CODE;
};

const struct net_device_ops my_netdev_ops = {
    .ndo_init = virtualNIC_init,
    .ndo_open = virtualNIC_open,
    .ndo_stop = virtualNIC_release,
    .ndo_start_xmit = virtualNIC_xmit,
};

static void virtual_setup(struct net_device *dev){
    dev->netdev_ops = &my_netdev_ops;
}

int virtualNIC_init_module(void) {
    int result;
    virtualNIC = alloc_netdev(0, "virtnC%d", NET_NAME_UNKNOWN, virtual_setup);
    if((result = register_netdev(virtualNIC))) {
        printk("virtualNIC: Error %d initalizing card ...", result);
        return result;
    }
    return 0;
}

void virtualNIC_cleanup (void) {
    printk ("<0> Cleaning Up the Module\n");
    unregister_netdev (virtualNIC);
}

module_init(virtualNIC_init_module);
module_exit(virtualNIC_cleanup);
MODULE_LICENSE("GPL");