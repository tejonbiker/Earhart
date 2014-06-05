#include "imuboard.h"
#include "math.h"
#include "pwm.h"
#include "stdlib.h"

float raw[24];
float raw_before[24];
int counter=0;
int total_counter=0;
float yaw,pitch,roll;
int discard_values=1000;

//Yaw,Pitch and Roll
float targets[3]={0,0,0};
//Kpitch=3,Kroll=7, Kyaw=7
//Kpitch=1,Kroll=1,Kyaw=0.0;
float Kpitch=0.0,Kroll=0.0,Kyaw=0.0; 
float errors[3];
int omega[6];
float WBase=1100;
float WBase_1=1100;


#define MOTORS

int esc[]={24,17,4,27,25,22,23};

int channel=0;

float max_pitch=0.0f;
int mid_seconds=0;

int main(void)
{

	FILE *log=NULL;
	int i,j;
	if(IMUBInit()<0){
		printf("Error with IMU appear\n");
		exit(0);
	}
        IMUB_DLPF(6);
	IMUBAccelScale(16);

	log=fopen("log_accel_damping_rug_16g.txt","w");
	
	#ifdef MOTORS
	//Setup PWM
	setup(1,DELAY_VIA_PWM);
	init_channel(channel, SUBCYCLE_TIME_US_DEFAULT);
	
	for(i=0;i<6;i++)
		add_channel_pulse(channel, esc[i], 0, 1000);	
	#endif

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
			printf("y,p,r: %f,%f,%f\n",yaw,pitch,roll);

			for(i=0;i<6;i++)
				printf("%f ",omega[i]);

			printf("MP: %f",max_pitch);
			printf("\n");
			fflush(log);


			mid_seconds++;

			if(mid_seconds>0 && mid_seconds<=40)
			{
				WBase = WBase_1 + 300*(mid_seconds/40.0f) ;
			}
			else if(mid_seconds>40)
			{
				WBase=1300;
			}

			printf(" Power: %f ",WBase);
		}

		yaw+=raw[9]/512.0f;
		pitch =(180.0/M_PI)*atan(   raw[3]/sqrt(raw[4]*raw[4] + raw[5]*raw[5])    );
		roll = (180.0/M_PI)*atan(   raw[4]/sqrt(raw[3]*raw[3] + raw[5]*raw[5])    );
	
		if(fabs(pitch)>max_pitch)
			max_pitch=fabs(pitch);	

		errors[0]=targets[0]-yaw;
		errors[1]=(targets[1]-pitch);
		errors[2]=-(targets[2]-roll);

			  //Yaw segment           Pitch		    Roll
		omega[0]=  errors[0]*Kyaw    +   errors[1]*Kpitch                         + WBase;
		omega[1]=  -errors[0]*Kyaw   +   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[2]=  errors[0]*Kyaw    -   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[3]= -errors[0]*Kyaw    -   errors[1]*Kpitch    			  + WBase;
		omega[4]=  errors[0]*Kyaw    -   errors[1]*Kpitch    - errors[2]*Kroll 	  + WBase;
		omega[5]= -errors[0]*Kyaw    +   errors[1]*Kpitch    - errors[2]*Kroll    + WBase;


		//fprintf(log,"%f, %f, %f",errors[0],errors[1],errors[2]);
		fprintf(log,"%f, %f, %f",raw[3],raw[4],raw[5]);

		#ifdef MOTORS
		for(i=0;i<6;i++)
		{
			//fprintf(log,"%i, ",omega[i]);
			add_channel_pulse(channel, esc[i], 0, omega[i]);
			//add_channel_pulse(channel, esc[i], 0, 1200);
		}		
		#endif	

		fprintf(log,"\n");
		
	}
	
	printf("Total counter: %i\n", total_counter);

	return 0;
}
