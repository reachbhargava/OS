#include <linux/module.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#define FILE_MAIN		"deeds_clock"
#define FILE_CONFIG		"deeds_clock_config"
#define FILE_ACCESS_RONLY	0444		// (r-- r-- r--) The owner rw, others - read only
#define FILE_ACCESS_RW		0666		// (rw- rw- rw-)
#define PROCFS_MAX_LEN		2

MODULE_AUTHOR("R");
MODULE_DESCRIPTION("Lab 2 Solution - Displays time in seconds or in predefined format");
MODULE_LICENSE("GPL");

static struct proc_dir_entry *proc_entry_ptr_main;
static struct proc_dir_entry *proc_entry_ptr_config;

static char procfs_buf[PROCFS_MAX_LEN];
static unsigned long procfs_buf_size = 0;

static int clock_format = 1;	// default to 1 i.e. to "yyyy-mm-dd hh:MM:sec" format

// creates output i.e. current time for deeds_clock
static int clock_show(struct seq_file *m, void *v)
{
	static struct timeval clock_time;

	static struct tm formatted_time;
	
	printk(KERN_INFO "inside clock_show() \n");
	
	do_gettimeofday(&clock_time);

	time_to_tm((time_t)clock_time.tv_sec, 3600, &formatted_time);	// offset:3600 secs, since CET = UTC + 1 hour
	
	if(clock_format == 0)
	{
		seq_printf(m, "current time: %llu seconds \n", (long long unsigned int)clock_time.tv_sec);
	}
	else
	{
		seq_printf(m, "current time: %d-%d-%d %d:%d:%d \n", (int)(formatted_time.tm_year+1900), (int)(formatted_time.tm_mon + 1), formatted_time.tm_mday, formatted_time.tm_hour, formatted_time.tm_min, formatted_time.tm_sec);
	}

	return 0;
}

static int clock_config_show(struct seq_file *m, void *v)
{
	seq_printf(m, "current clock format: %d\n", clock_format);	
	return 0;
}


// this method is called whenever the module is being used
// e.g. for both read and write operations
static int clock_module_open(struct inode * inode, struct file * file)
{
	 return single_open(file, clock_show, NULL);
}


static int clock_config_open(struct inode * inode, struct file * file)
{
	 return single_open(file, clock_config_show, NULL);
}


/*
// this method is executed when reading from the module
static ssize_t clock_config_read(struct file *file, char *buf, size_t count, loff_t *ppos)
{
	static char done = 'n';

	printk(KERN_INFO "inside clock_config_read() \n");	

	if ( done == '\0' ) {
		printk(KERN_INFO "clock_config_read: END\n");
		done = 'n';
		return 0;
	}
	
	done = '\0';

	if ( copy_to_user(buf, procfs_buf, procfs_buf_size) ) {
		return -EFAULT;
	}

	printk(KERN_INFO "current clock format: %d\n", clock_format);	
	printk(KERN_INFO "clock_module_read: read %lu bytes\n", procfs_buf_size);

	return procfs_buf_size;
}
*/

// this method is executed when writing to the module
static ssize_t clock_config_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "inside clock_config_write() \n");

	if ( count > PROCFS_MAX_LEN )	
	{
		procfs_buf_size = PROCFS_MAX_LEN-1;	// last byte reserved for '\0'
		printk(KERN_INFO "Only %d byte(s) written since that is the max allowed length\n", (int)procfs_buf_size);
	}
	else	
	{
		procfs_buf_size = count;
	}
	
	if ( copy_from_user(procfs_buf, buf, procfs_buf_size) ) 
	{
		return -EFAULT;
	}

	printk(KERN_INFO "clock_config_write: write %lu bytes\n", procfs_buf_size);

	if(procfs_buf[0] == '0')
	{
		clock_format = 0;
	}
	else if(procfs_buf[0] == '1')
	{
		clock_format = 1;
	}
	else	// if user writes anything apart from 0 and 1
	{
		//procfs_buf[0] = '1';	// Default the output time format to 1
		//clock_format = 1;
		//printk(KERN_INFO "clock_config_write: defaulted the output time format to 'yyyy-mm-dd hh:MM:sec'.\n");

		// keeping the clock_format unchanged
		printk(KERN_NOTICE "clock_config_write: invalid input (input must be 0 or 1)");
	}

	procfs_buf[procfs_buf_size] = '\0';

	return procfs_buf_size;
}


// module's file operations, a module may need more of these
static struct file_operations file_ops_main = {
	.owner =	THIS_MODULE,
//	.read =		clock_module_read,
	.read =		seq_read,
//	.write =	clock_module_write,
	.open =		clock_module_open,
	.release =	single_release,
};

static struct file_operations file_ops_config = {
	.owner =	THIS_MODULE,
//	.read =		clock_config_read,
	.read =		seq_read,
	.write =	clock_config_write,
	.open =		clock_config_open,
	.release =	single_release,
};

// initialize module (executed when using insmod)
static int __init clock_module_init(void)
{
	printk(KERN_INFO "MyClockModule is being loaded...\n");

	// create a new ProcFS file
	proc_entry_ptr_main = proc_create(FILE_MAIN, FILE_ACCESS_RONLY, NULL, &file_ops_main);
	printk(KERN_INFO "MyClockModule has created the file object: /proc/deeds_clock.\n");
	
	proc_entry_ptr_config = proc_create(FILE_CONFIG, FILE_ACCESS_RW, NULL, &file_ops_config);
	printk(KERN_INFO "MyClockModule has created the file object: /proc/deeds_clock_config.\n");

	return 0;
}

// cleanup module (executed when using rmmod)
static void __exit clock_module_cleanup(void)
{
	// remove an existing ProcFS entry
	proc_remove(proc_entry_ptr_main);	// not offered by my kernel ver
	proc_remove(proc_entry_ptr_config);
	//remove_proc_entry(FILE_MAIN, proc_entry_ptr_main);
	//remove_proc_entry(FILE_CONFIG, proc_entry_ptr_main);

	printk(KERN_INFO "MyClockModule is being unloaded...\n");
}

module_init(clock_module_init);
module_exit(clock_module_cleanup);