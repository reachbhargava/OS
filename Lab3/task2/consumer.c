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


static void timed_consume(struct delayed_work *default_work_item);
static DECLARE_DELAYED_WORK(consume_work, timed_consume);
static struct workqueue_struct *wq = 0;
static char *name = "Consumer 1";
static unsigned long items_per_second=1;
static unsigned long rate_of_consume=1;


static void
timed_consume(struct delayed_work *default_work_item)
{
    printk(KERN_INFO "Invoking the consumer work module based on configured rate");
    
    Data_Item* d = consume();

    // printk(KERN_INFO"Printed from kernel consumer - qid %d \n", d->qid);
    // printk(KERN_INFO"Printed from kernel consumer- time %llu \n", d->time);
    // printk(KERN_INFO"Printed from kernel consumer - msg %s \n", d->msg);

printk(KERN_INFO "[%s][%u][%llu]\t%s\n", name, d->qid, d->time, d->msg);

    queue_delayed_work(wq,&consume_work,rate_of_consume); 

}

int init_module(void)
{
    printk(KERN_INFO "Consumer Entered\n"); 

   rate_of_consume = (long)HZ/items_per_second;

    if (!wq)
        wq = create_singlethread_workqueue("consumer_queue");
    if (wq)
        queue_delayed_work(wq, &consume_work, rate_of_consume);

    return 0; 
}

void cleanup_module(void)
{
    printk(KERN_INFO "Consumer Exit\n");
    if(wq)
  	{
	    cancel_delayed_work(&consume_work);
	    flush_scheduled_work();
	    destroy_workqueue(wq);
  	}
}

module_param(name, charp, 0000);
module_param(items_per_second, int, S_IWUSR);