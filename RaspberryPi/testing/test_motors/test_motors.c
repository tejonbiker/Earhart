#include <math.h>
#include "pwm.h"
#include <stdlib.h>
#include <stdio.h>

float raw[24];
float raw_before[24];
int counter=0;
int total_counter=0;
float yaw,pitch,roll;
int discard_values=1000;


int omega[6];
float WBase=1100;
float WBase_1=1100;

int max_power;
int time;

#define MOTORS

int esc[]={24,17,4,27,25,22,23};

int channel=0;

float max_pitch=0.0f;
int mid_seconds=0;

int main(int argc, char **argv)
{
	int end_profiler=0;
	int i,j,motor_loop;
	int time_steps;
	int current_motor=0;
	int sleep_steps=3*2;


	if(argc>=2)
	{
		max_power=atoi(argv[1]);
		time = atoi(argv[2]);
	}
	else
	{
		max_power=50;
		time=20;
		
		
	}

	printf("Parameters: max_power=%i, time=%i\n",max_power,time);

	time_steps=time*2;
	
	#ifdef MOTORS
	//Setup PWM
	setup(1,DELAY_VIA_PWM);
	init_channel(channel, SUBCYCLE_TIME_US_DEFAULT);
	
	for(i=0;i<6;i++)
		add_channel_pulse(channel, esc[i], 0, 1000);	
	#endif

	usleep(5000000);


	while(!end_profiler)
	{

		total_counter++;
		
		usleep(2000);


		if(total_counter==250)
		{
			total_counter=0;

			mid_seconds++;

			if(mid_seconds>0 && mid_seconds<=time_steps)
			{
				WBase = WBase_1 + max_power*(mid_seconds/((float)time_steps)) ;
			}
			else if(mid_seconds>time_steps && mid_seconds<=time_steps+sleep_steps)
			{
				WBase=max_power+WBase_1;
			}
			else if(mid_seconds>time_steps+sleep_steps)
			{
				add_channel_pulse(channel, esc[current_motor], 0, 1000);
				
				current_motor++;

				if(current_motor>=6)
				{
					end_profiler=1;
					continue;
				}

				mid_seconds=0;
				usleep(3000000);
			}

			printf(" Power: %f \n",WBase);
		}

		#ifdef MOTORS
		add_channel_pulse(channel, esc[current_motor], 0, WBase);
		#endif	
	
	}
	
	
	pwm_shutdown();
        exit(0);

	return 0;
}
