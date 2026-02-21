//************************************************
//  FILE        :timer_interrupt_latency_test_2.ino
//  DATE        :2026/02/16
//  DESCRIPTION :GPT irq latency in several sources
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************
/*
このプログラムは、タイマーによる3種類のISRのレスポンスを記録して表示します。
オーバーフロー、コンペアマッチA、コンペアマッチBの割り込みは、FspTimerライブラリによって構成されます。
レスポンス時間は、割り込みが要求された瞬間からISRがカウンターを読み取る瞬間までのGPTタイマーの
カウントアップ数として測定されます。

This program records and displays the latency of three types of timer ISRs. 
Overflow, Compare-match A and Compare-match B interrupts are configured by the FspTimer library. 
Latency is measured as the number of GPT timer count-ups from the moment the interrupt is requested 
to the moment its ISR reads the counter.
*/

//// Pin connection
//  - not required
//

//// Includes
#include <Arduino.h>
#include <FspTimer.h>

//// Grobals
FspTimer timer;
uint32_t  $TEST0;
uint32_t  $TEST1;
uint32_t  $TEST2;
uint32_t  $TEST3;
uint32_t  $TEST4;
uint32_t  $TEST5;
uint8_t   ledState = 0;
uint8_t   printOut = 0;

//// ISR tasks every 1 sec set by FspTimer library
void onCallback(timer_callback_args_t *args) {
  if (args->event == TIMER_EVENT_CYCLE_END) {       //corresponding to setup_overflow_irq()
    $TEST0 = R_GPT0->GTCNT;
    $TEST1 = R_GPT0->GTCNT;
  }
  else if (args->event == TIMER_EVENT_CAPTURE_A) {
    $TEST2 = R_GPT0->GTCNT;
    $TEST3 = R_GPT0->GTCNT;
  }
  else if (args->event == TIMER_EVENT_CAPTURE_B) {
    $TEST4 = R_GPT0->GTCNT;
    $TEST5 = R_GPT0->GTCNT;
  }
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
  timer.begin(TIMER_MODE_PWM, GPT_TIMER, 0, 48000000, 1000, TIMER_SOURCE_DIV_1, onCallback, nullptr);     //Set GPT0 period to 1 sec (1*48e6/1)
  timer.setup_overflow_irq(2, nullptr);
  timer.setup_capture_a_irq(3, nullptr);
  timer.setup_capture_b_irq(4, nullptr);
  timer.open();
  timer.start();
  timer.set_duty_cycle(481, CHANNEL_A);             //Trigger after 10 microsecond from overflow
  timer.set_duty_cycle(961, CHANNEL_B);             //Trigger after 20 microsecond from overflow
  interrupts();
  check_nvic();                                     //Dump setting of interrupts
  delay(1100);                                      //wait for GTCCR buffer propagation
  Serial.print("R_GPT0->GTCCRA:  "); Serial.println(R_GPT0->GTCCR[0]);
  Serial.print("R_GPT0->GTCCRB:  "); Serial.println(R_GPT0->GTCCR[1]);
  Serial.println();
  while ($TEST5 == 0) {};
  $TEST5 = 0;
}

//// Loop function
void loop() {
  if ($TEST5 > 0) {
    if (printOut++ < 10) {
      Serial.print("FspTimer Overflow ISR:        "); Serial.print($TEST0); Serial.print(" / "); Serial.println($TEST1);
      Serial.print("FspTimer Compare-match A ISR: "); Serial.print($TEST2 - 480); Serial.print(" / "); Serial.println($TEST3 - 480);
      Serial.print("FspTimer Compare-match B ISR: "); Serial.print($TEST4 - 960); Serial.print(" / "); Serial.println($TEST5 - 960);
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
