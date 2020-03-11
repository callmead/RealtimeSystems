#include <vxWorks.h> /* Always include this as the first thing in every program */
#include <time.h>	/* we use clock_gettime */
#include <taskLib.h>  /* we use tasks */
#include <sysLib.h>   /* we use sysClk... */
#include <semLib.h>   /* we use semaphores */
#include <logLib.h>   /* we use logMsg rather than printf */

/* define useful constants for timing */
#define NANOS_IN_SEC 1000000000
#define NANOS_PER_MILLI 1000000
#define TICK sysClkRateGet()/60

/* function prototypes */ 
void SensorP(int);
void SensorM(int); 
void Display(int);

/* globals */
#define ITER 22   /* arbitrary number of iterations – can be changed */

SEM_ID semMtx; /* a semaphore supporting mutual exclusion */
/* only the task "taking" semaphore can "give" it */ int taskSensorP, taskSensorM,taskDisplay; /* task references */
/* our "shared memory" area: three data to be kept coherent */
/* i.e. they need to show the same values when printing */ struct mem{
int x; int y; int z;
} data;

/* a routine createM to create "mutex" semaphore - can be also done from the shell line  */
/* queue tasks on FIFO basis and deletion safety	*/ 
void createM()
{
semMtx = semMCreate(SEM_Q_FIFO | SEM_DELETE_SAFE);
}

/* the main program named mutex creating semaphore and spawning three working tasks */ 
void mutex(int protect)
{
/* clear the memory */
data.x = 0; data.y = 0; data.z = 0;

/* spawn three tasks */
taskDisplay = taskSpawn("td", 95,0x100,2000,(FUNCPTR)Display,protect,0,0,0,0,0,0,0,0,0); 
taskSensorP = taskSpawn("tsp",95,0x100,2000,(FUNCPTR)SensorP,protect,0,0,0,0,0,0,0,0,0); 
taskSensorM = taskSpawn("tsm",95,0x100,2000,(FUNCPTR)SensorM,protect,0,0,0,0,0,0,0,0,0); 
taskDelay(220*TICK);    /* delay arbitrary # "ticks" before terminating Display task */ 
taskDelete(taskDisplay);
}

/* the "Display" routine printing the contents of shared memory */
/* every ten time "ticks"; */
/* the protect argument controls whether or not the semaphore */
/* will be used (1 - used, 0 - not used) */ 
void Display(int protect)
{
/* preparation for time computation */ 
struct timespec tpstart, tpend;
int count=0, isec, insec, milli_sec; 
clock_gettime(CLOCK_REALTIME, &tpstart);
/* loop forever (until the task get killed) */ 
while(1)
{
/* "critical section" - wait indefinitely for semaphore, if protect = 1 */ 
if(protect)  semTake(semMtx,WAIT_FOREVER);

/* beginning of the "critical section" for printing */
/* necessary computations to display current time */ 
clock_gettime(CLOCK_REALTIME, &tpend);
isec = tpend.tv_sec - tpstart.tv_sec; 
insec = tpend.tv_nsec - tpstart.tv_nsec;
if (insec < 0) { 
insec = insec + NANOS_IN_SEC; isec--; };
milli_sec=insec/NANOS_PER_MILLI;

/* we use VxWorks logMsg rather than printf - as printf may block */ 
logMsg("Display #%d=> %d %d %d at %d sec and %d milli_sec\n",count++,data.x,data.y,data.z,isec,milli_sec);

/* end of the" critical section" give up semaphore, if protect = 1 */ 
if(protect) 
semGive(semMtx);
/* clear the memory for the next printing */ 
data.x = 0; 
data.y = 0; 
data.z = 0;

taskDelay(22*TICK); /* delay arbitrary # ticks - periodic task */
}
}

/* the "sensor Plus" routine increasing the shared memory ITER times; */
/* the protect argument controls whether or not the semaphore will */
/* be used (1 - used, 0 - not used) */ 
void SensorP(int protect)
{
int i;
for (i=0; i < ITER; i++)
{
/* "critical section" - wait indefinitely for semaphore, if protect = 1 */ 
if(protect) semTake(semMtx,WAIT_FOREVER);
/* beginning of the the "critical section" with simulated operation delay */ 
data.x++; taskDelay(7*TICK);
data.y++; taskDelay(1*TICK); 
data.z++; taskDelay(2*TICK);
/* end of the" critical section" - give up semaphore, if protect = 1 */ 
if(protect) semGive(semMtx);
}
}

/* the "sensor Minus" routine decreasing the shared memory ITER times; */
/* the protect argument controls whether or not the semaphore will */
/* be used (1 - used, 0 - not used) */ 
void SensorM(int protect)
{
int i;
for (i=0; i < ITER; i++)
{
/* "critical section" - wait indefinitely for semaphore, if protect = 1 */ 
if(protect) semTake(semMtx,WAIT_FOREVER);
/* beginning of the the "critical section" with simulated operation delay */ 
data.x--; taskDelay(1*TICK);
data.y--; taskDelay(6*TICK); 
data.z--; taskDelay(3*TICK);
/*  end of the" critical section" - give up semaphore, if protect = 1 */ 
if(protect) semGive(semMtx);
}
}
