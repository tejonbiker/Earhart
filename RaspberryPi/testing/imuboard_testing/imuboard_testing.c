#include "imuboard.h"

float raw[24];

int main(void)
{
	int i;
	IMUBInit();

	IMUBPollRaw(raw);

	for(i=0;i<10;i++)
	{
		printf("%g, ",raw[i]);
	}
	
	printf("\n");

	return 0;
}