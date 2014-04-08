/*
	A short library to manage the connections and calculus of MPU6050
	and HMC5883L intertial sensors
*/

#ifndef _IMUBOARD_
#define _IMUBOARD_

#define IMUB_MPU 1
#define IMUB_HMC 2

#define MPU_SLAVE_ADDR 0x68
#define HMC_SLAVE_ADDR 0x1E

#define IMUB_SR_1000 0
#define IMUB_SR_512  1
#define IMUB_SR_111  9

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>

int IMUBInit();	//Open the I2C Channel and verify the sensors
int IMUBSampleRate(int sample_rate); //Setup the sample rate
int IMUBPollRaw(float *data_dest); //Use  Poll method to sample the raw data
int IMUBPOllAngles(float *yaw, float *pitch, float *roll); //Use poll method to sample the processed angles
int IMUBClose(); //close all resources

#endif