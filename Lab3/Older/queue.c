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
#include "data_item.h"
 #include <linux/semaphore.h>
 #include <linux/mutex.h>


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

static Data_Item *queue;

static int in, out = 0;
static int queue_size = 5, counter=1;
static struct semaphore full, empty;
static struct mutex my_mutex;
/**
 * inits fifo with requested size
 */

static void initFifo(void) {
    // kfree(queue);
    queue = (Data_Item*) kmalloc(sizeof(Data_Item) * queue_size, GFP_KERNEL);
    printk(KERN_INFO "Queue Initialized. Size set as : %d\n", queue_size);
    in = 0; 
    out = 0;
    // queue_size = size;
    //printk(KERN_INFO "JUST BEFORE\n");
    sema_init(&empty, queue_size);
    //printk(KERN_INFO "empty init\n");
    sema_init(&full,0);
    //printk(KERN_INFO "full init\n");
    mutex_init(&my_mutex);
    //printk(KERN_INFO "mutex done\n");
}


/**
 * gets from fifo, no check if it was empty before
 * handles full flag
 */
Data_Item* consume(void) {

    Data_Item *toret; 
    //toret = queue;

    // while(in == out) {

    // } 


    if (down_interruptible(&full))
        return -ERESTARTSYS;
    
    
    if (mutex_lock_interruptible(&my_mutex)) {
        return -ERESTARTSYS;
    }

    toret = &queue[out];
    out = (out + 1) % queue_size;

    printk(KERN_INFO
    "Consumed Item->qid %d \n", toret->qid);

    printk(KERN_INFO
    "Consumed Item->time %llu \n",toret->time);

    printk(KERN_INFO
    "Consumed Item->msg %s \n",toret->msg);
        
    mutex_unlock(&my_mutex);
    up(&empty);
    
    return toret;
}




/**
 * Opens the fifo device, returns -EBUSY when already opened
 */
static int fifo_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO
    "Queue Opened.\n");
    return 0;
}

/**
 *Add Element to Fifo  not checking if full
 * Setting full flag, when it became full
 */
void produce(Data_Item *databufInKernel) {
    printk(KERN_INFO "Adding to FIFO Queue\n");

    // while((in + 1) % queue_size == out)
    // {
    //     printk(KERN_INFO "Queue full");
    //     return;
    // }

    
    

    if (down_interruptible(&empty))
        return -ERESTARTSYS;
    
    if (mutex_lock_interruptible(&my_mutex)) {
        return -ERESTARTSYS;
    }
    printk(KERN_INFO
    "Element number %d being added.\n", counter);


    queue[in].qid = counter++;
    queue[in].time = databufInKernel->time;
    strcpy(queue[in].msg,databufInKernel->msg);

    printk(KERN_INFO
    "Produced Item->qid %d \n", queue[in].qid);

    printk(KERN_INFO
    "Produced Item->time %llu \n",queue[in].time);

    printk(KERN_INFO
    "Produced Item->msg %s \n",queue[in].msg);
    
    in = (in + 1) % queue_size;

    mutex_unlock(&my_mutex);
    up(&full);

    
    printk(KERN_INFO "Add finished. \n");
}

/**
 * writes data to the fifo buffer, if there is space left
 */
static ssize_t fifo_write(struct file *file, const char *buf, size_t count, loff_t *ppos) {
    
    printk(KERN_INFO "Entered FIFO Queue write. \n");

    int procfs_buffer_size = sizeof(char)*count;
    Data_Item* databuf = kmalloc(sizeof(Data_Item), GFP_KERNEL);
    char *tok = kmalloc(procfs_buffer_size + 1, GFP_KERNEL);
    char *end = kmalloc(procfs_buffer_size + 1, GFP_KERNEL);
    
    if (copy_from_user(tok, buf, procfs_buffer_size)) {
        return -EFAULT;
    }


    if (copy_from_user(end, buf, procfs_buffer_size)) {
        return -EFAULT;
    }

    tok[procfs_buffer_size] = '\0';
    end[procfs_buffer_size] = '\0';
    //printk(KERN_INFO
    //"tok copied of size %d from user input %s  \n", procfs_buffer_size, tok);

    //printk(KERN_INFO
    //"end copied of size %d from user input %s  \n", procfs_buffer_size, end);

    
    int j=0;
    while(tok != NULL) {
        tok = strsep(&end, ",");
        if(tok == NULL){
            return -EINVAL;
        }
        switch (j) {
            case 0:
                if(tok == NULL){
                    return -EINVAL;
                }
                kstrtol (tok,10,&(databuf->qid));
                break;
            case 1:
                if(tok == NULL){
                return -EINVAL;
                }
                kstrtoul (tok,10,&(databuf->time));
                break;
            case 2:
                if(strlen(tok) > 50)
                {
                    return -E2BIG;
                }
                strcpy (databuf->msg, tok);
                break;
        }
        tok = end;
        j++;
    }

    
    produce(databuf);

    kfree(tok);
    kfree(end);

    printk(KERN_INFO
    "FIFO Write completes. \n");
    
    return procfs_buffer_size;
}


/**
 * release fifo
 */
static int fifo_release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO
    "FIFO releasing FIFO device.\n");
    
    return 0;

}

/**
 * read the data from the fifo
 * returns EOF as Data when fifo is empty 
 */
static ssize_t fifo_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {

    printk(KERN_INFO "FIFO reading a new element. \n");


    int procfs_buffer_size = count;
    int data_size = sizeof(Data_Item);
    char* my_buf = kmalloc(data_size, GFP_KERNEL);
    if (procfs_buffer_size > data_size) procfs_buffer_size = data_size;
    
    
        Data_Item* d = consume();
        
        const int a = snprintf(NULL, 0, "%d", d->qid);
        char qid[a+1];
        snprintf(qid, a+1, "%d",d->qid);

        const int n = snprintf(NULL, 0, "%lu", d->time);
        char times[n+1];
        snprintf(times, n+1, "%lu",d->time);

        int len = strlen(d->msg);
        char msg[len+1];
        strncpy(msg, d->msg,len);
        msg[len] = '\0';
        
        /*printk(KERN_INFO"Data_Item values from buf are ... %s \n", qid);
        printk(KERN_INFO"Data_Item values from buf are ... %s \n", times);
        printk(KERN_INFO"Data_Item values from buf are ... %s \n", msg);
*/
        strcpy(my_buf, qid);
        strcat(my_buf, ",");
        strcat(my_buf, times);
        strcat(my_buf, ",");
        strcat(my_buf, msg);

        //printk(KERN_INFO"Data_Item values from buf are ... %s \n",my_buf);        

        procfs_buffer_size = strlen(my_buf);
        if (copy_to_user(buf, my_buf, procfs_buffer_size)) {
        return -EFAULT;
    }

    kfree(my_buf);

    return 0;

}


// fops for fifo
static struct file_operations file_ops_fifo = {
        .owner =    THIS_MODULE,
        .read =        fifo_read,
        .write =    fifo_write,
        .open =        fifo_open,
        .release =    fifo_release,
};


// initialize module (executed when using insmod)
static int __init

fifo_module_init(void) {
    int result;
    printk(KERN_INFO
    "Init FIFO...\n");

    initFifo();
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
    kfree(queue);
    // remove an existing ProcFS entry
    // proc_remove(proc_entry_ptr_config);
    printk(KERN_INFO
    "FIFO released proc file...\n");
    unregister_chrdev(MAJOR_NUMBER, "FIFO");
    printk(KERN_INFO
    "FIFO released chrdev majof number %d...\n", MAJOR_NUMBER);
    printk(KERN_INFO
    "FIFO is being unloaded...\n");
}

module_init(fifo_module_init);
//module_param(queue_size, int, 5);
module_exit(fifo_module_cleanup);
EXPORT_SYMBOL_GPL(produce);
EXPORT_SYMBOL_GPL(consume);
