#include <nrk.h>
#include <include.h>
#include <ulib.h>
#include <stdio.h>
#include <avr/sleep.h>
#include <hal.h>
#include <rt_link.h>
#include <nrk_error.h>
#include <nrk_events.h>
#include <nrk_driver.h>
#include <nrk_driver_list.h>
#include <ff_basic_sensor.h>
#include <stdlib.h>

//constants related to channel & slots
#define MY_CHANNEL 14
#define NODE_ID  1
#define MY_TX_SLOT (NODE_ID*2)
#define NODE_2_RX_SLOT  4
#define NODE_3_RX_SLOT  6

//constants related to time
#define EXCHANGE_INTERVAL 5

//everything related to the TX & RX process
uint8_t tx_buf[RF_MAX_PAYLOAD_SIZE];
uint8_t rx_buf[RF_MAX_PAYLOAD_SIZE];

//task related NRK stuff
NRK_STK Stack1[NRK_APP_STACKSIZE];
nrk_task_type TaskOne;
void Task1(void);
void nrk_create_taskset();

//time related variables
nrk_time_t exchangeTime;
nrk_time_t currentTime;

uint8_t myX=0, myY=0;
uint8_t X[3]={0,0,0}, Y[3]={0,0,0};

int main ()
{
	nrk_setup_ports();
	nrk_setup_uart(UART_BAUDRATE_115K2);
	nrk_kprintf( PSTR("Starting up...\r\n") );
	nrk_init();

	nrk_led_clr(0);
	nrk_led_clr(1);
	nrk_led_clr(2);
	nrk_led_clr(3);
  
	nrk_time_set(0,0);

	nrk_register_drivers();
	rtl_task_config();
	nrk_create_taskset ();

	nrk_start();
	
	return 0;
}


void Task1()
{
	int8_t rssi, slot,length; //all parameters recieved along with an rx

	printf( "Task1 PID=%d\r\n",nrk_get_pid());
	printf( "Node ID=%d\r\n",NODE_ID);

	nrk_led_set(RED_LED); 

	rtl_init (RTL_COORDINATOR);
	rtl_set_channel(MY_CHANNEL);

	rtl_set_schedule( RTL_RX, NODE_2_RX_SLOT, 1 );
	rtl_set_schedule( RTL_RX, NODE_3_RX_SLOT, 1 );
	rtl_set_schedule( RTL_TX, MY_TX_SLOT, 1 );
	//rtl_set_tx_power(MAX_POWER);
	rtl_rx_pkt_set_buffer(rx_buf, RF_MAX_PAYLOAD_SIZE); //to limit payload size
	rtl_start();
	
	while(!rtl_ready())  nrk_wait_until_next_period();  

	
	while(1)
	{
		
		if( rtl_tx_pkt_check(MY_TX_SLOT)!=0 )
		{
			//printf("Pending TX");
		}
		else
		{
                	nrk_time_get(&currentTime);
                	if(currentTime.secs-exchangeTime.secs>EXCHANGE_INTERVAL)
                	{
                		//TODO:Data to be transmitted
                		int8_t length;
				//sprintf( &tx_buf[PKT_DATA_START],"",);
				length = sprintf(&tx_buf[PKT_DATA_START], "%-+5d%-+5d",myX,myY); //TODO: Data
				printf ("(%s) is the result of our sprintf, which is %d characters long",tx_buf,length);
				length=length+PKT_DATA_START+1;
				printf("Sending: ");
/*				for(int i=PKT_DATA_START; i<length; i++ )*/
/*				{*/
/*					printf("%c",tx_buf[i] );*/
/*				}*/
/*				printf("\n\r");*/
				addToTXBuffer(tx_buf,length);
				nrk_time_get(&exchangeTime);
                	}
			fetchTxBuffer(); //Actual Transmit
               		nrk_led_toggle(BLUE_LED);
		}
		if( rtl_rx_pkt_check()!=0 )
		{
			int8_t senderNode;
			nrk_led_set(ORANGE_LED);
                   	uint8_t *local_rx_buf;
                   	local_rx_buf=rtl_rx_pkt_get(&length, &rssi, &slot);
                   	printf( "Got Packet on slot %d %d: ",slot,length );
                   	senderNode=slot/2;
                   	if(senderNode==0)
                   	{
                   		int li = atoi (&local_rx_buf[NODE_ID]);
				printf ("\nXrecd=%d",li);
				myX=li;
				li = atoi (&local_rx_buf[NODE_ID+5]);
				printf ("\nYrecd=%d",li);
				myY=li;
			}
                   	else
                   	{
                   		int li = atoi (&local_rx_buf[NODE_ID]);
				printf ("\nXrecd=%d",li);
				X[senderNode]=li;
				li = atoi (&local_rx_buf[NODE_ID+5]);
				printf ("\nYrecd=%d",li);
				Y[senderNode]=li;
                   	}
                   	
		}
		rtl_wait_until_rx_or_tx();
	}
}


//Add to Queue
void addToTXBuffer(uint8_t toTx[], int8_t length)
{
	//printf("adding to trasmit \n\r");
	for(int i=0; i<length; i++ )
	{
		txQueue[rear][i]=toTx[i] ;
		//printf("%c",txQueue[rear][i] );
	}
	entry[rear]=1;	
	rear++;
	if(rear==MAX_QUEUE_SIZE)
	{
		rear=0;
	}
}

//Dequeue oldest & transmit
void  fetchTxBuffer()
{
	int8_t length;
	if(entry[front]==1)
	{
		length=strlen(&txQueue[front][PKT_DATA_START])+PKT_DATA_START+1;
		//printf("transmitting from queue   \n\r");
		/*
		for(int i=PKT_DATA_START; i<length; i++ )
		{
			printf("%c",txQueue[front][i] );
		}
		printf("\n\r");*/
				
		rtl_tx_pkt( &txQueue[front], length, MY_TX_SLOT);
		entry[front]=0;
		*txQueue[front] = "";
		//printf("now the que is%s",txQueue[front]);
		front++;
		//rtl_wait_until_rx_or_tx();
	}
	else
	{
	//NOTHING TO TRANSMIT, BUT GATEWAY HAS TO TRANSMIT SOMETHING
	rtl_tx_pkt("type=0", strlen("type=0")+PKT_DATA_START+1, MY_TX_SLOT);
	}
	
	//wrap around
	if(front==MAX_QUEUE_SIZE)
	{
		front=0;
	}
}

/*PRE-EXISTING STUFF DO NOT TOUCH*/
void nrk_create_taskset()
{
	TaskOne.task = Task1;
	TaskOne.Ptos = (void *) &Stack1[NRK_APP_STACKSIZE-1];
	TaskOne.Pbos = (void *) &Stack1[0];
	TaskOne.prio = 2;
	TaskOne.FirstActivation = TRUE;
	TaskOne.Type = BASIC_TASK;
	TaskOne.SchType = PREEMPTIVE;
	TaskOne.period.secs = 1;
	TaskOne.period.nano_secs = 0;
	TaskOne.cpu_reserve.secs = 0;
	TaskOne.cpu_reserve.nano_secs = 0;
	TaskOne.cpu_reserve.nano_secs = 100*NANOS_PER_MS;
	TaskOne.offset.secs = 0;
	TaskOne.offset.nano_secs= 0;
	nrk_activate_task (&TaskOne);
	nrk_kprintf( PSTR("Create Done\r\n") );
}


void nrk_register_drivers()
{
	int8_t val;

	// Register the Basic FireFly Sensor device driver
	// Make sure to add: 
	//     #define NRK_MAX_DRIVER_CNT  
	//     in nrk_cfg.h
	// Make sure to add: 
	//     SRC += $(ROOT_DIR)/src/drivers/platform/$(PLATFORM_TYPE)/source/ff_basic_sensor.c
	//     in makefile
	val=nrk_register_driver( &dev_manager_ff_sensors,FIREFLY_SENSOR_BASIC);
	if(val==NRK_ERROR) nrk_kprintf( PSTR("Failed to load my ADC driver\r\n") );

}
