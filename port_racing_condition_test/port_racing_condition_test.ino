//************************************************
//  FILE        :port_racing_condition_test.ino
//  DATE        :2026/01/09
//  DESCRIPTION :simultaneously port access by loop and ISR
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************

/*
loop()のdigitalWrite(D0, HIGH/LOW)はピンD0を更新し、GPT0コンペアマッチBのISRはピンD1をトグルします。
 - D0ピンの立ち上がりパルスによってGPT0がクリアされ、またGPT0コンペアマッチBのISRはパルスの数クロック後にトリガーされます。
 - 隣接するピンのdigitalWrite()と競合するよう、GPT0コンペアマッチBは割り込み毎に1クロックずらされます。
 - MCUクロック速度を1/8 (6 MHz) に下げて、オシロスコープでの観察を容易にしています。
 
digitalWrite(D0, HIGH/LOW) in loop() updates pin D0, and GPT0 Compare Match B ISR toggles pin D1.
 - GPT0 is cleared by rising pulse of pin D0 and GPT0 Compare Match B ISR will be triggered after several clocks.
 - GPT0 compare match B is shifted by 1 clock each cycle to force a collision with digitalWrite() on adjacent pins.
 - MCU clock speed is reduced to 1/8 (6 MHz) for easier observation with oscilloscope.
*/

//// Pin connection
//  - D0ピンとD7ピンを接続する
//  - D0ピン(P301) → オシロスコープ CH1
//  - D1ピン(P302) → オシロスコープ CH2
//  - Connect pin D0 and D7(P107)
//  - Connect pin D0(P301) to CH1 probe of oscilloscope
//  - Connect pin D1(P302) to CH2 probe of oscilloscope


//// Includes
#include <Arduino.h>
#include <GPT_basicfunction.h>                    //Custom library install is necessary

//// Definitions
#define     __NOP8            asm volatile ( \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                "nop   \n" \
                                )

//// Grobals
GPTFunction myGpt;
uint32_t    lag    = 0;
uint32_t    lagOfs = 8;

//// ISR compare-match B
void onCompareB() {
  R_PFS->PORT[3].PIN[2].PmnPFS = 0x05;              //Set D1=HIGH
  R_GPT0->GTCCR[GTCCRB] = (lag++ & 0x1F) + lagOfs;  //Shift compare-match B by 1 clock (as vernier sweep)
  __NOP8;
  R_PFS->PORT[3].PIN[2].PmnPFS = 0x04;              //Set D1=LOW
  myGpt.clearInterruptFlag(); 
}

//// Setup function
void setup() {
//Setup general port
  pinMode(D0, OUTPUT);                            //Set D0(P301) to output
  digitalWrite(D0, LOW);
  R_PFS->PORT[3].PIN[2].PmnPFS= 0x04;             //Set D1(P302) to output
//Down-clocking
  R_SYSTEM->PRCR = 0xA501;                        //Register access enable
  R_SYSTEM->SCKDIVCR = 0x43043433;                //1/8 of default
//Setup GPT by custom library
  noInterrupts();
  myGpt.begin(0, 2000, TIMER_SOURCE_DIV_1);       //Set GPT0 period (some large value)
  myGpt.setCallback(COMPARE_B, 2, onCompareB);    //Set compare-match B ISR
  R_GPT0->GTBER = 0x00000003;                     //Does not use buffer mode
  R_GPT0->GTCSR = 0x0300;                         //Set GPT0 clear polarity to rising edge of pulse
  R_GPT0->GTUDDTYC = 1;                           //Set GPT0 to count-up
  R_GPT0->GTIOR = 0x010F0000;                     //Set output of compare-match B to indicate ISR timing (option)
  R_PFS->PORT[1].PIN[6].PmnPFS = 0x03010400;      //D6(P106, GPT0_B) set to compare-match output to indicate ISR trigger timing (option)
  R_PFS->PORT[1].PIN[7].PmnPFS = 0x03010000;      //D7(P107, GPT0_A) set to capture input for external Irq signal
  R_GPT0->GTCCR[GTCCRB] = lagOfs;                 //Set initial value of GTCCRB
  R_GPT0->GTCNT = 0;                              //Clear GPT0
  R_GPT0->GTSTR = 0x01;                           //Start GPT0
  interrupts();
}

//// Loop function
void loop() {
  digitalWrite(D0, HIGH);
  digitalWrite(D0, LOW);
}



