#include <linux/module.h> 
#include <linux/init.h>   
#include <linux/kernel.h> 
#include <linux/workqueue.h>


static void work_func(struct delayed_work *unused);
static DECLARE_DELAYED_WORK(work, work_func);
static struct workqueue_struct *wq = 0;
static unsigned long onesec;
static int count ;

static void
work_func(struct delayed_work *unused)
{
  printk(KERN_INFO "yee ha timer, %d.\n", ++count);
  queue_delayed_work(wq,&work, HZ);     // one second from now
}

static int hi(void)
{
  printk(KERN_INFO "Hello from timer.\n");
  if (!wq)
    wq = create_singlethread_workqueue("mykmod");
  if (wq)
    queue_delayed_work(wq, &work, HZ);

  //schedule_delayed_work(&work, HZ);
  count = 0 ;
  return 0;
}

static void bye(void)
{
  printk(KERN_INFO "Goodbye from timer.\n");
  if(wq)
  {
    cancel_delayed_work(&work);
    flush_scheduled_work();
    destroy_workqueue(wq);
  }
}

module_init(hi);
module_exit(bye);

MODULE_LICENSE("GPL");