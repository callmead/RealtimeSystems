#include <vxWorks.h> 	/* Always include this as the first thing in every program */
#include <time.h>		/* we use clock_gettime */
#include <taskLib.h>  	/* we use tasks */
#include <sysLib.h>   	/* we use sysClk... */
#include <semLib.h>   	/* we use semaphores */
#include <logLib.h>   	/* we use logMsg rather than printf */

/* define useful constants for timing */
#define NANOS_IN_SEC 1000000000
#define NANOS_PER_MILLI 1000000
#define TICK sysClkRateGet()/60

/* globals */
#define ITER 22	/* arbitrary number of iterations – can be changed */

/* function prototypes */ 
void Sensor(); 
void Display();

SEM_ID semBin; 	/* a semaphore supporting mutual exclusion */
/* only the task "taking" semaphore can "give" it */ 
int taskSensor,taskDisplay;	/* task references */
/* our "shared memory" area: three data to be kept coherent */
/* i.e. they need to show the same values when printing */ 
struct mem{
	int x; int y; int z;
} data;

/* a routine createB to create a binary semaphore - can be also done from the shell line  */
/* queue tasks on FIFO basis and deletion safety	*/ 
void createB(){
	semBin = semBCreate(0, 1);
}

/* the main program named mutex creating semaphore and spawning three working tasks */ 
void binary(){
	/* clear the memory */
	data.x = 0; data.y = 0; data.z = 0;

	/* spawn three tasks */
	taskSensor = taskSpawn("ts",95,0x100,2000,(FUNCPTR)Sensor,0,0,0,0,0,0,0,0,0,0);
	taskDisplay = taskSpawn("td", 95,0x100,2000,(FUNCPTR)Display,0,0,0,0,0,0,0,0,0,0); 
	taskDelay(220*TICK);    /* delay arbitrary # "ticks" before terminating Display task */ 
	taskDelete(taskDisplay);
}

/* the "sensor" routine */
void Sensor(){
	int i;
	for (i=0; i < ITER; i++){
		/* "critical section" - wait indefinitely for semaphore */ 
		semTake(semBin, WAIT_FOREVER);
		/* beginning of the the "critical section" with simulated operation delay */ 
		data.x++; data.y++; data.z++;
		/* end of the" critical section" - give up semaphore, if protect = 1 */ 
		semGive(semBin);
		taskDelay(22*TICK); /* delay arbitrary # ticks - periodic task */		
	}
}

/* the "Display" routine */
void Display(int protect){
	int i=1; 
	/* loop forever (until the task get killed) */ 
	while(1){
		/* "critical section" - wait indefinitely for semaphore, if protect = 1 */ 
		semTake(semBin,WAIT_FOREVER);
		/* beginning of the "critical section" for printing */
		/* we use VxWorks logMsg rather than printf - as printf may block */ 
		logMsg("Display #%d x: %d, y: %d, z: %d %d %d\n", i++, data.x, data.y, data.z, 0, 0);
		semGive(semBin);
		/* clear the memory for the next printing */ 
		data.x = 0; data.y = 0; data.z = 0;
		taskDelay(22*TICK); /* delay arbitrary # ticks - periodic task */
	}
}

