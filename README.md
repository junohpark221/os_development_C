# os_development_C
Year 2 COMS20001-Concurrent Computing TB 2 Coursework


### Overview:
  This project has been completed to the second stage of the marking criteria. So, it covers 1) pre-emptive multi-tasking, 2) priority-based scheduler, 3) system calls such as fork, exec aexit, and 4) Inter-Process Communication (IPC) with spawn and philosopher programs.


### How to use:
The way to start this kernel is:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;1) You need three terminals in the directory.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;2) Build the project by typing "make build" in the first terminal.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;3) Type "make launch-qemu" in the first terminal.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;4) Type "nc 127.0.0.1 1235" in the second terminal. Then, first terminal shows text lines.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;5) Type "make launch-gdb" in the third terminal. Then, it shows few lines end with "(gdb) "<br/>
&nbsp;&nbsp;&nbsp;&nbsp;6) Type "continue" or "c" in the third terminal.<br/>
Then, the kernel will start and the second terminal will be a console pannel.<br/>
   
The way to use console is:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;- Type "execute P2/P3/P4/P5" makes P2/P3/P4/P5 start.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;- Type "execute spawn" makes spawn program for philosopher start.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;- Type "terminate " followed by the pid number makes the program with the pid number terminate.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(New child process from the fork() function gets the lowest available pid between 1 to 32.)<br/>


### Explanation for spawn.c and Philosopher.c:
spawn.c spawns 16 philosophers at first, and it gives them a local_pid that is used in IPC.<br/>
In the mechanism of IPC, it uses the mutex in every variable that are shared between philosopher processes.<br/>

The logic of the Philosopher.c:<br/>
&nbsp;&nbsp;&nbsp;&nbsp;1) Philosopher is in HUNGRY_STATUS.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;2) The Philosopher with even local_pid tries to grab left fork first, and the philosopher with odd local_pid treis to grab right fork first.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;3) When the philsopher holds one fork, the try to grab another fork.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3-1) If no one is holding another fork, the philosopher grabs it and starts eating.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;3-2) If someone is holding another fork, the philosopher also puts down the fork that he/she is holding. Then, starts again from 1).<br/>
&nbsp;&nbsp;&nbsp;&nbsp;4) When the philosopher finishes eating, he/she puts down all forks and starts thinking.<br/>
&nbsp;&nbsp;&nbsp;&nbsp;5) After the philosopher finishes thinking, the philosopher is in HUNGRY_STATUS again.<br/>
