/* spawn_tasks.c */

#include <vxWorks.h> /* include this as the first thing in every program */
#include <stdio.h> /* we use printf */
#include <taskLib.h> /* we use taskSpawn */
#include <sysLib.h> /* we use sysClk... */
#include <private\trgLibP.h>
int global_howmany=-1;


int printing(int arg) /* subroutine to be spawned */
{
   int k; /* counter to repeat printing "arg" times */
   for (k=0;k<arg;k++) 
   {
      printf("I am task %d, iter: #%d\n",taskIdSelf(), k); /* Print task Id */
      taskDelay(sysClkRateGet()/60); /* delay for 1/60 of second */
   }
   /*trigger user-event 10 to make WV stop logging when the last
    task has finished printing. Assuming all tasks are spawned 
    with the same priority level. If this is not the case, a
    different triggering method needs to be used to stop WV.*/

   if (arg==global_howmany) 
   {
      trgEvent(40010);
      global_howmany=-1;
   }
   return(0);
}

void spawn_tasks(int howmany) /* subroutine to perform the spawning */
{
   int i, taskId;
   global_howmany=howmany;
   for(i=1; i <= howmany; i++)  /* Creates "howmany" tasks */
   {
      taskId = taskSpawn(NULL,90,0x100,2000,(FUNCPTR)printing,i,0,0,0,0,0,0,0,0,0);
   }
}
