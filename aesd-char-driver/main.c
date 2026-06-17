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
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Juergen Blob"); /** fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("<aesd_open>");
    /**
     * handle open
     */
    filp->private_data = &aesd_device; // Set filp->private_data with our aesd_dev device struct to get access to the device

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("<aesd_release>");
    /**
     * handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("<aesd_read>read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    struct aesd_dev *dev = filp->private_data;
    if (!dev->buffer)
        return 0;

    /* EOF reached, nothing more to read */
    if (*f_pos >= dev->size)
        return 0;

    /* Adjust count if too large */
    if (count > (dev->size - *f_pos))
        count = dev->size - *f_pos;

    /* Copy to user */
    if (copy_to_user(buf, dev->buffer + *f_pos, count))
        return -EFAULT;

    *f_pos += count;
    retval = count;

    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM; // count ?
    PDEBUG("<aesd_write>write %zu bytes with offset %lld",count,*f_pos);

    /**
     * handle write
     */

    /* Basic sanity check */
    if (!buf || count == 0)
        return 0;
	 
	char *kbuf = NULL;
	struct aesd_dev *dev = filp->private_data;

    /* Allocate kernel buffer */
    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf)
    {
        printk(KERN_ERR "<aesdchar>kmalloc failed\n");
        return -ENOMEM;
    }

    /* Copy from user space */
    if (copy_from_user(kbuf, buf, count))
    {
        printk(KERN_ERR "<aesdchar>copy_from_user failed\n");
        kfree(kbuf);
        return -EFAULT;
    }

    /*
     * VERY SIMPLE BEHAVIOUR (for now):
     * Just free old buffer and store new one
     * -> prevents memory leaks
     */
    if (dev->buffer)
    {
        kfree(dev->buffer);
    }

    dev->buffer = kbuf;
    dev->size = count;
	
	retval = count;

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
    result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) 
	{
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */

    result = aesd_setup_cdev(&aesd_device);

    if( result ) 
	{
        unregister_chrdev_region(dev, 1);
    }
	
	mutex_init(&aesd_device.lock);
	
    return result;

}


void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    /**
     * cleanup AESD specific poritions here as necessary
     */

	if (aesd_device.buffer)
		kfree(aesd_device.buffer);
    cdev_del(&aesd_device.cdev);
    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
