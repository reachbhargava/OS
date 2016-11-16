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
MODULE_DESCRIPTION("Lab 3 Task 3.1 Solution - FIFO Device Driver");
MODULE_LICENSE("GPL");

extern void produce(Data_Item *databufInKernel);

static int qid=1;
static unsigned long long times;
static char* msg;

int init_module(void)
{
    printk(KERN_INFO "Producer Entered.\n");

    static struct timeval clock_time;
    do_gettimeofday(&clock_time);
    times = (long long unsigned int)clock_time.tv_sec;

    msg = kmalloc(20, GFP_KERNEL);
    msg = "Message from Producer";

    Data_Item* databuf = kmalloc(sizeof(Data_Item), GFP_KERNEL);
    databuf->qid = qid;
    databuf->time = times;
    strcpy (databuf->msg, msg);

    produce(databuf);

    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Producer Exit\n");
}