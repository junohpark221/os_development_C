/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

#include "hilevel.h"

pcb_t procTab[ MAX_PROCS ]; pcb_t* executing = NULL; int priority[MAX_PROCS + 1]; int bPriority[MAX_PROCS + 1];
int mutex; unsigned int phil_status1; unsigned int phil_status2; int phil_pids; int pid_mutex; int state_mutex;

void dispatch( ctx_t* ctx, pcb_t* prev, pcb_t* next ) {
  char prev_pid = '?', next_pid = '?';

  if( NULL != prev ) {
    memcpy( &prev->ctx, ctx, sizeof( ctx_t ) ); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if( NULL != next ) {
    memcpy( ctx, &next->ctx, sizeof( ctx_t ) ); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc( UART0, '[',      true );
    PL011_putc( UART0, prev_pid, true );
    PL011_putc( UART0, '-',      true );
    PL011_putc( UART0, '>',      true );
    PL011_putc( UART0, next_pid, true );
    PL011_putc( UART0, ']',      true );

    executing = next;                           // update   executing process to P_{next}

  return;
}

void schedule( ctx_t* ctx ) {
  int max;
  priority[MAX_PROCS] = 0;
  for(int i = 0; i < MAX_PROCS; i++) {
    if (procTab[i].status == STATUS_READY) {
      priority[i] += 1;
    }
    if ((procTab[i].status == STATUS_READY || procTab[i].status == STATUS_EXECUTING) && priority[i] >= priority[MAX_PROCS]) {
      priority[MAX_PROCS] = priority[i];
      max = i;
    }
  }
  priority[max] = bPriority[max];

  if(procTab[executing->pid - 1].status = STATUS_EXECUTING){
    procTab[executing->pid - 1].status = STATUS_READY;
  }
  dispatch(ctx, &procTab[executing->pid - 1], &procTab[max]);
  procTab[max].status = STATUS_EXECUTING;

  return;
}

extern void     main_console();
extern uint32_t tos_P1;

void hilevel_handler_rst(ctx_t* ctx) {
  TIMER0->Timer1Load  = 0x00100000; // select period = 2^20 ticks ~= 1 sec
  TIMER0->Timer1Ctrl  = 0x00000002; // select 32-bit   timer
  TIMER0->Timer1Ctrl |= 0x00000040; // select periodic timer
  TIMER0->Timer1Ctrl |= 0x00000020; // enable          timer interrupt
  TIMER0->Timer1Ctrl |= 0x00000080; // enable          timer

  GICC0->PMR          = 0x000000F0; // unmask all            interrupts
  GICD0->ISENABLER1  |= 0x00000010; // enable timer          interrupt
  GICC0->CTLR         = 0x00000001; // enable GIC interface
  GICD0->CTLR         = 0x00000001; // enable GIC distributor

  int_enable_irq();

  mutex = 0;
  phil_status1 = 0;
  phil_status2 = 0;
  phil_pids = 0;
  pid_mutex = 0;
  state_mutex = 0;

  for( int i = 0; i < MAX_PROCS; i++ ) {
    procTab[ i ].status = STATUS_INVALID;
    bPriority[i] = 0;
    priority[i] = bPriority[i];
  }

  memset( &procTab[ 0 ], 0, sizeof( pcb_t ) );
  procTab[ 0 ].pid      = 1;
  procTab[ 0 ].status   = STATUS_READY;
  procTab[ 0 ].tos      = ( uint32_t )( &tos_P1  );
  procTab[ 0 ].ctx.cpsr = 0x50;
  procTab[ 0 ].ctx.pc   = ( uint32_t )( &main_console );
  procTab[ 0 ].ctx.sp   = procTab[ 0 ].tos;

  dispatch( ctx, NULL, &procTab[ 0 ] );

  return;
}

void hilevel_handler_irq(ctx_t* ctx) {
  // Step 2: read  the interrupt identifier so we know the source.

  uint32_t id = GICC0->IAR;

  // Step 4: handle the interrupt, then clear (or reset) the source.

  if( id == GIC_SOURCE_TIMER0 ) {
    schedule(ctx);
    TIMER0->Timer1IntClr = 0x01;
  }

  // Step 5: write the interrupt identifier to signal we're done.

  GICC0->EOIR = id;

  return;
}

void hilevel_handler_svc(ctx_t* ctx, uint32_t id) {
  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      schedule( ctx );
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );
      char*  x = ( char* )( ctx->gpr[ 1 ] );
      int    n = ( int   )( ctx->gpr[ 2 ] );

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }

      ctx->gpr[ 0 ] = n;

      break;
    }
    case 0x03 : { // 0x03 => fork()
      int procs = 0;
      int avail = 0;
      for(int i = 0; i < MAX_PROCS; i++){
        if(procTab[i].status == STATUS_READY || procTab[i].status == STATUS_EXECUTING){
          procs++;
        }
        if(procTab[i].status == STATUS_INVALID && avail == 0) {
          avail = i;
        }
      }

      if(procs < MAX_PROCS) {
        memcpy(&tos_P1 + ((avail - 1) * 0x00001000), &tos_P1 - 0x00001000, 0x00001000);
        procTab[avail].pid      = avail + 1;
        procTab[avail].status   = STATUS_READY;
        procTab[avail].tos      = (uint32_t)(&tos_P1 + ((avail - 1) * 0x00001000));
        memcpy(&procTab[avail].ctx, ctx, sizeof(ctx_t));
        procTab[avail].ctx.sp   = procTab[avail].tos;
        ctx->gpr[0] = procTab[avail].pid;
        procTab[avail].ctx.gpr[0] = 0;
      }
      break;
    }
    case 0x04 : { // 0x04 => exit( int x )
      procTab[executing->pid - 1].status = STATUS_INVALID;
      schedule(ctx);
      break;
    }
    case 0x05 : {  // 0x05 => exec(const void* x)
      ctx->cpsr = 0x50;
      ctx->pc = (uint32_t)(ctx->gpr[0]);
      for(int i = 0; i < 13; i++) {
        ctx->gpr[i] = 0;
      }
      ctx->sp = procTab[executing->pid - 1].tos;
      ctx->lr = 0;
      break;
    }
    case 0x06 : { // 0x06 => kill( pid_t pid, int x )
      int termpid = ctx->gpr[0];
      procTab[termpid].status = STATUS_TERMINATED;
      schedule(ctx);
      procTab[termpid].status = STATUS_INVALID;
      break;
    }
    case 0x07 : { // 0x07 => nice( pid_t pid, int x )
      int setpid = ctx->gpr[0];
      int newprio = ctx->gpr[1];
      bPriority[setpid - 1] = newprio;
      break;
    }
    case 0x08 : {   // 0x08 => getstat(int local_pid)
      int pid_loca = ctx->gpr[0];
      if(pid_loca == 1) {
        ctx->gpr[0] = phil_status1;
      }
      else if(pid_loca == 2) {
        ctx->gpr[0] = phil_status2;
      }
      break;
    }
    case 0x09 : {   // 0x09 => setstat(unsigned int state, int local_pid)
      int pid_loca = ctx->gpr[1];
      unsigned int value = ctx->gpr[0];
      if(pid_loca == 1) {
        phil_status1 = value;
      }
      else if(pid_loca == 2) {
        phil_status2 = value;
      }
      break;
    }
    case 0x0a : {   // 0x0a => getmut()
      ctx->gpr[0] = mutex;
    }
    case 0x0b : {   // 0x0b => setmut(int mutex)
      mutex = ctx->gpr[0];
      break;
    }
    case 0x0c : {   // 0x0c => getpid()
      ctx->gpr[0] = phil_pids;
      break;
    }
    case 0x0d: {    // 0x0d => setpid(int pid)
      phil_pids = ctx->gpr[0];
      break;
    }
    case 0x0e : {   // 0x0e => getpidm()
      ctx->gpr[0] = pid_mutex;
      break;
    }
    case 0x0f : {   // 0x0f => setpidm()
      pid_mutex = ctx->gpr[0];
      break;
    }
    case 0x10 : {   // 0x10 => getstatem()
      ctx->gpr[0] = state_mutex;
      break;
    }
    case 0x11 : {   // 0x11 => setstatem()
      state_mutex = ctx->gpr[0];
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
