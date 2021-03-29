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

MODULE_LICENSE("GPL");

unsigned long FORK_ADDRS;	//to hold function address
long fork_counter;		//fork counter

struct myfile{
	struct file *f;
	mm_segment_t fs;
	loff_t pos;
};
//-----------------------------------------------------------------------------------

void count_fork(){

	fork_counter++;			//increament the counter
	if (fork_counter%10 == 0){	//print every 10 times
		printk(KERN_INFO "Number of forks: %d \n" , fork_counter);
	}
	//call fork system call
//	fork();
}



//-------------------------------------------------------------------------------------

int init(void){

//int i = 0;		//for while loops
//char *testt;		//temp string
//char *neww;		//temp string
unsigned long add;	//pointer to string
//sys_call_ptr_t *call;		//for array

add = kallsyms_lookup_name("sys_call_table");

printk(KERN_ALERT "%d\n" , add);

printk(KERN_INFO "init function\n");

FORK_ADDRS = add;	//store the original address at FORK_ADDRS
kallsyms_lookup_name("sys_call_table") = (sys_call_ptr_t)count_fork; //replace the address of the clone function with the function count_fork


	return 0;
}

//-------------------------------------------------------------------------------------------


void cleanup(void){
	
//int i = 0;		//for while loops
//char *testt;		//temp string
//char *neww;		//temp string
unsigned long add;	//pointer to string
//sys_call_ptr_t *call;		//for array

kallsyms_lookup_name("sys_call_table") = (sys_call_ptr_t)FORK_ADDRS;	//return the original address back


	printk(KERN_INFO "cleanup function\n");
}

module_init(init);
module_exit(cleanup);
