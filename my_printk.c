#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage int sys_my_printk(int pid, long t_start_sec, long t_start_nsec, long t_end_sec, long t_end_nsec)
{
	printk("[Project1] %d %ld.%ld %ld.%ld\n", pid, t_start_sec, t_start_nsec, t_end_sec, t_end_nsec);
	return 0;
}
