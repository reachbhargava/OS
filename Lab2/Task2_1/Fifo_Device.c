/**
 * Fifo_Device for Lab2 Task 2.1
 *  - Blocks access if accessed
 *  - Returns error when size of data to write is bigger than free space in fifo
 *  - returns EOF as data when fifo is read and empty
 *  - fifo is a ringbuffer
 *  - fifo are 2 devices with major 240 and minor 1 and 2 distinguished by file access rights, see setup.txt on how to set them up
 * 
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/slab.h>


#define FILE_ACCESS_RONLY    0444        // (r-- r-- r--)
#define FILE_ACCESS_RW        0666        // (rw- rw- rw-)
#define FILE_ACCESS_WONLY    0222        // (-w- -w- -w-)

#define MAJOR_NUMBER 240
#define WRITE_END_MINOR 0
#define READ_END_MINOR 1

#define CONFIG_FILE "fifo_config"

MODULE_AUTHOR("OurGroup");
MODULE_DESCRIPTION("Lab 2 Task 2.1 Solution - FIFO Device Driver");
MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry_ptr_config;


static char *fifo;
static int fifo_size = 4;
//size of fifo
static int fifo_start = 0;
//first used place
static int fifo_end = 0;
//next free place
static int fifo_fill = 0;
//how much places are filled
static int fifo_full = 0;
//full
//value indicating if access to fifo function is currently blocked
static int block_access = 0;
static int write_count = 0, read_count = 0;

/**
 * inits fifo with requested size
 */
static void initFifo(unsigned int size) {
    kfree(fifo);
    fifo = kmalloc(sizeof(char) * size, GFP_KERNEL);
    fifo_size = size;
    fifo_start = 0;
    fifo_end = 0;
    fifo_fill = 0;
    fifo_full = 0;
    printk(KERN_INFO
    "FIFO set fifo size to %d\n", size);
}


/**
 * gets from fifo, no check if it was empty before
 * handles full flag
 */
static char popFromFifo(void) {
    char toret = fifo[fifo_start];
    fifo_start++;
    fifo_start = fifo_start % fifo_size;
    if (fifo_end != fifo_start)fifo_full = 0;
    fifo_fill--;
    read_count++;
    return toret;
}


/**
 *Add Element to Fifo  not checking if full
 * Setting full flag, when it became full
 */
static void addToFifo(char thing) {
    fifo[fifo_end] = thing;
    fifo_end++;
    fifo_end = fifo_end % fifo_size;
    if (fifo_end == fifo_start)fifo_full = 1;
    fifo_fill++;
    write_count++;
}


/**
 * Opens the fifo device, returns -EBUSY when already opened
 */
static int fifo_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO
    "FIFO opening FIFO device ...\n");
    if (block_access == 1) {
        printk(KERN_ERR
        "FIFO could not open, device is busy ...\n");
        return -EBUSY;
    }
    block_access = 1;
    return 0;
}


/**
 * writes data to the fifo buffer, if there is space left
 */
static ssize_t fifo_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    int i;
    int procfs_buffer_size = count;
    char *databuf;
    printk(KERN_INFO
    "FIFO writing FIFO device ...\n");
    //if(iminor(file->f_path.dentry->d_inode) == 0) return 0;

    /* get buffer size */



    if (procfs_buffer_size > fifo_size - fifo_fill) {
        printk(KERN_WARNING
        "FIFO Input too large input (%zu) > free places (%d)\n", count, fifo_size - fifo_fill);
        return -EFAULT;
    }
    //should change to count
    databuf = kmalloc(sizeof(char) * (fifo_size - fifo_fill), GFP_KERNEL);
    /* write data to the buffer */
    if (copy_from_user(databuf, buf, procfs_buffer_size)) {
        return -EFAULT;
    }

    for (i = 0; i < procfs_buffer_size; i++) {
        addToFifo(databuf[i]);
    }
    kfree(databuf);

    return procfs_buffer_size;
}

/**
 * release fifo
 */
static int fifo_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO
    "FIFO releasing FIFO device ...\n");
    block_access = 0;
    return 0;

}

/**
 * read the data from the fifo
 * returns EOF as Data when fifo is empty 
 */
static ssize_t fifo_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

    //if(iminor(filp->f_path.dentry->d_inode) == 1) return 0;
    int procfs_buffer_size = count;
    int i;
    printk(KERN_INFO
    "FIFO reading FIFO device ...\n");
    if (procfs_buffer_size > fifo_fill)procfs_buffer_size = fifo_fill;

    //if(isFifoEmpty()){
    //procfs_buffer_size = 1;
    //buf[1] = 0; //EOF
    //}//else read data from fifo
    //else
    for (i = 0; i < procfs_buffer_size; i++) {
        char d = popFromFifo();
        buf[i] = d;
    }
    //return eof when fifo empty

    return procfs_buffer_size;

}


// fops for fifo
static struct file_operations file_ops_fifo = {
        .owner =    THIS_MODULE,
        .read =        fifo_read,
        .write =    fifo_write,
        .open =        fifo_open,
        .release =    fifo_release,
};


/**
 * sequential read function for the config stats
 */
static int config_read(struct seq_file *m, void *v) {
    printk(KERN_INFO
    "FIFO Reading config\n");

    seq_printf(m, "Fifo_size:\t%d\nFifo_fill:\t%d\ntotal_write:\t%d\ntotal_read\t%d\n", fifo_size, fifo_fill,
               write_count, read_count);
    return 0;
}


/**
 * config file opened
 */
static int config_open(struct inode *inode, struct file *file) {
    return single_open(file, config_read, NULL);
}

/**
 * writing to config file
 * Expecting a single number as string
 * tries to convert it to an integer
 * the integer should be in the given bounds
 */
static ssize_t config_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    long int newsize;
    int err;
    char *databuf;

    if (fifo_fill > 0) {
        printk(KERN_WARNING
        "Fifo not empty, cant change buffer size. fifo_fill=%d\n", fifo_fill);
        return -EBUSY;
    }

    if (block_access != 0) {
        printk(KERN_WARNING
        "Fifo currently in use, cant change buffer size\n");
        return -EBUSY;
    }
    printk(KERN_INFO
    "FIFO writing config ...\n");
    if (count > sizeof(char) * 4) {
        printk(KERN_INFO
        "FIFO  got more data on config_write than expected. ignoring ...\n");
        return -EFAULT;
    }
    databuf = kmalloc(sizeof(char) * count + 1, GFP_KERNEL);
    databuf[count] = 0;
    /* write data to the buffer */
    if (copy_from_user(databuf, buf, count)) {
        printk(KERN_ERR
        "FIFO  could not copy data from user\n");
        return -EFAULT;
    }


    err = kstrtol(databuf, 10, &newsize);

    if (err != 0 || newsize > 4096 || newsize < 4) {
        printk(KERN_ERR
        "FIFO  supllied buffer size value for config too large: converted string %s to %ld (expected between 4 and 4098) ...\n", databuf, newsize);
        kfree(databuf);
        return -EFAULT;
    }
    kfree(databuf);
    initFifo(newsize);
    return count;
}


/**
 * sequential operations for config file
 */


//fops for config file
static struct file_operations file_ops_config = {
        .owner =    THIS_MODULE,
//	.read =		clock_config_read,
        .read =        seq_read,
        .write =    config_write,
        .open =        config_open,
        .release =    single_release,
};


// initialize module (executed when using insmod)
static int __init

fifo_module_init(void) {
    int result;
    printk(KERN_INFO
    "Init FIFO...\n");

    initFifo(8);//default size
    //todo init fifo array, write fifo funs


    // create a new ProcFS file
    proc_entry_ptr_config = proc_create(CONFIG_FILE, FILE_ACCESS_RW, NULL, &file_ops_config);
    printk(KERN_INFO
    "FIFO has created the file object:%s\n", CONFIG_FILE);

    //register this driver for MAJOR NUMBER
    result = register_chrdev(MAJOR_NUMBER, "FIFO", &file_ops_fifo);
    if (result < 0) {
        printk(KERN_ERR
        "FIFO cannot obtain major number %d\n", MAJOR_NUMBER);
        return result;
    } else printk(KERN_INFO
    "FIFO has obtained major number: %d\n", MAJOR_NUMBER);

    return 0;
}

// cleanup module (executed when using rmmod)
static void __exit

fifo_module_cleanup(void) {
    kfree(fifo);
    // remove an existing ProcFS entry
    proc_remove(proc_entry_ptr_config);
    printk(KERN_INFO
    "FIFO released proc file...\n");
    unregister_chrdev(MAJOR_NUMBER, "FIFO");
    printk(KERN_INFO
    "FIFO released chrdev majof number %d...\n", MAJOR_NUMBER);
    printk(KERN_INFO
    "FIFO is being unloaded...\n");
}

module_init(fifo_module_init);
module_exit(fifo_module_cleanup);