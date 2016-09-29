#include "r2k.h"

static struct cdev *r2_dev;
static dev_t dev;
static char *r2_devname = "r2";

static int io_open(struct inode *inode, struct file *file) {
	return 0;
}

static int io_close(struct inode *inode, struct file *file) {
	return 0;
}

static long io_ioctl (struct file *file, unsigned int cmd, unsigned long data_addr) {
	unsigned long val;
	int ret = 0;

	if (_IOC_TYPE (cmd) != R2_TYPE) {
		return -EINVAL;
	}

	switch (_IOC_NR (cmd)) {
	case R2_READ_REG:
		{
			// TESTED ON: linx kernel 3.16.0-4-686-pae x86 arch
			struct r2k_control_reg __user *data = NULL;
#if (defined(CONFIG_X86_32) || defined(CONFIG_X86_64))
			data = (struct r2k_control_reg __user *)data_addr;

			val = native_read_cr0 ();
			ret = copy_to_user ((unsigned long *)(&(data->cr0)), &val, reg_size);
			if (ret) {
				pr_info ("ERROR: copy_to_user0 failed\n");
				return -EINVAL;
			}
			val = native_read_cr2 ();
			ret = copy_to_user ((unsigned long *)(&(data->cr2)), &val, reg_size);
			if (ret) {
				pr_info ("ERROR: copy_to_user2 failed\n");
				return -EINVAL;
			}
			val = native_read_cr3 ();
			ret = copy_to_user ((unsigned long *)(&(data->cr3)), &val, reg_size);
			if (ret) {
				pr_info ("ERROR: copy_to_user3 failed\n");
				return -EINVAL;
			}
			val = native_read_cr4_safe ();
			ret = copy_to_user ((unsigned long *)(&(data->cr4)), &val, reg_size);
			if (ret) {
				pr_info ("ERROR: copy_to_user4 failed\n");
				return -EINVAL;
			}
#ifdef CONFIG_X86_64
			val = native_read_cr8 ();
			ret = copy_to_user ((unsigned long *)(&(data->cr8)), &val, reg_size);
#endif
#endif
			break;
		}
	case R2_PROC_INFO:
		{
			unsigned long counter = 0;
			struct r2k_proc_info *data = NULL;
			struct task_struct *task = NULL;
			struct mm_struct *mm = NULL;
			struct vm_area_struct *vma = NULL;

			data = kmalloc (sizeof (*data), GFP_KERNEL);
			if (!data) {
				return -ENOMEM;
			}
			memset (data, 0, sizeof (*data));
			ret = copy_from_user (&(data->pid), &(((struct r2k_proc_info __user *)data_addr)->pid), sizeof (pid_t));
			if (ret) {
				kfree (data);
				ret = -EFAULT;
				break;
			}

			task = pid_task (find_vpid (data->pid), PIDTYPE_PID);
			if (!task) {
				pr_info ("Couldn't retrieve task_struct for pid (%d)\n", data->pid);
				kfree (data);
				ret = -ESRCH;
				break;
			}

			mm = task->mm;
			vma = mm ? mm->mmap : NULL;

			get_task_comm (data->comm, task);

		    if (vma) {
				counter = 0;
				for (; vma; vma = vma->vm_next) {
					if (counter + 3 > 1024) {
						kfree (data);
						return -ENOMEM;
					}
					data->vmareastruct[counter] = vma->vm_start;
					if (stack_guard_page_start (vma, vma->vm_start)) {
						data->vmareastruct[counter] += PAGE_SIZE;
					}
					data->vmareastruct[counter+1] = vma->vm_end;
					if (stack_guard_page_end (vma, vma->vm_end)) {
						data->vmareastruct[counter+1] -= PAGE_SIZE;
					}
					data->vmareastruct[counter+2] = vma->vm_flags;
					counter += 3;
				}
			}

			ret = copy_to_user ((void *)data_addr, data, sizeof (*data));
			if (ret) {
				ret = -EFAULT;
			}
			kfree (data);
			break;
		}
	default:
		break;
	}
	return ret;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = io_open,
	.release = io_close,
	.unlocked_ioctl = io_ioctl,
};

static int __init r2k_control_reg_init(void) {
	int ret;
	pr_info ("%s: loading dirver\n", r2_devname);

	ret = alloc_chrdev_region (&dev, 0, 1, r2_devname);
	r2_dev = cdev_alloc ();
	cdev_init (r2_dev, &fops);
	cdev_add (r2_dev, dev, 1);

	pr_info ("%s: please create the proper device with - mknod /dev/%s c %d %d\n", r2_devname, r2_devname, MAJOR (dev), MINOR (dev));

    return 0;
}

static void __exit r2k_control_reg_exit(void) {
	unregister_chrdev_region (dev, 1);
	cdev_del (r2_dev);
	pr_info ("%s: unloading driver\n", r2_devname);
}

module_init (r2k_control_reg_init);
module_exit (r2k_control_reg_exit);

MODULE_LICENSE ("GPL v2");
MODULE_AUTHOR ("radare");
MODULE_DESCRIPTION ("r2 driver to read control registers");
