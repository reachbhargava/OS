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
MODULE_DESCRIPTION("Lab 3 Task 3.1 Solution - Producer Module");
MODULE_LICENSE("GPL");

extern void produce(Data_Item *databufInKernel);
static void timed_produce(struct delayed_work *default_work_item);
static DECLARE_DELAYED_WORK(produce_work, timed_produce);
static struct workqueue_struct *wq = 0;

static int qid=1;
static unsigned long long times;
static char *msg = "Default Message - Producer";
static char *name = "Producer 1";
static int items_per_second = 1;
static unsigned long rate_of_produce=1;

static void
timed_produce(struct delayed_work *default_work_item)
{
    printk(KERN_INFO "Invoking the producer work module based on configured rate");
    struct timeval clock_time;
    do_gettimeofday(&clock_time);
    times = (long long unsigned int)clock_time.tv_sec;

    Data_Item* databuf = kmalloc(sizeof(Data_Item), GFP_KERNEL);
    databuf->qid = qid;
    databuf->time = times;
    strcpy (databuf->msg, msg);

printk(KERN_INFO "[%s][%u][%llu]\t%s\n", name, qid, times, msg);

    produce(databuf);
	
    queue_delayed_work(wq,&produce_work,rate_of_produce); 

}

int init_module(void)
{
    printk(KERN_INFO "Producer Entered.\n");

    rate_of_produce = (long)HZ/items_per_second;

    if (!wq)
        wq = create_singlethread_workqueue("producer_queue");
    if (wq)
        queue_delayed_work(wq, &produce_work, rate_of_produce);

    return 0;
}

void cleanup_module(void)
{
    printk(KERN_INFO "Producer Exit\n");
    if(wq)
    {
        cancel_delayed_work(&produce_work);
        flush_scheduled_work();
        destroy_workqueue(wq);
    }
}

module_param(msg, charp , 0000);
module_param(name, charp, 0000);
module_param(items_per_second, int, S_IWUSR);