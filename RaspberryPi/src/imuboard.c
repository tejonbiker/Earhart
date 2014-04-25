/*
	A short library to manage the connections and calculus of MPU6050
	and HMC5883L intertial sensors
*/

#include "imuboard.h"

static int i2c_port=0;

int IMUBInit()	//Open the I2C Channel and verify the sensors
{
	unsigned char buffer[10];	

	//The I2c port was openned?
	if(i2c_port!=0)
	{
		printf("I2C port was openned, please close first!\n");
		return -1;
	}

	//Open the port!!!
	i2c_port=open("/dev/i2c-1",O_RDWR);
	
	//Verify we have the port opened
	if(i2c_port<0)
	{
		//If we don't have the port available, suggest enable 
		printf("I can't open i2c, ensure you have enabled first!\n");
		printf("ex: gpio load i2c 400\n");
		return -2;
	}

	//Verify we have the MPU on the i2c bus
	if(ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR)<0)
	{
		printf("Can't Connect to MPU6050\n");
		return -3;
	}

	//Verify we have the HMC on the i2c bus
	if(ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR)<0)
	{
		printf("Can't Connect to HMC5883L\n");
	}

	//first setup mpu6050
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x6B;  //Register
	buffer[1]=0x00;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Power Enable

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x21;  //Register
	buffer[1]=0x01;  //Value
	write(i2c_port,buffer,2); //Write to i2C, 16 bit format

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x1A;  //Register
	buffer[1]=0x01;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Disable FSYNC and  enable DLPF at 1 level

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x19;  //Register
	buffer[1]=0x01;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Sample Rate division

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x1C;  //Register
	buffer[1]=0x00;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Accelerometer scale 2G


	//let's continue with HMC5883L
	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
	buffer[0]=0x00;  //Register
	buffer[1]=0x70;  //Value
	write(i2c_port,buffer,2); //Write to i2C, 8-bit avg, 15 Hz

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
	buffer[0]=0x01;  //Register
	buffer[1]=0xA0;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Gain=5

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
	buffer[0]=0x02;  //Register
	buffer[1]=0x00;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Continuos measurements

	//HMC and MPU Configurated, let's continue with the code
	return 0;
	
}

int IMUB_DLPF(int level)
{
	unsigned char buffer[2];

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x1A;  //Register
	buffer[1]=level | 7 ;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Disable FSYNC and  enable DLPF at 1 level
}

int IMUBSampleRate(int sample_rate) //Setup the sample rate
{

}

int IMUBPollRaw(float *data_dest) //Use  Poll method to sample the raw data
{

	static char ptr_buffer_before[14];
	static int firstRead=1;
	int different=1;
	int j;

	unsigned char ptr_buffer_current[256], *buffer_read;
	int i;
	int result,low,high;
	unsigned char ptr_buffer_mpu[256];
	int total_read=0;

	if(data_dest==NULL)
		return -1;

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
	ptr_buffer_current[0]=0x03;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);
	result=read(i2c_port,ptr_buffer_current,6);

	buffer_read=ptr_buffer_current;

	for(i=0;i<3;i++)
	{
		low=buffer_read[i*2+1];
		high=buffer_read[i*2];
		data_dest[i]=(short)(((unsigned char)low) + ((unsigned char)(high))*256);
	}

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	ptr_buffer_current[0]=0x3B;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);
	result=read(i2c_port,ptr_buffer_mpu,14); 

	/*
	do{
		ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
		ptr_buffer_current[0]=0x3B;                  //init register of the data
		result=write(i2c_port,ptr_buffer_current,1);
		result=read(i2c_port,ptr_buffer_mpu,14); 

		total_read++;

		if(firstRead==1)
		{
			different=1;
			firstRead=0;
			memcpy(ptr_buffer_before,ptr_buffer_mpu,14);
			//printf("first read\n");
		}else{
			if(memcmp(ptr_buffer_mpu,ptr_buffer_before,14)==0)
			{
				//different=0;
				//printf("Equal Sample\n");
			}
			else
			{
				different=1;
				memcpy(ptr_buffer_before,ptr_buffer_mpu,14);
			}
		}
	}while(different!=1);
	*/

	//printf("total Read: %i \n",total_read++);

	for(i=0;i<3;i++)
	{
		low=ptr_buffer_mpu[i*2+1];
		high=ptr_buffer_mpu[i*2];
		data_dest[i+3]=((short)(((unsigned char)low) + ((unsigned char)(high))*256))*2/32767.0f;
	}

	for(i=4;i<7;i++)
	{
		low=ptr_buffer_mpu[i*2+1];
		high=ptr_buffer_mpu[i*2];
		data_dest[i+3]=((short)(((unsigned char)low) + ((unsigned char)(high))*256))*250/32767.0f;
	}

}

int IMUBPOllAngles(float *yaw, float *pitch, float *roll) //Use poll method to sample the processed angles
{

}

int IMUBClose() //close all resources
{

}