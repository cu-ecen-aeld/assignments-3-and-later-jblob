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
#include <linux/slab.h> 
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
    struct aesd_dev *dev = filp->private_data;
    ssize_t retval = 0;
    size_t entry_offset = 0;
    size_t bytes_copied = 0;

    struct aesd_buffer_entry *entry;

    mutex_lock(&dev->lock);

    /* Find entry based on file position */
    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, *f_pos, &entry_offset);

    if (!entry)
        goto out;

    while (entry && bytes_copied < count)
    {
        size_t bytes_available = entry->size - entry_offset;
        size_t bytes_to_copy;

        /* respect count */
        bytes_to_copy = min(count - bytes_copied, bytes_available);

        if (copy_to_user(buf + bytes_copied, entry->buffptr + entry_offset, bytes_to_copy))
        {
            retval = -EFAULT;
            goto out;
        }

        bytes_copied += bytes_to_copy;
        *f_pos += bytes_to_copy;

        /* move to next entry */
        entry_offset = 0;
        entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, *f_pos, &entry_offset);
    }

    retval = bytes_copied;

out:
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{

    struct aesd_dev *dev = filp->private_data;
    char *kbuf = NULL;
    char *newbuf = NULL;
    ssize_t retval = count;
    bool found_newline = false;
    size_t i;

    mutex_lock(&dev->lock);

    /* Allocate buffer for incoming data */
    kbuf = kmalloc(count, GFP_KERNEL);
    if (!kbuf) 
    {
        retval = -ENOMEM;
        goto END;
    }

    if (copy_from_user(kbuf, buf, count)) 
    {
        kfree(kbuf);
        retval = -EFAULT;
        goto END;
    }

    /* Check for newline */
    for (i = 0; i < count; i++) 
    {
        if (kbuf[i] == '\n') 
        {
            found_newline = true;
            break;
        }
    }

    /* Append to partial buffer */
    newbuf = kmalloc(dev->size_partial + count, GFP_KERNEL);
    if (!newbuf) 
    {
        kfree(kbuf);
        retval = -ENOMEM;
        goto END;
    }

    /* Copy old partial */
    if (dev->buffer_partial) 
    {
        memcpy(newbuf, dev->buffer_partial, dev->size_partial);
        kfree(dev->buffer_partial);
    }

    /* Append new data */
    memcpy(newbuf + dev->size_partial, kbuf, count);

    dev->buffer_partial = newbuf;
    dev->size_partial += count;

    kfree(kbuf);

    /* If newline found → finalize command */
    if (found_newline) 
    {
		struct aesd_buffer_entry entry;
		const char *old_buff;

		/* prepare entry */
		entry.buffptr = dev->buffer_partial;
		entry.size = dev->size_partial;

		/* add to circular buffer */
		old_buff = aesd_circular_buffer_add_entry(&dev->buffer, &entry);
		// NOTE: aesd_circular_buffer_add_entry saves the new entry, might override the old entry, returns the old pointer

		/* free overwritten entry if buffer full */
		if (old_buff)
			kfree(old_buff);

		/* reset partial */
		dev->buffer_partial = NULL;
		dev->size_partial = 0;
    }

END:
    mutex_unlock(&dev->lock);
    return retval;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .llseek  =  aesd_llseek,
    .unlocked_ioctl = aesd_unlocked_ioctl,
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
     * initialize the AESD specific portion of the device
     */

    result = aesd_setup_cdev(&aesd_device);

    if( result ) 
	{
        unregister_chrdev_region(dev, 1);
    }
	
	mutex_init(&aesd_device.lock);
	
	aesd_circular_buffer_init(&aesd_device.buffer);
	
    return result;

}


void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    /**
     * cleanup AESD specific poritions here as necessary
     */
	if (aesd_device.buffer_partial)
		kfree(aesd_device.buffer_partial);

	struct aesd_buffer_entry *entry;
	uint8_t index;

	AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index) 
	{
		if (entry->buffptr)
			kfree(entry->buffptr);
	}

    cdev_del(&aesd_device.cdev);
    unregister_chrdev_region(devno, 1);
}


loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
{
    struct aesd_dev *dev = filp->private_data;
    loff_t newpos;
    size_t total_size = 0;
    int i;

    // calculate total size
    for (i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) 
    {
        if (dev->buffer.entry[i].buffptr != NULL) 
        {
            total_size += dev->buffer.entry[i].size; // use buffer_entries, not f_pos !!!
        }
    }

    switch (whence) 
    {
        case SEEK_SET:
            newpos = offset;
            break;

        case SEEK_CUR:
            newpos = filp->f_pos + offset;
            break;

        case SEEK_END:
            newpos = total_size + offset;
            break;

        default:
            return -EINVAL;
    }

    if (newpos < 0)
        return -EINVAL;

    if (newpos > total_size)
        newpos = total_size;

    filp->f_pos = newpos;
    return newpos;
}

long aesd_unlocked_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct aesd_dev *dev = filp->private_data;
    struct aesd_seekto seekto;
    struct aesd_buffer_entry *entry;
    size_t offset = 0;
    uint32_t index = 0;
    uint32_t i;

    if (_IOC_TYPE(cmd) != AESD_IOC_MAGIC)
        return -ENOTTY;

    if (_IOC_NR(cmd) > AESDCHAR_IOC_MAXNR)
        return -ENOTTY;

    switch (cmd) 
    {
        case AESDCHAR_IOCSEEKTO:

            if (copy_from_user(&seekto, (const void __user *)arg, sizeof(seekto)))
                return -EFAULT;

            mutex_lock(&dev->lock);

            for (i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) 
            {
                entry = &dev->buffer.entry[(dev->buffer.out_offs + i) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED];

                if (!entry->buffptr)
                    continue;

                if (index == seekto.write_cmd)
                    break;

                offset += entry->size;
                index++;
            }

            if (!entry->buffptr) 
            {
                mutex_unlock(&dev->lock);
                return -EINVAL;
            }

            if (seekto.write_cmd_offset >= entry->size) 
            {
                mutex_unlock(&dev->lock);
                return -EINVAL;
            }

            filp->f_pos = offset + seekto.write_cmd_offset;

            mutex_unlock(&dev->lock);

            return 0;

        default:
            return -ENOTTY;
    }
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
