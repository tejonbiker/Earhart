/*
    C socket server example, handles multiple clients using threads
    Compile
    gcc server.c -lpthread -o server

    https://gist.github.com/silv3rm00n/5821760
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread

#include "imuboard.h"
#include "math.h"
#include "pwm.h"
#include "stdlib.h"

 
//the thread function
void *connection_handler(void *);
void *control_handler(void *);

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


 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c;
    struct sockaddr_in server , client;
    pthread_t control_thread_id;
    FILE *log=NULL;
    int i,j;

    if(IMUBInit()<0){
	printf("Error with IMU appear\n");
	exit(0);
    }
	IMUB_DLPF(6);
	IMUBAccelScale(16);

	if(argc>=2)
	{
		log=fopen(argv[1],"w");
		if(log==NULL)
		{
			printf("Error to open user file: %s\n",argv[1]);
			return -1;
		}
	}
	else
	{
		printf("Using default name: IMU_log.txt\n");
		log=fopen("IMU_log.txt","w");
	}
	
	#ifdef MOTORS
	//Setup PWM
	setup(1,DELAY_VIA_PWM);
	init_channel(channel, SUBCYCLE_TIME_US_DEFAULT);
	
	for(i=0;i<6;i++)
		add_channel_pulse(channel, esc[i], 0, 1000);	
	#endif

	usleep(5000000);


    	if( pthread_create( &control_thread_id , NULL ,  control_handler , (void*)NULL) < 0)
    	{
       		perror("could not create control thread");
       		return 1;
    	}
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	
    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
     
    //Send some messages to the client
    message = "Greetings! I am your connection handler\n";
    write(sock , message , strlen(message));
     
    message = "Now type something and i shall repeat what you type \n";
    write(sock , message , strlen(message));
     
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //end of string marker
		client_message[read_size] = '\0';
		
		//Send the message back to client
        write(sock , client_message , strlen(client_message));
		
		//clear the message buffer
		memset(client_message, 0, 2000);
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    return 0;
}

/*
	This will handle the control loop
*/ 

void *control_handler(void *socket_desc)
{

	

	int i,mid_seconds;
	
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
			
			for(i=0;i<6;i++)
				printf("%f ",omega[i]);

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

			  //Yaw segment           Pitch		    Roll
		omega[0]=  errors[0]*Kyaw    +   errors[1]*Kpitch                         + WBase;
		omega[1]=  -errors[0]*Kyaw   +   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[2]=  errors[0]*Kyaw    -   errors[1]*Kpitch    + errors[2]*Kroll    + WBase;
		omega[3]= -errors[0]*Kyaw    -   errors[1]*Kpitch    			  + WBase;
		omega[4]=  errors[0]*Kyaw    -   errors[1]*Kpitch    - errors[2]*Kroll 	  + WBase;
		omega[5]= -errors[0]*Kyaw    +   errors[1]*Kpitch    - errors[2]*Kroll    + WBase;


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

         
    return 0;
}