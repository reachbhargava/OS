#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>

#define FILE_NAME		"deeds_clock"
#define FILE_ACCESS_MODE	0444		// (r-- r-- r--) The owner ro, others - read only

MODULE_AUTHOR("R");
MODULE_DESCRIPTION("Lab 2 Solution - Displays time in seconds");
MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry_ptr;


// creates output i.e. current time for deeds_clock
static int clock_show(struct seq_file *m, void *v)
{
	static struct timeval clock_time;	

	printk(KERN_INFO "inside clock_show() \n");
	
	do_gettimeofday(&clock_time);
	
	seq_printf(m, "current time: %llu seconds \n", (long long unsigned int)clock_time.tv_sec);

	return 0;
}


// this method is called whenever the module is being used
// e.g. for both read and write operations
static int clock_module_open(struct inode * inode, struct file * file)
{
	 return single_open(file, clock_show, NULL);
}


// module's file operations, a module may need more of these
static struct file_operations file_ops = {
	.owner =	THIS_MODULE,
//	.read =		clock_module_read,
	.read =		seq_read,
//	.write =	clock_module_write,
	.open =		clock_module_open,
	.release =	single_release,
};

// initialize module (executed when using insmod)
static int __init clock_module_init(void)
{
	printk(KERN_INFO "MyClockModule is being loaded...\n");

	// create a new ProcFS file
	proc_entry_ptr = proc_create(FILE_NAME, FILE_ACCESS_MODE, NULL, &file_ops);
	printk(KERN_INFO "MyClockModule has created the file object: /proc/deeds_clock.\n");

	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit clock_module_cleanup(void)
{
	// remove an existing ProcFS entry
	proc_remove(proc_entry_ptr);	// not offered by my kernel ver
    //remove_proc_entry(FILE_NAME, proc_entry_ptr);

	printk(KERN_INFO "MyClockModule is being unloaded...\n");
}

module_init(clock_module_init);
module_exit(clock_module_cleanup);