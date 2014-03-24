/*
	Code fixed by using 
	https://github.com/richardghirst/PiBits/tree/master/MPU6050-Pi-Demo
	as reference.

	This code is to test the MPU6050, Acceleromete+ Gyroscopepe in efficient way.
	The mpu6050  have an i2c interface, to run you need before run this:
	
	gpio load i2c 400
	
	This loads the i2c kernel, this example only extract the data and print on screen
	or print to log the raw data
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

//RPi headers!!!
#include <wiringPi.h> 
#include <wiringPiI2C.h>
#include <linux/i2c-dev.h>

#define RPI_SERVER

#define LED 0

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	int pid;
	char buffer[256];
	int i;
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	int accel=0;
	int low,high;
	short accelerations[3];
	int address;
	FILE *mpu_log_ptr=NULL;
	int	data_written=0;
	short MPU_RAM_LOG[20000*3];
	int sample_print=300;
	clock_t start_time,end_time;
	unsigned char buffer_read[14];
	unsigned char buffer_before[14];
	unsigned char *ptr_buffer_current=buffer_read;
	unsigned char *ptr_buffer_before=buffer_before;
	unsigned char *ptr_exchange_buffer=NULL;
	int result;
	off_t real_offset;
	int total_read=0;


	address=0x68;
	accel=wiringPiI2CSetup(address);

	if(accel<0)
	{
		printf("Error to access Acceleromteter");
		return 0;
	}

	printf("Conected to %x, result connection: %i\n",address,accel);

	//Enable The power up of the accelerometer
	wiringPiI2CWriteReg8(accel,0x6B,0x00);
	//Enable 16 bit format
	wiringPiI2CWriteReg8(accel,0x21,0x01);
	//Disable FSYNC and enable 1 KHz sample rate (disable DLPF)
	wiringPiI2CWriteReg8(accel,0x1A,0x00);
	//disable sample rate division
	wiringPiI2CWriteReg8(accel,0x19,0x00);

	//Open the file log
	mpu_log_ptr=fopen("mpu6050.log","wt");

	if(mpu_log_ptr==NULL)
	{
		printf("Sorry, the file cannot be created, be sure you have permissions\n");
		return -1;
	}
	
	memset(ptr_buffer_before,0,14);

	
	start_time=clock();
	i=0;
	while(1)
	{
		ioctl(accel,I2C_SLAVE,0x68); //The slave that we want talk
		ptr_buffer_current[0]=0x3B;  //The initial register to read              
		result=write(accel,ptr_buffer_current,1); //Write the dir
		result=read(accel,ptr_buffer_current,14); //Read 14 bytes from memory of the mpu6050
							  //for more info see the datasheet
		
		//Are different sample the current and the before? (avoid oversample)
		result=memcmp(ptr_buffer_before,ptr_buffer_current,14);

		if(result!=0)
		{	
			//Track the total readings
			data_written++;

			//Converto to signed short int, a type that C understand better
			//We have 7 varaibles, Accelerometer X,Y,Z, Gyroscope R,P, Y and 
			//Temperature
			for(i=0;i<7;i++)
			{	

				low=buffer_read[i*2+1];
				high=buffer_read[i*2];
				accelerations[2]=(short)(((unsigned char)low) + ((unsigned char)(high))*256);
				
				//Write to log, the special case is the temperature conversion
				if(i==3)
					//temperature
					fprintf(mpu_log_ptr,"%f \t",accelerations[2]/340.0f +36.53f);
					//result++;
				else
					fprintf(mpu_log_ptr,"%hi \t",accelerations[2]);
					//result--;
			}
			fprintf(mpu_log_ptr,"\n");
		}

		//Exchange the buffers
		ptr_exchange_buffer=ptr_buffer_current;
		ptr_buffer_current=ptr_buffer_before;
		ptr_buffer_before=ptr_exchange_buffer;


		//Force file flush every 1000 samples
		total_read++;
		sample_print=1000;
		if(data_written==sample_print)
		{
			end_time=clock();
			printf("%i sampled, time %lf, total read: %i\n", sample_print,(double)(end_time - start_time)/CLOCKS_PER_SEC,total_read);
			fflush(mpu_log_ptr);
			data_written=0;
			total_read=0;
			start_time=end_time;
		}
		
	}
}
