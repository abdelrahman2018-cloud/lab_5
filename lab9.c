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
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
MODULE_LICENSE("GPL");

unsigned char key [128] = {0};	//for key
unsigned char massage [4096] = {0};	//for massage
unsigned char ciphered [4096] = {0};
unsigned char key_proc[128] = {0};
int count = 0;
//-------------------------------------------------------------------------------------
//rc4 stream cipher code
void rc4(unsigned char * p, unsigned char * k, unsigned char * c, int l)
{
	unsigned char s[256];
	unsigned char t [256];
	unsigned char temp;
	unsigned char kk;
	int i , j , x;

	for( i = 0 ; i < 256 ; i++)
	{
		s[i] = i;
		t[i] = k[i & 4];
	}
	j = 0;
	for (i = 0 ; i < 256 ;  i++)
	{
		j = (j+s[i]+t[i])%256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}

	i = j = -1;
	for( x = 0 ; x < 1 ; x++ )
	{
		i = (i+1) % 256;
		j = (j+s[i]) % 256;
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
		kk = (s[i]+s[j]) % 256;
		c[x] = p[x] ^ s[kk];
	}
}

//*****************************FILE OPERATIONS STRUCTS*********************************
//-------------------------------------------------------------------------------------
//cipher driver
static int myopen(struct inode * inode , struct file* file)
{
	printk(KERN_ALERT "Cipher Driver opened\n");
	return 0;
};
static ssize_t myread(struct file *filp ,char __user * buf , size_t len , loff_t *off ){
	printk(KERN_ALERT "Reading cipher driver\n");
	printk(KERN_ALERT "%s\n", ciphered);	//print the massage
	return 0;
};
static ssize_t mywrite(struct file * filp, const char __user *buf , size_t len , loff_t *off){
	printk(KERN_ALERT "Writing the massage in cipher driver\n");
	
	if (copy_from_user(massage, buf, len) != 0){
		printk(KERN_ALERT "error happened\n");
		return 0;
	}
	//take the massage from the user and save it on the driver
	return 1;
};
static int myrelease(struct inode * inode , struct file * file){
	printk(KERN_ALERT "Closing cipher driver\n");
	return 0;
};

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = myopen,
	.read = myread,
	.write = mywrite,
	.release = myrelease,
};
//-------------------------------------------------------------------------------------
//cipher_key driver
static int myopenk(struct inode * inode , struct file* file)
{
	printk(KERN_ALERT "cipher key driver opened.\n");
	return 0;
};
static ssize_t myreadk(struct file *filp ,char __user * buf , size_t len , loff_t *off ){
	printk(KERN_ALERT "Go away silly one, you cannot see my key >-:\n");
	return 0;
};
static ssize_t mywritek(struct file * filp, const char __user *buf , size_t len , loff_t *off){
	printk(KERN_ALERT "Writing on cipher key\n");

	if(copy_from_user(key, buf, len) != 0){
		printk(KERN_ALERT "eroor! \n" );
	}

	rc4(massage , key , ciphered , 4096);	//encrypting

//take the cipher key from the user and store it on the driver
//then encrypt the massage stored in the driver	
	
	return 1;
};
static int myreleasek(struct inode * inode , struct file * file){
	printk(KERN_ALERT "Closing cipher key driver\n");
	return 0;
};

static struct file_operations fopsk = {
	.owner = THIS_MODULE,
	.open = myopenk,
	.read = myreadk,
	.write = mywritek,
	.release = myreleasek,
};
//-------------------------------------------------------------------------------------
//cipher proc
static int myopenp(struct inode * inode , struct file* file)
{
	printk(KERN_ALERT "cipher proc got opened\n");
	return 0;
};
static ssize_t myreadp(struct file *filp ,char __user * buf , size_t len , loff_t *off ){
	printk(KERN_ALERT "Reading the massage stored in cipher\n");
	printk(KERN_ALERT "%s\n" , massage);	
	return 0;
};
static ssize_t mywritep(struct file * filp, const char __user *buf , size_t len , loff_t *off){
	printk(KERN_ALERT "cannot write on the cipher proc!\n");
	return 1;
};
static int myreleasep(struct inode * inode , struct file * file){
	printk(KERN_ALERT "Cipher proc release\n");
	return 0;
};

static struct file_operations fopsp = {
	.owner = THIS_MODULE,
	.open = myopenp,
	.read = myreadp,
	.write = mywritep,
	.release = myreleasep,
};
//-------------------------------------------------------------------------------------
//cipher_key proc
static int myopenpk(struct inode * inode , struct file* file)
{
	printk(KERN_ALERT "Opening cipher_key proc\n");
	return 0;
};
static ssize_t myreadpk(struct file *filp ,char __user * buf , size_t len , loff_t *off ){
	printk(KERN_ALERT "Cannot read the cipher_key!\n YOU CANNPT READ THE CIPHER_KEY!!!\n");
	return 0;
};
static ssize_t mywritepk(struct file * filp, const char __user *buf , size_t len , loff_t *off){
	printk(KERN_ALERT "writing the cipher key\n");

	if(copy_from_user(key_proc , buf , len) != 0){
		printk(KERN_ALERT "error!\n");
	}

	if(key_proc == key){
		rc4(ciphered , key , massage , 4096);	//decrypting
	}else{
	       	printk(KERN_ALERT "Wrong Key!");
	}

	//read the cipher key then check and if right then decrypt the massage and save it on cipher proc
	return 1;
};
static int myreleasepk(struct inode * inode , struct file * file){
	printk(KERN_ALERT "cipher_key proc release\n");
	return 0;
};

static struct file_operations fopspk = {
	.owner = THIS_MODULE,
	.open = myopenpk,
	.read = myreadpk,
	.write = mywritepk,
	.release = myreleasepk,
};

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

//declarations
	static struct cdev cipher;
	static struct cdev cipher_key;
	static dev_t c , ck;

int init(void){
	printk(KERN_INFO "Init function!\n");
		
//register
	register_chrdev(111, "cipher", &fops);
	register_chrdev(111, "cipher_key", &fopsk);

//allocate the major and minor number in dev_t	
	c = MKDEV(111,0);
	ck = MKDEV(111,1);

//init
	cdev_init(&cipher, &fops);
	cdev_init(&cipher_key, &fopsk);
//add
	cdev_add(&cipher , c , 1);
	cdev_add(&cipher_key , ck , 1);
	
//proc file create
	proc_create("cipher" , 0 , NULL , &fopsp);
	proc_create("cipher_key" , 0 , NULL , &fopspk);

	return 0;
}

void cleanup(void){
//unregister
	unregister_chrdev(111, "cipher");	
	unregister_chrdev(111, "cipher_key");
//delete
	cdev_del(&cipher);
	cdev_del(&cipher_key);
//remove proc
	remove_proc_entry("cipher", NULL);
	remove_proc_entry("cipher_key", NULL);

	printk(KERN_INFO "Cleanup function\n");
}

module_init(init);
module_exit(cleanup);
