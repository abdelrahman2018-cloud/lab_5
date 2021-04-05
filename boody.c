#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");

unsigned long *FORK_ADDRS;	//to hold function address
long fork_counter = 200;		//fork counter

//----------------------------------------------------------------------------------
//LAB_8 code

static int fork_count_show(struct seq_file *m , void *v){	//function to print the fork count
	seq_printf(m , "Fork count is: %d\n" , fork_counter);
}

static int fork_count_open(struct inode *inode, struct file *file ){	//for .open call
	return single_open(file, fork_count_show ,NULL);
}
/*	//ignore this code
static int fork_count_write(struct inode *inode, struct file *file ){	//for .write call
	return single_open(file, fork_count_show ,NULL);
}

static int fork_count_read(struct inode *inode, struct file *file ){	//for .read call
	return single_open(file, fork_count_show ,NULL);
}
*/

static const struct file_operations fork_count_fops = {
.open = fork_count_open, 
.read = seq_read,
//.write = fork_count_write,	//bonus but not working
.llseek = seq_lseek,
};

//-----------------------------------------------------------------------------------

void count_fork(int i){

	fork_counter++;			//increament the counter
	if (fork_counter%10 == 0){	//print every 10 times
		printk(KERN_INFO "Number of forks: %d \n" , fork_counter);
	}
	//call fork system call
//	fork();
}



//-------------------------------------------------------------------------------------

int init(void){
unsigned long add;	//pointer to string
//sys_call_ptr_t *call;		//for array

printk(KERN_INFO "Init function\n");


add = kallsyms_lookup_name("sys_call_table");

//printk(KERN_INFO "%d\n" , add);
proc_create("fork_count" , 0 , NULL , &fork_count_fops);	//create the proc file

	return 0;
}

//-------------------------------------------------------------------------------------------


void cleanup(void){

//unsigned long *add;	//pointer to string
//sys_call_ptr_t *call;		//for array

	remove_proc_entry("fork_count" , NULL);		//remove the proc file

	printk(KERN_INFO "cleanup function\n");
}

module_init(init);
module_exit(cleanup);
