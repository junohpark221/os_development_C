/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "spawn.h"

extern void main_Philosopher();

void main_spawn() {
  int stop = 0;
  while(1){
    for(int i = 0; i < 16; i++){
      if(fork() == 0){
        while(1){
          if(getpidm() == 0){
            setpidm(1);
            if(getpidm() == 1){
              setpid(i+1);
              stop = 1;
            }
          }
          if(stop != 0){
            break;
          }
        }
        setpidm(0);
        stop = 0;
        exec(&main_Philosopher);
      }
    }
    exit(0);
  }
  exit(EXIT_SUCCESS);
}
