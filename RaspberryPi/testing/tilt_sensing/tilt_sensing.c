/*
    	Capture the MPU6050 data only, no pwm or other things are involved,
	this file is created to sense the tilt of a steel sheet during a stress process

    	https://gist.github.com/silv3rm00n/5821760
    	Enable 400 kHz I2C clock rate with:
		
    	sudo modprobe -r i2c_bcm2708
    	sudo modprobe i2c_bcm2708 baudrate=400000

	Federico Ramos
	Francisco Moya Ibanez

	Universidad de Guanajauto
	DICIS
	March 2015
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<unistd.h>    //write
#include<sys/time.h>
#include<signal.h>


#include "imuboard.h"
#include "math.h"
#include "stdlib.h"

 
void *control_handler(void *);

float raw[24];
int counter=0;
int total_counter=0;

FILE *logIMU=NULL;

//for timing
unsigned int RefreshTimer()
{
	unsigned int result = 0;
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == 0)
	{
		tv.tv_sec &= 0x003FFFFF;        // to prevent overflow from *1000, we only care about relative time anyway
		result = (unsigned int) ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
	}
	else
	{
		printf("gettimeofday failed");
	}

	return result;
}

//monitor ctrl +c event to end properly the log
void my_handler(int s){
	printf("Exit...\n",s);
	fprintf(logIMU,"];");
	fflush(logIMU);
	exit(1);
}

int main(int argc , char *argv[])
{

    	int i,j;
	struct sigaction sigIntHandler;

	
	//catch the ctrl+c (exit combination) event to end properly the log
	sigIntHandler.sa_handler=my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags=0;
	sigaction(SIGINT,&sigIntHandler,NULL);

	//Init the IMU Board
    	if(IMUBInit()<0){
		printf("Error with IMU appear\n");
		exit(0);
    	}

	IMUB_DLPF(0); 		//Setup the Digital Low Pass Filter level
	IMUBAccelScale(2);	//Setup the scale of accelerometer to 16G

	//Open the log (file where the output of the IMU will be saved)
	if(argc>=2)
	{
		logIMU=fopen(argv[1],"w");
		if(logIMU==NULL)
		{
			printf("Error to open user file: %s\n",argv[1]);
			return -1;
		}
	}
	else
	{
		printf("Using default name: IMU_log.txt\n");
		logIMU=fopen("IMU_log.txt","w");
	}

	fprintf(logIMU,"accel=[");
	control_handler(NULL);
     
    return 0;
}
 
void *control_handler(void *socket_desc)
{

	int i;
	unsigned int startTime = RefreshTimer();
	unsigned int lastTime;
	int counter_seconds=1;

	printf("Starting IMU...\n");

	IMUB_DLPF(6);         //Go from 0 to 6, a more high value indicates a more low frecuency cut of the filter
	IMUBSampleRate(100);  //Go from 0 to 255, a more high value indicates a slow sample rate

	//Start with an infinite loop	
    	while(1)
	{
		counter++;

		 
		lastTime=RefreshTimer();
		if(lastTime - startTime >= 1000)
		{
		        printf("samples!: %i, time (ms): %u, ", counter,lastTime - startTime);
			printf("Angle Roll: %f  \n",atan2(-raw[4],sqrt(raw[5]*raw[5] + raw[6]*raw[6])) );
			startTime=lastTime;
			counter=0;

		}
		IMUBPollRaw(raw);

					//   X      Y      Z
		fprintf(logIMU,"%f  %f  %f",raw[3],raw[4],raw[5]);
		fprintf(logIMU,"\n");

	}

    
    return 0;
}