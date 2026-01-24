//************************************************
//  FILE        :timer_interrupt_latency_test.ino
//  DATE        :2026/01/15
//  DESCRIPTION :GPT irq latency in several conditions
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************
/*
このプログラムは、タイマーによる3種類のISRのレスポンスを記録して表示します。
1つめのISRはFspTimerライブラリによって構成され、他の2つは自作ライブラリによって構成されます。
レスポンス時間は、割り込みが要求された瞬間からISRがカウンターを読み取る瞬間までのGPTタイマーの
カウントアップ数として測定されます。

This program records and displays the latency of three types of timer ISRs. 
One ISR is configured by the FspTimer library, and the other two are configured by a custom library. 
Latency is measured as the number of GPT timer count-ups from the moment the interrupt is requested 
to the moment its ISR reads the counter.
*/

//// Pin connection
//  - not required
//

//// Includes
#include <Arduino.h>
#include <FspTimer.h>
#include <GPT_basicfunction.h>

//// Grobals
FspTimer timer;
GPTFunction  myGpt;
uint32_t  $TEST0;
uint32_t  $TEST1;
uint32_t  $TEST2;
uint32_t  $TEST3;
uint32_t  $TEST4;
uint32_t  $TEST5;
uint8_t   ielstable[192];               //event number to IELSRn conversion table
uint8_t   ledState = 0;
uint8_t   printOut = 0;

//// ISR overflow task every 1 sec set by FspTimer library
void onOverflow(timer_callback_args_t *args) {
  $TEST0 = R_GPT0->GTCNT;
  $TEST1 = R_GPT0->GTCNT;
}

//// ISR comparematch-A task every 1 sec set by custom library
void onCompareA() {
  $TEST2 = R_GPT1->GTCNT;
  myGpt.clearInterruptFlag();
  $TEST3 = R_GPT1->GTCNT;
}

//// ISR comparematch-B task every 1 sec set by custom library
void onCompareB() {
  $TEST4 = R_GPT1->GTCNT;
  R_ICU->IELSR_b[ielstable[ELC_EVENT_GPT1_CAPTURE_COMPARE_B]].IR = 0;
  $TEST5 = R_GPT1->GTCNT;
}


//// Setup function
void setup() {
//Setup general port
  Serial.begin(115200);
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
//Setup GPT by Fsp library
  noInterrupts();
  timer.begin(TIMER_MODE_PWM, GPT_TIMER, 0, 48000000, 1000, TIMER_SOURCE_DIV_1, onOverflow, nullptr);   //Set GPT0 period to 1 sec (1*48e6/1)
  timer.setup_overflow_irq();
  timer.open();
//Setup GPT by custom library
  myGpt.begin(1, 48000000, TIMER_SOURCE_DIV_1);   //Set GPT1 period to 1 sec (1*48e6/1)
  myGpt.setCallback(COMPARE_A, 2, onCompareA);
  myGpt.setCallback(COMPARE_B, 2, onCompareB);
  R_GPT1->GTCCR[GTCCRA] = 480;                    //Trigger after 10 microsecond from overflow
  R_GPT1->GTCCR[GTCCRB] = 960;                    //Trigger after 20 microsecond from overflow
  R_GPT1->GTBER = 0x00000003;                     //Does not use buffer mode 
  for (int i=0;i<32;i++) {
    ielstable[R_ICU->IELSR_b[i].IELS] = i;        //Make event number to IELSRn conversion table
  }
  R_GPT0->GTCNT = 0;
  R_GPT1->GTCNT = 0;
  R_GPT0->GTSTR = 0x03;                           //Start GPT0 and GPT1
  interrupts();
  check_nvic();                                   //Dump setting of interrupts
  while ($TEST5 == 0) {};
  $TEST5 = 0;
}


//// Loop function
void loop() {
  if ($TEST5 > 0) {
    if (printOut++ < 10) {
      Serial.print("FspTimer set and return:      "); Serial.print($TEST0); Serial.print(" / "); Serial.println($TEST1);
      Serial.print("Custom set and Bsp return:    "); Serial.print($TEST2 - 480); Serial.print(" / "); Serial.println($TEST3 - 480);
      Serial.print("Custom set and table return:  "); Serial.print($TEST4 - 960); Serial.print(" / "); Serial.println($TEST5 - 960);
      Serial.println();
    }
    else {printOut = 10;}
    $TEST0 = 0; $TEST1 = 0; $TEST2 = 0; $TEST3 = 0; $TEST4 = 0; $TEST5 = 0; 
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);
  }
  delayMicroseconds(10);

}   //End of loop()


//Dump interrupt related register settings
void  check_nvic(void) {
  uint32_t* vp;
  for(int i=0;i<10;i++) {
    uint8_t IELSR = (uint8_t)(R_ICU->IELSR_b[i].IELS);
    uint8_t NVICP = NVIC->IP[i] >> 4;
    vp = (uint32_t*)(SCB->VTOR + i * 4 + 0x0040);
    uint32_t IVECT = *vp;
    Serial.print("Event: "); Serial.print(IELSR, HEX);
    Serial.print("    Priority: "); Serial.print(NVICP, HEX);
    Serial.print("    Vector Addr: "); Serial.println(IVECT, HEX);
  }
  Serial.println();
}


