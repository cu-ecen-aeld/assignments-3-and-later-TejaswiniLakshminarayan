/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include <linux/slab.h>
#include <linux/uaccess.h>
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Tejaswini Lakshminarayan"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev* char_ptr;
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    
    char_ptr = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = char_ptr;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    size_t offset_byte = 0;
    size_t remaining_bytes = 0;
    size_t read_bytes = 0;
    struct aesd_buffer_entry* buffer_entry = NULL;
    struct aesd_dev *dev = filp->private_data;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    
    if (mutex_lock_interruptible(&dev->buffer_write_lock)) {
        retval = -ERESTARTSYS;
        return retval;
    }

    buffer_entry = aesd_circular_buffer_find_entry_offset_for_fpos(
        &dev->aesd_buffer, *f_pos, &offset_byte);
    if (buffer_entry == NULL) {
        retval = 0;
        mutex_unlock(&dev->buffer_write_lock);
    	return retval;
    }
    
    remaining_bytes = buffer_entry->size - offset_byte;

    read_bytes = (remaining_bytes > count) ? count : remaining_bytes;

    if (copy_to_user(buf, buffer_entry->buffptr + offset_byte, read_bytes)) {
        retval = -EFAULT;
        mutex_unlock(&dev->buffer_write_lock);
    	return retval;
    }

    *f_pos += read_bytes;
    retval = read_bytes;
    mutex_unlock(&dev->buffer_write_lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    const char * ret_entry = NULL;
    size_t num_bytes;
    struct aesd_dev *dev = NULL;

    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
    dev = filp->private_data;
    if (dev == NULL) {
        //mutex_unlock(&dev->buffer_write_lock);
        return retval;
    }

    if (mutex_lock_interruptible(&dev->buffer_write_lock)) {
        retval = -ERESTARTSYS;
        //mutex_unlock(&dev->buffer_write_lock);
        return retval;
    }

    if (!dev->buffer_entry.size) {
        dev->buffer_entry.buffptr = kzalloc(count, GFP_KERNEL);
    } else {
        dev->buffer_entry.buffptr = krealloc(dev->buffer_entry.buffptr, dev->buffer_entry.size + count, GFP_KERNEL);
    }

    if (!dev->buffer_entry.buffptr) {
        retval = -ENOMEM;
        mutex_unlock(&dev->buffer_write_lock);
        return retval;
    }

    num_bytes = copy_to_user((void *)(&dev->buffer_entry.buffptr[dev->buffer_entry.size]), buf, count);
    if (num_bytes) {
        retval = count - num_bytes;
    } else {
        retval = count;
    }

    dev->buffer_entry.size += retval;
    
    if (strchr((char*)(dev->buffer_entry.buffptr), '\n')) {
        ret_entry = aesd_circular_buffer_add_entry(&dev->aesd_buffer, &dev->buffer_entry);
        if (ret_entry) {
            kfree(ret_entry);
        }
        dev->buffer_entry.size = 0;
        dev->buffer_entry.buffptr = NULL;
    }
    mutex_unlock(&dev->buffer_write_lock);
    return retval;
}
struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    aesd_circular_buffer_init(&aesd_device.aesd_buffer);
    mutex_init(&aesd_device.buffer_write_lock);
    result = aesd_setup_cdev(&aesd_device);
        if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;
}

void aesd_cleanup_module(void)
{
    struct aesd_buffer_entry* buffer_entry;
    uint8_t index;

    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    kfree(aesd_device.buffer_entry.buffptr);
    AESD_CIRCULAR_BUFFER_FOREACH(buffer_entry, &aesd_device.aesd_buffer, index) {
        if (buffer_entry->buffptr != NULL)
            kfree(buffer_entry->buffptr);
    }

    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
