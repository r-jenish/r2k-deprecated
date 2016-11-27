#ifndef __R2K__H__
#define __R2K__H__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/ioctl.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/mm_types.h>

#if (defined(CONFIG_X86_32) || defined(CONFIG_X86_64))
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,0))
#include <asm/special_insns.h>
#elif (KERNEL_VERSION(3,4,0) > LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28))
#include <asm/system.h>
#elif (KERNEL_VERSION(2,6,28) > LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
#include <asm-x86/system.h>
#elif (KERNEL_VERSION(2,6,24) > LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
#include <asm-i386/system.h>
#elif (KERNEL_VERSION(2,6,22) > LINUX_VERSION_CODE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))//KERNEL_VERSION(2,6,14)
#include <asm-i386/system.h>
#define native_read_cr0 read_cr0

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,16)) && defined (CONFIG_X86_32)
#define native_read_cr4_safe() ({		\
	unsigned int __dummy;			\
	__asm__("1:movl %%cr4, %0           \n"	\
		"2:                         \n"	\
		".section __ex_table,\"a\"  \n"	\
		".long 1b, 2b               \n"	\
		".previous                  \n"	\
		:"=r" (__dummy): "0" (0));	\
	__dummy;				\
})
#else
#define native_read_cr4_safe read_cr4
#endif	/* defined as it is in 2.6.16 */

#ifdef CONFIG_X86_64
#define native_read_cr8 read_cr8
#endif

#ifdef read_cr2
#define native_read_cr2 read_cr2
#else
#define native_read_cr2() ({				\
	unsigned int __dummy;				\
	__asm__ __volatile__(				\
			     "movl %%cr2, %0\n\t"	\
			     :"=r" (__dummy));		\
	__dummy;					\
})
#endif	/* defined as it is in 2.6.16 */

#ifdef read_cr3
#define native_read_cr3 read_cr3
#else
#define native_read_cr3() ({		\
	unsigned int __dummy;		\
	__asm__ (			\
		 "movl %%cr3, %0\n\t"	\
		 :"=r" (__dummy));	\
	__dummy;			\
})
#endif	/* defined as it is in 2.6.16 */
#endif

/*
#elif KERNEL_VERSION(2,6,14) > LINUX_VERSION_CODE && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#include <asm-i386/system.h>
#define native_read_cr0 read_cr0
#define native_read_cr4 read_cr4
*/
#endif

static char R2_TYPE = 'k';


#if defined(CONFIG_X86_32) || defined(CONFIG_X86_64)
struct r2k_control_reg {
	unsigned long cr0;
	unsigned long cr1; //Register Not used. What to do?
	unsigned long cr2;
	unsigned long cr3;
	unsigned long cr4;
#ifdef CONFIG_X86_64
	unsigned long cr8;
#endif
};
#endif


//fails for kernel 3.15 x86
struct r2k_proc_info {
	pid_t pid;
	char comm[16]; //TASK_COMM_LEN = 16 include/linux/sched.h
	unsigned long vmareastruct[4096];
	unsigned long stack;
};

#define R2_READ_REG 0x4
#define R2_PROC_INFO 0x8

#if defined(CONFIG_X86_32) || defined(CONFIG_ARM)
#define reg_size 4
#elif defined(CONFIG_X86_64) || defined(CONFIG_ARM64)
#define reg_size 8
#endif

#if defined (CONFIG_ARM)
struct r2k_control_reg {
	unsigned long ttbr0;
	unsigned long ttbr1;
	unsigned long ttbcr;
	unsigned long c1;
	unsigned long c3;
};

#define read_ttbr0() ({				\
	unsigned long __dummy;			\
	__asm__ ("mrc p15, 0, %0, c2, c0, 0":	\
		 "=r" (__dummy));		\
	__dummy;				\
})

#define read_ttbr1() ({ 			\
	unsigned long __dummy;			\
	__asm__ ("mrc p15, 0, %0, c2, c0, 1":	\
		 "=r" (__dummy));		\
	__dummy;				\
})

#define read_ttbcr() ({				\
	unsigned long __dummy;			\
	__asm__ ("mrc p15, 0, %0, c2, c0, 2":	\
		 "=r" (__dummy));		\
	__dummy;				\
})

#define read_c1() ({				\
	unsigned long __dummy;			\
	__asm__ ("mrc p15, 0, %0, c1, c0, 0":	\
		 "=r" (__dummy));		\
	__dummy;				\
})

#define read_c3() ({				\
	unsigned long __dummy;			\
	__asm__ ("mrc p15, 0, %0, c3, c0, 0":	\
		 "=r" (__dummy));		\
	__dummy;				\
})

#elif defined (CONFIG_ARM64)
struct r2k_control_reg {
	unsigned long sctlr_el1;
	unsigned long ttbr0_el1;
	unsigned long ttbr1_el1;
	unsigned long tcr_el1;
};
#define read_sctlr_EL1() ({		\
	unsigned long __dummy;		\
	__asm__ ("MRS %0, SCTLR_EL1":	\
		 "=r" (__dummy));	\
	__dummy;			\
})

#define read_ttbr0_EL1() ({		\
	unsigned long __dummy;		\
	__asm__ ("MRS %0, TTBR0_EL1":	\
		 "=r" (__dummy));	\
	__dummy;			\
})

#define read_ttbr1_EL1() ({		\
	unsigned long __dummy;		\
	__asm__ ("MRS %0, TTBR1_EL1":	\
		 "=r" (__dummy));	\
	__dummy;			\
})

#define read_tcr_EL1() ({		\
	unsigned long __dummy;		\
	__asm__ ("MRS %0, TCR_EL1": 	\
		 "=r" (__dummy));	\
	__dummy;			\
})

#if 0
#define read_sctlr_EL2() ({    \
			unsigned long __dummy;    \
			__asm__ ("MRS %0, SCTLR_EL2":    \
					 "=r" (__dummy));    \
			__dummy;    \
})

#define read_sctlr_EL3() ({    \
			unsigned long __dummy;    \
			__asm__ ("MRS %0, SCTLR_EL3":    \
					 "=r" (__dummy));    \
			__dummy;    \
})

#define read_ttbr0_EL3() ({    \
			unsigned long __dummy;    \
			__asm__ ("MRS %0, TTBR0_EL3":    \
					 "=r" (__dummy));    \
			__dummy;    \
})

#define read_tcr_EL2() ({    \
			unsigned long __dummy;    \
			__asm__ ("MRS %0, TCR_EL2":    \
					 "=r" (__dummy));    \
			__dummy;    \
})

#define read_tcr_EL3() ({    \
			unsigned long __dummy;    \
			__asm__ ("MRS %0, TCR_EL3":    \
					 "=r" (__dummy));    \
			__dummy;    \
})
#endif //end if 0

#endif

#endif
