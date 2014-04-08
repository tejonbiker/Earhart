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

int main(void)
{
	int i,j;
	IMUBInit();


	
	//Setup PWM
	setup(1,DELAY_VIA_PWM);
	init_channel(channel, SUBCYCLE_TIME_US_DEFAULT);

	
	for(i=0;i<6;i++)
		add_channel_pulse(channel, esc[i], 0, 1000);	
	

	usleep(5000000);

	while(1)
	{
		IMUBPollRaw(raw);
		if(memcmp(raw,raw_before,sizeof(float)*10)!=0)
		{
			memcpy(raw_before,raw,sizeof(float)*10);
		}else
		{continue;}

		/*
		discard_values--;
		if(discard_values!=0)
			continue;
		*/

		total_counter++;

		if(total_counter==250)
		{
			total_counter=0;
			pitch =(180.0/M_PI)*atan(   raw[3]/sqrt(raw[4]*raw[4] + raw[5]*raw[5])    );
			roll = (180.0/M_PI)*atan(   raw[4]/sqrt(raw[3]*raw[3] + raw[5]*raw[5])    );
			printf("y,p,r: %f,%f,%f\n",yaw,pitch,roll);

			for(i=0;i<6;i++)
				printf("%f ",omega[i]);
			printf("\n");
		}

		yaw+=raw[9]/512.0f;

		errors[0]=targets[0]-yaw;
		errors[1]=-(targets[1]-pitch);
		errors[2]=targets[2]-roll;

			  //Yaw segment           Pitch		    Roll
		omega[0]=  errors[0]*Kyaw    +   errors[1]*Kpitch                         + WBase;
		omega[1]=  -errors[0]*Kyaw   +   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[2]=  errors[0]*Kyaw    -   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[3]= -errors[0]*Kyaw    -   errors[1]*Kpitch    			  + WBase;
		omega[4]=  errors[0]*Kyaw    -   errors[1]*Kpitch    - errors[2]*Kroll 	  + WBase;
		omega[5]= -errors[0]*Kyaw    +   errors[1]*Kpitch    - errors[2]*Kroll    + WBase;


		
		for(i=0;i<6;i++)
			add_channel_pulse(channel, esc[i], 0, omega[i]);			
		
	}
	
	printf("Total counter: %i\n", total_counter);

	return 0;
}