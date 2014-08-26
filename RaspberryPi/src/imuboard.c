/*
	Copyright Federico Ramos
	
	Air Image HD

	A short library to manage the connections and calculus of MPU6050
	and HMC5883L intertial sensors

	This library manage the i2c stuff, how it work? well pretty easy!

	Firt of all we need open the i2c channel in code (and in kernel system)
	the i2c channel is opened like a file descriptor.

	The i2c protocol is a broadcast protocol (multiple devices hearing the messages all the time
	and responding if they are called), i2c is simplex, this mean you can only send or receive 
	but not both at the same time.

	The first step in i2c protocol is tell to the kernel we are a master device (in other words, the 
	device who generate the clock to communicate), this is accomplished with this command:

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);

	-- i2c_port 		is the descriptor of the i2c channel
	-- I2C_SLAVE 		is a constant that tells we are in master mode
	-- HMC_SLAVE_ADDR 	is a constant that tells the address of the chip to work


	After of this, we have the slave ready to hear the messages of the master (Raspberry),we can do two things:
	Read or Write, both of them requires very similar steps with a little difference.

	------- Write i2C Slave ----------
	
	To write to i2c slave we need specify two things, the register and the value of the register.
	Example:

	MPU6050:
	Register (Hex)		Value
	0x6B			0x00

	So, we need to transfer this two things into the same call to i2c channel, for this we use an array
	of unsigned char, like this:
	
	unsigned char buffer[10];

	And use the write function (this flush the array to the i2c channel), the complete sequence is like this:

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x6B;  //Register
	buffer[1]=0x80;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Power Enable

	After of this the register is as follow:

	MPU6050:
	Register (Hex)		Value
	0x6B			0x80

	Note: you can modify consecutive register with a more big array transfer (some chips have incremental read/writes 
	address).

	------- Read i2C Slave ----------

	Read the slave need a more tricky method, firt of all we need to know the register to read, example the accelerometer
	data out (from 0x3B to 0x40 to read the three axis), we start with 0x3B (ACCEL_XOUT) and enter the RPi in master mode.

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);

	We need to write to i2c channel the register to star to reading (0x3B):

	ptr_buffer_current[0]=0x3B;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);

	After write we read the registers, in this case 6 consecutive registers and puts in a array.

	result=read(i2c_port,ptr_buffer_mpu,6);

	With this we can use a very efficient way to read the MPU6050 chip, the fastest way I've seen in the web

	All the things looke like this:
	
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	ptr_buffer_current[0]=0x3B;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);
	result=read(i2c_port,ptr_buffer_mpu,6); 
*/

#include "imuboard.h"

static int i2c_port=0;
static float accel_scale=2; //By default

int IMUBInit()	//Open the I2C Channel and verify the sensors
{
	unsigned char buffer[10];	
	int end_reset;

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

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
        buffer[0]=0x75;  	//Register, Who am I
        write(i2c_port,buffer,1); //Write register
	read(i2c_port,buffer,1);  //read register

	if(buffer[0]!=0x68)
	{
	 	printf("MPU6050 not respond\n");
		return -3;
	}



	//Verify we have the HMC on the i2c bus
	if(ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR)<0)
	{
		printf("Can't Connect to HMC5883L\n");
		return -3;
	}

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
        buffer[0]=0x0A;           //Register, Who am I
        write(i2c_port,buffer,1); //Write register
        read(i2c_port,buffer,1);  //read register

        if(buffer[0]!='H')
        {
                printf("HMC5883L not respond\n");
                return -3;
        }

	/*
	//reset mpu6050
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x6B;  //Register
	buffer[1]=0x80;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Power Enable

	//wait for the reset of all registers!
	end_reset=1;
	while(end_reset!=0)
	{
		ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
		buffer[0]=0x6B;  		//Register
		write(i2c_port,buffer,1); 	//Read to i2C, Power Enable
		read(i2c_port,buffer,1); 	//Read to i2C, Power Enable

		printf("waiting %x...\n",(buffer[0]&0x80));

		if((buffer[0] & 0x80) == 0 )
		{
			end_reset=0;
		}
	}
	*/

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

/**
   Setup the Digital Low Pass Filter (aka DLFP), there are several levels in MPU6050:

	Level(value)	Bandwith(Hz)   Delay(ms)
	0		260		0
	1		184		2
	2		94		3
	3		44		4.9
	4		21		8.5
	5		10		13.8
	6		5		19
	7		Reserved

	See the datasheet for more info
*/
int IMUB_DLPF(int level)
{
	unsigned char buffer[2];


	if(level<0 || level>7)
		return -1;

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x1A;  //Register
	buffer[1]=(level);  //Value
	write(i2c_port,buffer,2); //Write to i2C, Disable FSYNC and  enable DLPF

	/*
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x1A;  //Register
	buffer[1]=0x01;  //Value
	write(i2c_port,buffer,2); //Write to i2C, Disable FSYNC and  enable DLPF at 1 level
	*/
}

int IMUBSampleRate(int sample_rate_divider) //Setup the sample rate
{	
	/*
	Chip: MPU6050
	Register involved: 0x19 - SMPLRT_DIV
	Mean: This register set the divider from the gyroscope output rate (8 bit unsigned)
	Sample Rate = Gyro Output Rate / (1 + SMPLRT_DIV)
	Warning: Gyro Output Rate is affected by DLPF (8 kHz if disabled, otherwise 1 kHz)
	*/
	unsigned char buffer[2];

	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	buffer[0]=0x19;  				//Register:SMPLRT_DIV
	buffer[1]=(unsigned char)sample_rate_divider;   //Divider
	write(i2c_port,buffer,2); 			//Write to i2C	
	
}

int IMUBPollRaw(float *data_dest) //Use  Poll method to sample the raw data
{

	static char ptr_buffer_before[14];
	unsigned char mpu_data_init=0x3B;
	static int firstRead=1;
	int different=0;
	int j;

	unsigned char ptr_buffer_current[256], *buffer_read;
	int i;
	int result,low,high;
	unsigned char ptr_buffer_mpu[256];
	int total_read=0;

	if(data_dest==NULL)
		return -1;	

	/*
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	ptr_buffer_current[0]=0x3B;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);
	result=read(i2c_port,ptr_buffer_mpu,14); 
	*/
	
	
	do{
		ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
		//ptr_buffer_current[0]=0x3B;                  //init register of the data
		result=write(i2c_port,&mpu_data_init,1);
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
				different=0;
				//printf("Equal Sample\n");
			}
			else
			{
				different=1;
				memcpy(ptr_buffer_before,ptr_buffer_mpu,14);
				//printf("different");
			}
		}
	}while(different!=1);
	
	
	//printf("total Read: %i \n",total_read++);

	//Accelerometer convertion
	for(i=0;i<3;i++)
	{
		low=ptr_buffer_mpu[i*2+1];
		high=ptr_buffer_mpu[i*2];
		data_dest[i+3]=((short)(((unsigned char)low) + ((unsigned char)(high))*256))*accel_scale/32767.0f;
	}

	//Gyroscope convertion
	for(i=4;i<7;i++)
	{
		low=ptr_buffer_mpu[i*2+1];
		high=ptr_buffer_mpu[i*2];
		data_dest[i+3]=((short)(((unsigned char)low) + ((unsigned char)(high))*256))*250/32767.0f;
	}

	ioctl(i2c_port,I2C_SLAVE,HMC_SLAVE_ADDR);
	ptr_buffer_current[0]=0x03;                  //init register of the data
	result=write(i2c_port,ptr_buffer_current,1);
	result=read(i2c_port,ptr_buffer_current,6);

	//Magnetometer Convertion
	buffer_read=ptr_buffer_current;
	for(i=0;i<3;i++)
	{
		low=buffer_read[i*2+1];
		high=buffer_read[i*2];
		data_dest[i]=(short)(((unsigned char)low) + ((unsigned char)(high))*256);
	}

	return 0;

}

int IMUBAccelScale(int g_scale)
{
	unsigned char byte_out[2];
	unsigned char accel_config_add=0x1C;
	int result;
	unsigned char code;


	//Checks ACCEL_CONFIG (0x1C register for more info)
	switch(g_scale)
	{
		case 2:
			byte_out[1]=0x00;
			break;
		case 4:
			byte_out[1]=0x08;
			break;
		case 8:
			byte_out[1]=0x10;
			break;
		case 16:
			byte_out[1]=0x18;
			break;
		default:
		 	printf("Unknown scale\n");
			return -1;
	}

	accel_scale=g_scale;
	ioctl(i2c_port,I2C_SLAVE,MPU_SLAVE_ADDR);
	byte_out[0]=0x1C;  //G Scale register address
	result=write(i2c_port,byte_out,2);
	return 0;
}

int IMUBPOllAngles(float *yaw, float *pitch, float *roll) //Use poll method to sample the processed angles
{

}

int IMUBClose() //close all resources
{

}
