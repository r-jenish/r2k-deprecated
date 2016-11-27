#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define VM_READ 0x1
#define VM_WRITE 0x2
#define VM_EXEC 0x4
#define VM_MAYSHARE 0x80

struct r2k_control_reg {
#if defined(__amd64__) || defined(__i386__)
	unsigned long cr0;
	unsigned long cr1;
	unsigned long cr2;
	unsigned long cr3;
	unsigned long cr4;
#ifdef __amd64__
	unsigned long cr8;
#endif
#elif defined(__arm__)
	unsigned long ttbr0;
	unsigned long ttbr1;
	unsigned long ttbcr;
	unsigned long c1;
	unsigned long c3;
#elif defined(__aarch64__)
	unsigned long sctlr_el1;
	unsigned long ttbr0_el1;
	unsigned long ttbr1_el1;
	unsigned long tcr_el1;
#endif
};

struct r2k_proc_info {
	int pid;
	char comm[16];
	unsigned long vmareastruct[4096];
	unsigned long stack;
};

#define R2_READ_REG 0x4
#define R2_PROC_INFO 0x8

static char R2_TYPE = 'k';

#define READ_CONTROL_REG _IOR (R2_TYPE, R2_READ_REG, sizeof (struct r2k_control_reg));
#define READ_PROCESS_INFO _IOR (R2_TYPE, R2_PROC_INFO, sizeof (struct r2k_proc_info));

int main (int argc, char **argv) {
	int fd, ret;
	unsigned int ioctl_n;
	char *devicename = "/dev/r2";
	struct r2k_control_reg data;
	//struct r2k_proc_info data;
	int i;

	fd = open (devicename, O_RDONLY);
	if (fd == -1) {
		perror ("error\n");
		return -1;
	}

	ioctl_n = READ_CONTROL_REG;
	ret = ioctl (fd, ioctl_n, &data);

#ifdef __i386__
	data.cr1 = 0;
	printf ("cr0 = %p\ncr1 = %p\ncr2 = %p\ncr3 = %p\ncr4 = %p\n", (void *)data.cr0, (void *)data.cr1, (void *)data.cr2, (void *)data.cr3, (void *)data.cr4);
#elif defined(__amd64__)
	data.cr1 = 0;
	printf ("cr0 = %p\ncr1 = %p\ncr2 = %p\ncr3 = %p\ncr4 = %p\ncr8 = %p\n", (void *)data.cr0, (void *)data.cr1, (void *)data.cr2, (void *)data.cr3, (void *)data.cr4, (void *)data.cr8);
#elif defined(__arm__)
	printf ("ttbr0 = %p\nttbr1 = %p\nttbcr = %p\nc1 = %p\nc3 = %p\n", (void *)data.ttbr0, (void *)data.ttbr1, (void *)data.ttbcr, (void *)data.c1, (void *)data.c3);
#elif defined(__aarch64__)
	printf ("sctlr_el1 = %p\nttbr0_el1 = %p\nttbr1_el1 = %p\ntcr_el1 = %p\n", (void *)data.sctlr_el1, (void *)data.ttbr0_el1, (void *)data.ttbr1_el1, (void *)data.tcr_el1);
#endif

#if 0
	ioctl_n = READ_PROCESS_INFO;
	printf ("pid: ");
	scanf ("%d", &(data.pid));
	ret = ioctl (fd, ioctl_n, &data);
	if (ret == 0) {
		printf ("pid = %d\nname = %s\n", data.pid, data.comm);
		for (i = 0; i < 4096;) {
			if (data.vmareastruct[i] == 0 && data.vmareastruct[i+1] == 0) {
				break;;
			}
			printf ("%08lx-%08lx %c%c%c%c %08lx %02x:%02x %lu",
					data.vmareastruct[i], data.vmareastruct[i+1],
					data.vmareastruct[i+2] & VM_READ ? 'r' : '-',
					data.vmareastruct[i+2] & VM_WRITE ? 'w' : '-',
					data.vmareastruct[i+2] & VM_EXEC ? 'x' : '-',
					data.vmareastruct[i+2] & VM_MAYSHARE ? 's' : 'p',
					data.vmareastruct[i+3], data.vmareastruct[i+4],
					data.vmareastruct[i+5], data.vmareastruct[i+6]);
			i += 7;
			printf ("\t\t%s\n", &(data.vmareastruct[i]));
			i += (strlen(&(data.vmareastruct[i])) - 1 + sizeof (unsigned long)) / sizeof (unsigned long);
		}
	    printf ("STACK = %p\n", (void *)data.stack);
	} else {
		printf ("OUT OF MEMORY\n");
	}
#endif
	close (fd);

	return 0;
}
