//************************************************
//  FILE        :agt_setup_test.ino
//  DATE        :2026/03/05
//  DESCRIPTION :AGT timer period measurement
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************
/*
このプログラムは、GPTタイマーのインプットキャプチャ機能を使ってAGTタイマーの周期を計測して表示します。
AGTタイマーがアンダーフローする毎にGPTタイマーへのイベントを発生させ、
イベントリンクによってGPTタイマーのインプットキャプチャをトリガーします。
そのため、現在のキャプチャ値とその前のキャプチャ値の差分がGPTタイマーで数えたAGTタイマーの周期の時間となります。

AGTタイマーのアンダーフローからGPTタイマーのキャプチャまでの伝達遅れは一定と仮定しています。

This program measures and displays the period of the AGT timer using the input‑capture function of the GPT timer.
Each time the AGT timer underflows, it generates an event that is routed to the GPT timer
through the Event Link Controller (ELC), and this event triggers input‑capture of the GPT.
Therefore, the difference between the current captured value and the previous one represents the period of the AGT timer in GPT timer counts.
In this example, the AGT period is set to 8 ms, which corresponds to 384,000 counts when the GPT timer runs at a 48 MHz clock.
It is assumed that the propagation delay from the AGT underflow event to the GPT capture trigger is constant.
*/

//// Pin connection
//  - not required
//

//// Includes
#include <Arduino.h>
#include <FspTimer.h>

//// Grobals
FspTimer timerGPT;
FspTimer timerAGT;
uint32_t capt_period = 0;                                           //equivarent to 0x0100000000
uint32_t capt_count  = 0;
uint32_t capt_value;
volatile uint32_t delta_value;
volatile bool     capt_flag;

//// ISR tasks every event occured
void onCallback(timer_callback_args_t *args) {
  if (args->event == TIMER_EVENT_CAPTURE_A) {
    delta_value = args->capture - capt_value;
    capt_value = args->capture;
    capt_flag  = true;
  }
}

//// Setup function
void setup() {
//Setup serial port
  Serial.begin(115200);
  delay(2000);
//AGT configure, base clock 24 MHz 
  timerAGT.begin(TIMER_MODE_PERIODIC, AGT_TIMER, 1, 24000, 1, (timer_source_div_t)TIMER_SOURCE_DIV_8);
  timerAGT.open();                                                  //AGT timer start automatically
//ELC configure
  R_MSTP->MSTPCRC_b.MSTPC14 = 0;
  R_ELC->ELSR[0].HA_b.ELS = ELC_EVENT_AGT1_INT;
  R_ELC->ELCR_b.ELCON = 1;
//GPT configure
  timerGPT.begin(TIMER_MODE_PWM, GPT_TIMER, 0, capt_period, capt_count, TIMER_SOURCE_DIV_1, onCallback);
  timerGPT.set_source_capture_a((gpt_source_t)GPT_SOURCE_GPT_A);    //performed on ELC GPTA event
  timerGPT.setup_capture_a_irq();
  timerGPT.open();
  timerGPT.start();
//show ELC settings
  Serial.print("R_ELC->ELSR[0].HA_b.ELS:  "); Serial.println(R_ELC->ELSR[0].HA_b.ELS, HEX);
  Serial.print("R_GPT0->GTICASR:  "); Serial.println(R_GPT0->GTICASR, HEX);
}

//// Loop function
void loop() {
  Serial.print("delta count: "); Serial.println(delta_value);
  delay(1000);                                                      //show recent result
}


