Concurrent Computing TB2 Coursework


Overview:
  This project has been completed to the second stage of the marking criteria. So, it covers 1) pre-emptive multi-tasking, 2) priority-based scheduler, 3) system calls such as fork, exec aexit, and 4) Inter-Process Communication (IPC) with spawn and philosopher programs.


How to use:
  The way to start this kernel is:
     1) You need three terminals in the directory.
     2) Build the project by typing "make build" in the first terminal.
     3) Type "make launch-qemu" in the first terminal.
     4) Type "nc 127.0.0.1 1235" in the second terminal. Then, first terminal shows text lines.
     5) Type "make launch-gdb" in the third terminal. Then, it shows few lines end with "(gdb) "
     6) Type "continue" or "c" in the third terminal.
     Then, the kernel will start and the second terminal will be a console pannel.
   
  The way to use console is:
     - Type "execute P2/P3/P4/P5" makes P2/P3/P4/P5 start.
     - Type "execute spawn" makes spawn program for philosopher start.
     - Type "terminate " followed by the pid number makes the program with the pid number terminate.
       (New child process from the fork() function gets the lowest available pid between 1 to 32.)


Explanation for spawn.c and Philosopher.c:
  - spawn.c spawns 16 philosophers at first, and it gives them a local_pid that is used in IPC.
  - In the mechanism of IPC, it uses the mutex in every variable that are shared between philosopher processes.
  - The logic of the Philosopher.c:
     1) Philosopher is in HUNGRY_STATUS.
     2) The Philosopher with even local_pid tries to grab left fork first, and the philosopher with odd local_pid treis to grab right fork first.
     3) When the philsopher holds one fork, the try to grab another fork.
        3-1) If no one is holding another fork, the philosopher grabs it and starts eating.
        3-2) If someone is holding another fork, the philosopher also puts down the fork that he/she is holding. And then, starts again from 1).
     4) When the philosopher finishes eating, he/she puts down all forks and starts thinking.
     5) After the philosopher finishes thinking, the philosopher is in HUNGRY_STATUS again.



