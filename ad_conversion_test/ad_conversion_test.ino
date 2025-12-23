//************************************************
//  FILE        :ad_conversion_test.ino
//  DATE        :2025/12/20
//  DESCRIPTION :adc by different sample time
//  BOARD TYPE  :UNO R4 MINIMA
//  AUTHER      :inteGN
//************************************************

/*
AD変換のサンプル時間を変化させて結果を出力する
 - 最初にデフォルトの条件でanalogRead()を実行し、結果をシリアルモニタに出力する
 - AD変換器のサンプル時間を変えながら、A1ピン、A2ピンの順で変換する
 - 同様に、サンプル時間を変えながら、A4ピン、A5ピンの順で変換する
 - この変換中、変換結果をシリアルモニタに出力する

AD convert with sweeping sample time and output that result.
 - First, execute analogRead() in default condition and output the results to serial monitor
 - convert pin A1 and pin A2 while sweeping sample time.
 - In the same way, convert pin A4 and pin A5 while sweeping sample time.
 - Output conversion result to serial monitor in each time of above.
*/

//// Pin connection
//  - サンプルホールドコンデンサの電圧の初期化のため、+5VまたはGNDをA1ピン、A4ピンに接続する
//  - 2本の10kohmを直列にし、両端を5VとGNDへ、中点をA2、A5ピンに接続する
//  - +5V or GND apply to pin A1 and A4 to initialise sample-hold capacitor voltage
//  - Connect two 10kohm resisistor in serial, connect one end to +5V and another end to GND and connect those midpoint to pin A2 and A5


//// Grobals
uint16_t  adResult0;
uint16_t  adResult1;


//// Setup function
void setup() {
//Setup general interface
  Serial.begin(115200);
  delay(2000);
  pinMode(LED_BUILTIN, OUTPUT);

//AD read in default condition
  Serial.print("analogRead(A1)  : "); Serial.println(analogRead(A1));
  Serial.print("analogRead(A2)  : "); Serial.println(analogRead(A2));
  Serial.print("ADSSTR[1] value : "); Serial.println(R_ADC0->ADSSTR[1]);  //of Arduino A2 pin (RA4M1 P001/AN01)
  Serial.print("analogRead(A4)  : "); Serial.println(analogRead(A4));
  Serial.print("analogRead(A5)  : "); Serial.println(analogRead(A5));
  Serial.print("ADSSTRL value   : "); Serial.println(R_ADC0->ADSSTRL);    //of Arduino A5 pin (RA4M1 P100/AN22)

//Setup AD converter input by directly resistor access
  R_ADC0->ADCSR = 0x0000;                           //single-scan, high-speed conversion, AD stop
  R_ADC0->ADCER = 0x0006;                           //flush-right 14bit
  R_PFS->PORT[0].PIN[0].PmnPFS  = 0x8000;           //Arduino A1 pin (RA4M1 P000/AN00) **High-precision channel
  R_PFS->PORT[0].PIN[1].PmnPFS  = 0x8000;           //Arduino A2 pin (RA4M1 P001/AN01) **High-precision channel
  R_PFS->PORT[1].PIN[0].PmnPFS  = 0x8000;           //Arduino A5 pin (RA4M1 P100/AN22) **Normal-precision channel
  R_PFS->PORT[1].PIN[1].PmnPFS  = 0x8000;           //Arduino A4 pin (RA4M1 P101/AN21) **Normal-precision channel

//Output AD converter clock condition
  Serial.print("PCLKC=ADCLK : "); Serial.println(R_SYSTEM->SCKDIVCR_b.PCKC); Serial.println();

//AD conversion of high-precision channel
  for (int i=0;i<10;i++) {
    R_ADC0->ADSSTR[0] = 255;
    R_ADC0->ADANSA[0]  = 0x0001;              //A1 activate, A2 off
    R_ADC0->ADANSA[1]  = 0x0000;              //A4, A5 off
    R_ADC0->ADCSR_b.ADST = 1;                 //AD start
    while (R_ADC0->ADCSR_b.ADST == 1) {}
    adResult0 = R_ADC0->ADDR[0];
    R_ADC0->ADSSTR[1] = i * 10 + 13;          //sweep and set sample time
    R_ADC0->ADANSA[0]  = 0x0002;              //A1 off, A2 activate
    R_ADC0->ADANSA[1]  = 0x0000;              //A4, A5 off
    R_ADC0->ADCSR_b.ADST = 1;                 //AD start
    while (R_ADC0->ADCSR_b.ADST == 1) {}
    adResult1 = R_ADC0->ADDR[1];
    Serial.print("  A1: "); Serial.print(adResult0); Serial.print("      A2: "); Serial.print(adResult1); Serial.print(" / ADSSTR: "); Serial.println(R_ADC0->ADSSTR[1]);
    delay(10);
  }
  Serial.println();

//AD conversion of normal-precision channel
  for (int i=0;i<10;i++) {
    R_ADC0->ADSSTRL = 255;
    R_ADC0->ADANSA[0]  = 0x0000;              //A1, A2 off
    R_ADC0->ADANSA[1]  = 0x0020;              //A4 active, A5 off
    R_ADC0->ADCSR_b.ADST = 1;                 //AD start
    while (R_ADC0->ADCSR_b.ADST == 1) {}
    adResult0 = R_ADC0->ADDR[21];
    R_ADC0->ADSSTRL = i * 10 + 13;            //sweep and set sample time
    R_ADC0->ADANSA[0]  = 0x0000;              //A1, A2 off
    R_ADC0->ADANSA[1]  = 0x0040;              //A4 off, A5 active
    R_ADC0->ADCSR_b.ADST = 1;                 //AD start
    while (R_ADC0->ADCSR_b.ADST == 1) {}
    adResult1 = R_ADC0->ADDR[22];
    Serial.print("  A4: "); Serial.print(adResult0); Serial.print("      A5: "); Serial.print(adResult1); Serial.print(" / ADSSTR: "); Serial.println(R_ADC0->ADSSTRL);
    delay(10);
  }
  Serial.println(); Serial.println("finished."); Serial.println();
}


//// Loop function
void loop() {
  //empty
  digitalWrite(LED_BUILTIN, 0);
  delay(1000);
  digitalWrite(LED_BUILTIN, 1);
  delay(1000);
  delayMicroseconds(10);

}   //End of loop()


