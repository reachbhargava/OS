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
#include <linux/kernel.h>
#include "data_item.h"

MODULE_AUTHOR("OurGroup");
MODULE_DESCRIPTION("Lab 3 Task 3.1 Solution - FIFO Consumer");
MODULE_LICENSE("GPL");

extern Data_Item* consume(void);

int init_module(void)
{
    printk(KERN_INFO "Consumer Entered\n"); 

    Data_Item* d = consume();

    printk(KERN_INFO"Printed from kernel consumer - qid %d \n", d->qid);
    printk(KERN_INFO"Printed from kernel consumer- time %llu \n", d->time);
    printk(KERN_INFO"Printed from kernel consumer - msg %s \n", d->msg);

    return 0; 
}

void cleanup_module(void)
{
    printk(KERN_INFO "Consumer Exit\n");
}