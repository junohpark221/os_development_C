/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */



#include "Philosopher.h"

// This function gives the local_pid to the philosopher. local_pid is in between 1 to 16.
int give_local_pid() {
  int stop = 0;
  int local_pid = 0;
  while(1){
    if(getpidm() == 0) {
      setpidm(2);
      local_pid = getpid();
      if(getpidm() == 2) {    // Check the process locks mutex appropriately or not.
        stop = 1;
      }
    }
    if(stop != 0) {
      break;
    }
  }
  setpidm(0);

  return local_pid;
}

// This functions shows the result of eating. (i.e 3 EAT, 4 EAT, ...)
void result(int local_pid){
  char new_pid = '0' + local_pid;
  PL011_putc( UART0, new_pid, true );
  PL011_putc( UART0, ' ', true );
  PL011_putc( UART0, 'E', true );
  PL011_putc( UART0, 'A', true );
  PL011_putc( UART0, 'T', true );
  PL011_putc( UART0, '\n', true );
}

// This function locks the mutex.
void lock_mutex(int local_pid) {
  int stop = 0;
  while(1){
    if(getmut() == 0) {
      setmut(local_pid);
      if(getmut() == local_pid){    // Check the process locks mutex appropriately or not.
        stop = 1;
      }
    }
    if(stop != 0){
      break;
    }
  }
}

// This function unlocks the mutex.
void unlock_mutex() {
  setmut(0);
}

// This function downloads the status of the philosopher which is saved in phil_status1 or phil_status2 in hilevel.c.
unsigned int decode_status(int local_pid, unsigned int currstatus) {
  unsigned int decoding = currstatus;
  decoding = decoding >> (((local_pid - 1) % 8) * 3);
  decoding = decoding & 0b0111;
  if(!(decoding >= 0 && decoding <= 5)) {
    decoding = 0;
  }

  return decoding;
}

// This function uploads the current status of the philosopher to the phil_status1 or phil_status2 in hilevel.c.
unsigned int code_status(int local_pid, unsigned int currp_state, unsigned int currstatus, status_p p_state) {
  unsigned int coding = currstatus;
  int stop = 0;
  while(1){
    if(getstatem() == 0){
      setstatem(local_pid);
      if(getstatem() == local_pid){   // Check the process locks mutex appropriately or not.
        stop = 1;
      }
    }
    if(stop != 0){
      break;
    }
  }
  coding = coding - (currp_state * 2^(((local_pid - 1) % 8) * 3));
  coding = coding + ((int)(p_state) * 2^(((local_pid - 1) % 8) * 3));
  setstatem(0);

  return coding;
}

// This function updates the philosopher's status.
void update_status(int local_pid, status_p p_state){
  unsigned int currstatus; unsigned int newstatus;
  unsigned int currp_state;
  currstatus = getstat(local_pid);
  currp_state = decode_status(local_pid, currstatus);
  newstatus = code_status(local_pid, currp_state, currstatus, p_state);
  setstat(newstatus, local_pid);
}

// This fucntion checks the left or right forks are available or not.
int check(int local_pid, int forks, int side) {
  unsigned int currstatus = getstat(local_pid);
  int avail = 0;
  if(side == 0){
    if(!(decode_status(forks, currstatus) == (unsigned int)4 || decode_status(forks, currstatus) == (unsigned int)5)){
      avail = 1;
    }
  }
  else if (side == 1) {
    if(!(decode_status(forks, currstatus) == (unsigned int)3 || decode_status(forks, currstatus) == (unsigned int)5)){
      avail = 1;
    }
  }

  return avail;
}

// The philosopher eats for a while.
void eating() {
  for(int i = 0; i < 10; i++) {
    yield();
  }
}

// after the philosopher eats, the philosopher thinks for a while with THINKING_STATUS.
void thinking(int local_pid) {
  update_status(local_pid, THINKING_STATUS);
  for(int i = 0; i < 10; i++) {
    yield();
  }
}

// the main function of Philosopher.c
void main_Philosopher() {
  int local_pid; int left_fork; int right_fork; int fork; int side;

  local_pid = give_local_pid();
  left_fork = (local_pid % 16) + 1;
  right_fork = ((local_pid - 2) % 16) + 1;

  update_status(local_pid, HUNGRY_STATUS);

  while(1) {
    if(local_pid % 2 == 0) {
      fork = left_fork;
      side = 0;
    }
    else {
      fork = right_fork;
      side = 1;
    }

    while(1){
      lock_mutex(local_pid);
      if(check(local_pid, fork, side) == 1) {
        if(side = 0){
          update_status(local_pid, HOLDING_LEFT_FORK_STATUS);
        }
        else{
          update_status(local_pid, HOLDING_RIGHT_FORK_STATUS);
        }
        break;
      }
      else{
        unlock_mutex();
        yield();
      }
    }

    if(local_pid % 2 == 0) {
      fork = right_fork;
      side = 1;
    }
    else {
      fork = left_fork;
      side = 0;
    }

    if(check(local_pid, fork, side) == 1){
      update_status(local_pid, EATING_STATUS);
      unlock_mutex();
      eating();
      thinking(local_pid);
      result(local_pid);
    }
    else{
      update_status(local_pid, HUNGRY_STATUS);
      unlock_mutex();
    }
  }
}
