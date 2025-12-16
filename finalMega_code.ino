/*************************************************
 Arduino Mega – ADC Expander
 Sends: M,p3,f3,p4,f4,p5,f5
*************************************************/

  /*=== SPARK For Innovation & Creativity ===*/

void setup() {
  Serial1.begin(115200); // TX1=18 RX1=19
}

void loop() {
  int p3 = analogRead(A0);
  int f3 = analogRead(A1);
  int p4 = analogRead(A2);
  int f4 = analogRead(A3);
  int p5 = analogRead(A4);
  int f5 = analogRead(A5);

  Serial1.print("M,");
  Serial1.print(p3); Serial1.print(",");
  Serial1.print(f3); Serial1.print(",");
  Serial1.print(p4); Serial1.print(",");
  Serial1.print(f4); Serial1.print(",");
  Serial1.print(p5); Serial1.println(",");

  delay(100);
}
