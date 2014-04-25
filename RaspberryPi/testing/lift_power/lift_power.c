#include "imuboard.h"
#include "math.h"
#include "pwm.h"

float raw[24];
float raw_before[24];
int counter=0;
int total_counter=0;
float yaw,pitch,roll;
int discard_values=1000;

//Yaw,Pitch and Roll
float targets[3]={0,0,0};
float Kpitch=0,Kroll=7,Kyaw=0; 
float errors[3];
float omega[6];
float WBase=1150;

int esc[]={24,23,4,27,25,22,17};

int channel=0;

int j;

int main(void)
{

	FILE *log=NULL;
	int i,j;
	
	//Setup PWM
	setup(1,DELAY_VIA_PWM);
	init_channel(channel, SUBCYCLE_TIME_US_DEFAULT);

	
	for(i=0;i<6;i++)
		add_channel_pulse(channel, esc[i], 0, 1000);	
	
	usleep(5000000);

	for(i=0;i<600;i++)
	{
		for(j=0;j<6;j++)
			add_channel_pulse(channel, esc[j], 0, 1000+i);

		printf("%i\n",1000+i);
		usleep(50000);
	}

	shutdown();	
	return 0;
}