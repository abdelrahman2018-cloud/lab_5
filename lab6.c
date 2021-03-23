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

MODULE_LICENSE("GPL");

unsigned long *FORK_ADDRS;	//to hold function address
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
	fork();
}

//--------------------------------------------------------------------------------------

struct myfile *open_file_for_read(char *filename){
        
	struct myfile *fi;
	fi = kmalloc(sizeof(struct myfile) , GFP_KERNEL);
	fi->fs = get_fs();
	set_fs(get_ds());
	fi->f = filp_open(filename , O_RDWR , 0);
 	set_fs(fi->fs);

	if (IS_ERR(fi->f)){
		printk(KERN_INFO "File failed to open\n");
		kfree(fi);
		return NULL;
	}else printk(KERN_INFO "File opened\n");
	return fi;

}

//--------------------------------------------------------------------------------------

volatile int read_from_file_until(struct myfile *mf,char *buf, unsigned long vlen, char c){

	int sss;
	int i = 0;

	mf->fs = get_fs();
	set_fs(get_ds());
	//mf->pos = 0;

while(i <vlen){
        sss = vfs_read(mf->f , buf+i , 1 , &(mf->pos));	
	if(buf[i] == c){
		buf[i] = '\0';	
	 	break;
	}
	i++;	
}
	set_fs(mf->fs);	
	return sss;
		
}

//--------------------------------------------------------------------------------------

void close_file(struct myfile * mf){
	if(mf->f){
		filp_close(mf->f , NULL);	//to clode the file
		printk(KERN_INFO "File closed");	
	}
}


//-------------------------------------------------------------------------------------

int init(void){

int i = 0;		//for while loops
char *testt;		//temp string
char *neww;		//temp string
unsigned long *add;	//pointer to string
sys_call_ptr_t *call;		//for array

	struct myfile *ff;	//struct to hold the opened file
	char *buf;		//buffer for read function
	int ss;			//returned back from read function
	printk(KERN_INFO "init function\n");

	ff = kmalloc(sizeof(struct myfile) , GFP_KERNEL);	//allocating memory space for struct
	ff = open_file_for_read("../../boot/System.map-4.19.0-13-amd64");	//call open function and savein ff	

	buf = kmalloc(sizeof(char) , GFP_KERNEL);	//allocate memory space
	ff -> pos = 0;

	testt = "sys_call_table";

	while(i != 1){
		ss = read_from_file_until(ff, buf, 1024 , '\n');		//read and print buf
		neww = strstr(buf , testt);
		if(neww){
			break;
		}
	}

//	printk(KERN_INFO "%s \n" , buf);		//print the whole line
	
	i = 0;
	while (i != 50){		//extract the address
		if(buf[i] == ' ') break;
		neww[i] = buf[i];
		i++;
	}

	neww[i] = NULL;				// the address is in char string neww
	
	add = (unsigned long *)neww;		//pointer unsigned
	
	//printk(KERN_ALERT "%s \n" , add);	//FOR TESTING
	
	sscanf(neww, "%lx", &add);		//for creating pointer to array
	call = (sys_call_ptr_t *)add;
//	printk(KERN_INFO "%px \n", call[__NR_clone]);	//third entry	call[2] = __NR_fork

	FORK_ADDRS = call[__NR_clone];		//store the original fork address(__NR_clone)
	call[__NR_clone] = (sys_call_ptr_t)count_fork;	//store the address of the function instead of clone 
	close_file(ff);				//close the file
	
	kfree(ff);		//free memory
	kfree(buf);
	return 0;
}

//-------------------------------------------------------------------------------------------


void cleanup(void){
	
int i = 0;		//for while loops
char *testt;		//temp string
char *neww;		//temp string
unsigned long *add;	//pointer to string
sys_call_ptr_t *call;		//for array

	struct myfile *ff;	//struct to hold the opened file
	char *buf;		//buffer for read function
	int ss;			//returned back from read function
	printk(KERN_INFO "init function\n");

	ff = kmalloc(sizeof(struct myfile) , GFP_KERNEL);	//allocating memory space for struct
	ff = open_file_for_read("../../boot/System.map-4.19.0-13-amd64");	//call open function and savein ff	

	buf = kmalloc(sizeof(char) , GFP_KERNEL);	//allocate memory space
	ff -> pos = 0;

	testt = "sys_call_table";

	while(i != 1){
		ss = read_from_file_until(ff, buf, 1024 , '\n');		//read and print buf
		neww = strstr(buf , testt);
		if(neww){
			break;
		}
	}

//	printk(KERN_INFO "%s \n" , buf);		//print the whole line
	
	i = 0;
	while (i != 50){		//extract the address
		if(buf[i] == ' ') break;
		neww[i] = buf[i];
		i++;
	}

	neww[i] = NULL;				// the address is in char string neww
	
	add = (unsigned long *)neww;		//pointer unsigned
	
	//printk(KERN_ALERT "%s \n" , add);	//FOR TESTING
	
	sscanf(neww, "%lx", &add);		//for creating pointer to array
	call = (sys_call_ptr_t *)add;
//	printk(KERN_INFO "%px \n", call[__NR_clone]);	//third entry	call[2] = __NR_fork

//	FORK_ADDRS = call[__NR_clone];		//store the original fork address(__NR_clone)
	call[__NR_clone] = (sys_call_ptr_t)FORK_ADDRS;	//store the original address back
	close_file(ff);				//close the file
	
	kfree(ff);		//free memory
	kfree(buf);
	return 0;

	printk(KERN_INFO "cleanup function\n");
}

module_init(init);
module_exit(cleanup);
