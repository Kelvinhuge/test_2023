#include <linux/module.h>       /* Needed by all modules */
#include <linux/kernel.h>       /* Needed for KERN_INFO */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define FIRST_MINOR 0
#define MINOR_CNT 1


static dev_t dev;
static struct cdev c_dev;
static struct class *cl;
static int status = 1, dignity = 3, ego = 5;

typedef struct
{
    int status, dignity, ego;
} query_arg_t;
 
#define QUERY_GET_VARIABLES _IOR('q', 1, query_arg_t *)
#define QUERY_CLR_VARIABLES _IO('q', 2)
#define QUERY_SET_VARIABLES _IOW('q', 3, query_arg_t *)


static int my_open(struct inode *i, struct file *f)
{
    return 0;
}
static int my_close(struct inode *i, struct file *f)
{
    return 0;
}

static long my_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    query_arg_t q;
 
    switch (cmd)
    {
        case QUERY_GET_VARIABLES:
            q.status = status;
            q.dignity = dignity;
            q.ego = ego;
            if (copy_to_user((query_arg_t *)arg, &q, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            break;
        case QUERY_CLR_VARIABLES:
            status = 0;
            dignity = 0;
            ego = 0;
            break;
        case QUERY_SET_VARIABLES:
            if (copy_from_user(&q, (query_arg_t *)arg, sizeof(query_arg_t)))
            {
                return -EACCES;
            }
            status = q.status;
            dignity = q.dignity;
            ego = q.ego;
            break;
        default:
            return -EINVAL;
    }
 
    return 0;
}


static struct file_operations query_fops =
{
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    .ioctl = my_ioctl
#else
    .unlocked_ioctl = my_ioctl
#endif
};

static int terminate = 0;
#define CLK_NUMERATOR (0xFFFF + 1)
int kthreadfunc_micmute(void *data)
{
	int mute_state=0,state_prev,state_toggle=0;
	unsigned int denominator;
	unsigned int i=0,j=0,k=0;
	unsigned int parent_rate = 2000000;
	struct timespec ts1,ts2;

	while (!kthread_should_stop()) {
		getnstimeofday(&ts1);
			mdelay(100);
/*		for(i=0;i<0xffffffff;i++){
			for(j=0;j<0xffffffff;j++){
				for(k=0;k<0xffffffff;k++)
					denominator = div_u64((uint64_t)parent_rate * 123,64000);
					denominator = div_u64((uint64_t)parent_rate * denominator,64000);
					denominator = div_u64((uint64_t)parent_rate * denominator,64000);

			}
		}*/

		getnstimeofday(&ts2);
		printk("start %ld seconds %ld nanoseconds\n",ts1.tv_sec, ts1.tv_nsec);
		printk("enddd %ld seconds %ld nanoseconds\n",ts2.tv_sec, ts2.tv_nsec);	
		msleep(1000);
		getnstimeofday(&ts2);
		printk("resume at: %ld seconds %ld nanoseconds\n\n",ts2.tv_sec, ts2.tv_nsec);	
	}
	printk("Kevn ====>>> the thread function quit\n");
	do_exit(0);
}

static struct task_struct *micMuteCtlTask;

static int __init hello_start(void)
{
printk(KERN_INFO "Loading hello module...\n");
printk(KERN_INFO "Hello world\n");

    int ret;
    struct device *dev_ret;
 
  
    if ((ret = alloc_chrdev_region(&dev, FIRST_MINOR, MINOR_CNT, "query_ioctl")) < 0)
    {
        return ret;
    }
 
    cdev_init(&c_dev, &query_fops);
 
    if ((ret = cdev_add(&c_dev, dev, MINOR_CNT)) < 0)
    {
        return ret;
    }
     
    if (IS_ERR(cl = class_create(THIS_MODULE, "char")))
    {
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(cl);
    }
    if (IS_ERR(dev_ret = device_create(cl, NULL, dev, NULL, "query")))
    {
        class_destroy(cl);
        cdev_del(&c_dev);
        unregister_chrdev_region(dev, MINOR_CNT);
        return PTR_ERR(dev_ret);
    }


	micMuteCtlTask = kthread_run(kthreadfunc_micmute, (void *)NULL, "MIC1");
	if (!micMuteCtlTask)
		dev_err(dev_ret, "Create Mic mute control thread failed.\n");



 printk(KERN_INFO "Hello world init success!!\n");
    return 0;


}

static void __exit hello_end(void)
{
	if(micMuteCtlTask){
		printk("entering %s\n",__func__);
		kthread_stop(micMuteCtlTask);
		micMuteCtlTask = NULL;
	}
    device_destroy(cl, dev);
    class_destroy(cl);
    cdev_del(&c_dev);
    unregister_chrdev_region(dev, MINOR_CNT);
printk(KERN_INFO "Goodbye Mr. Kevin Li\n");
}

MODULE_LICENSE("GPL v2");
module_init(hello_start);
module_exit(hello_end);
