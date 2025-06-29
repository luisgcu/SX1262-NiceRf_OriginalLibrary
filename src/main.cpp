#include <Arduino.h>

/********************************************************************************************
*	 App demo for SX1262 with ESP32 and PlatformIO
*	 This demo is used to test the SX1262 module with ESP32.
*	@version 1.0  
*********************************************************************************************	
Caution: 
	Make sure the power supply is UNDER 3.3V. Or the module will be destory!!
	
Usage:
	1. Select your ESP32 board configuration below (ESP32DEV_BOARD or ESP32WROOM_DEV_BOARD)
	2. Select your frequency (434.5 MHz or 915 MHz) based on your region
	3. Connect the LoRa1262 module and ESP32 Dev module as per pin configuration below
	4. Compile the code for master and slave respectively (uncomment #define SLAVE for slave)

ESP32 Dev Board Pin Configuration (ESP32DEV_BOARD):
		  ESP32		                    LoRa1262
		   GPIO5(SS)	      <------->    NSS
		   GPIO18(SCK)        <------->    SCK
		   GPIO23(MOSI)       <------->    MOSI
		   GPIO19(MISO)       <------->    MISO
		   GPIO16   	      <------->    NRESET
		   GPIO21   	      <------->    BUSY
		   GPIO17   	      <------->    DIO1
		   3V3   	          <------->    VCC
		   GND   	          <------->    GND

ESP32-WROVER-B Dev Board Pin Configuration (ESP32WROOM_DEV_BOARD):
		  ESP32		                    LoRa1262
		   GPIO5(SS)	      <------->    NSS
		   GPIO18(SCK)        <------->    SCK
		   GPIO23(MOSI)       <------->    MOSI
		   GPIO19(MISO)       <------->    MISO
		   GPIO4   	          <------->    NRESET
		   GPIO12   	      <------->    BUSY
		   GPIO14   	      <------->    DIO1
		   3V3   	          <------->    VCC
		   GND   	          <------->    GND
	   
*********************************************************************************************/

#include <SX1262.h>
#include <SPI.h>

// ============================================================================
// FREQUENCY SELECTION - Choose one based on your region
// ============================================================================
//#define FREQUENCY 434500000 // 434.5MHz - Europe/Asia ISM band
#define FREQUENCY 915000000   // 915MHz - Americas ISM band
#define POWER 10 // -9~22 dBm

// ============================================================================
// BOARD CONFIGURATION - Choose one ESP32 board type
// ============================================================================
//#define ESP32DEV_BOARD      // ESP32 Dev Board (GPIO16, GPIO21, GPIO17)
#define ESP32WROOM_DEV_BOARD  // ESP32-WROVER-B Dev Board (GPIO4, GPIO12, GPIO14)

#ifdef ESP32DEV_BOARD
// ============================================================================
// ESP32 Dev Board Pin Configuration
// ============================================================================
#define NSS_PIN 5      // Chip Select (CS/SS)
#define NRESET_PIN 16  // Reset pin
#define RFBUSY_PIN 21  // Busy pin
#define DIO1_PIN 17    // DIO1 interrupt pin

#else
// ============================================================================
// ESP32-WROVER-B Dev Board Pin Configuration (Default)
// ============================================================================
#define NSS_PIN 5      // Chip Select (CS/SS)
#define NRESET_PIN 4   // Reset pin
#define RFBUSY_PIN 12  // Busy pin
#define DIO1_PIN 14    // DIO1 interrupt pin

#endif
// Note: ESP32 hardware SPI pins are used automatically:
// SCK = 18, MISO = 19, MOSI = 23

/* if compile the code for slave, uncomment the below line*/
//#define		SLAVE

// Initialize SX1262 with ESP32 pins
SX1262 LoRa1262(NSS_PIN, NRESET_PIN, RFBUSY_PIN, DIO1_PIN);
loRa_Para_t	lora_para;	//define a struct to hold lora parameters.

int8_t power_table[32]={-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};
uint8_t sf_table[8]={LORA_SF5,LORA_SF6,LORA_SF7,LORA_SF8,LORA_SF9,LORA_SF10,LORA_SF11,LORA_SF12};

uint8_t bw_table[10]={LORA_BW_7,LORA_BW_10,LORA_BW_15,LORA_BW_20,LORA_BW_31,
LORA_BW_41,LORA_BW_62,LORA_BW_125,LORA_BW_250,LORA_BW_500};

uint8_t cr_table[4]={LORA_CR_4_5,LORA_CR_4_6,LORA_CR_4_7,LORA_CR_4_8};

uint8_t tx_buf[]={"www.nicerf.com"};
uint16_t tx_cnt = 0;

uint8_t rx_buf[20]={"www.nicerf.com"};
uint16_t rx_cnt = 0;
uint16_t rx_size = 0;

uint8_t val;
uint8_t state;


void setup(void) 
{
	bool temp;
	Serial.begin(115200);	//UART Init
	delay(1000); // Give time for serial to initialize
	
	Serial.println("Starting  NiceRF SX1262  initialization with NiceRf Sample library...");

	lora_para.rf_freq    = FREQUENCY;
	lora_para.tx_power   = POWER;	//-9~22
	lora_para.lora_sf    = LORA_SF8;
	lora_para.band_width = LORA_BW_125;
	lora_para.code_rate  = LORA_CR_4_5;
	lora_para.payload_size = sizeof(tx_buf);

	Serial.println("Calling LoRa1262.Init()...");
	temp = LoRa1262.Init(&lora_para);
	
	if(0 == temp)
	{
		Serial.println("Init fail!");
	}
	else
	{
		Serial.println("Init success!");
	}
	
	#ifdef SLAVE
		LoRa1262.RxBufferInit(rx_buf,&rx_size);
                LoRa1262.RxInit();    // wait for packet from master
		Serial.println("SX1262 demo slave!");
	#else
		Serial.println("SX1262 demo master!");
		Serial.print("Frequency: ");
		Serial.print(lora_para.rf_freq);
		Serial.println(" Hz");
		Serial.print("TX Power: ");
		Serial.print(lora_para.tx_power);
		Serial.println(" dBm");
	#endif
}

void loop(void) 
{
#ifdef SLAVE
        state = LoRa1262.WaitForIRQ_RxDone();
        if(state)    // wait for RxDone interrupt
        {
	    rx_cnt++;
	    Serial.print("rx_cnt = ");
            Serial.print(rx_cnt);
            Serial.print(" data:");
            Serial.write(rx_buf,rx_size);    // print out the receive data
            Serial.println();
        }
#else
	// Transmit packet every second (removed unnecessary Serial.read())
	LoRa1262.TxPacket(tx_buf,sizeof(tx_buf));
	state = LoRa1262.WaitForIRQ_TxDone();
	if(state)
	{
	  tx_cnt++;
	  Serial.print("tx_cnt = ");
          Serial.println(tx_cnt);
	}
        delay(10000);
#endif
}












