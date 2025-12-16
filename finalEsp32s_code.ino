/* ESP32 - Smart Water Distribution (Blynk + LCD 16x4 parallel 4-bit + HC-SR04)
   Parallel LCD (no I2C). Change LCD pin constants at top to match free GPIOs.
   Keeps Serial printing for debug, sends to Blynk, reads ADC pots, buttons, LEDs as valves and alarms.
*/

/* === SPARK For Innovation & Creativity === */


/* === Libraries === */
#include <Wire.h>
#include <LiquidCrystal.h>
#include <BlynkSimpleEsp32.h>

/* === Blynk & WiFi settings === */
char auth[] = "BLYNK_AUTH_TOKEN";
const char* ssid = "WIFI_SSID";
const char* pass = "WIFI_PASS";

/* ========== CONFIGURABLE PINS ========== */
const int PIN_MAIN_ADC = 34;
const int PIN_BRANCH_ADC[5] = {32, 33, 35, 36, 39};

/* Buttons */
const int PIN_BTN_APPARENT = 27;
const int PIN_BTN_REAL     = 14;

/* Valve LEDs (represent valve state) */
int PIN_VALVE_LED[5] = {2, 4, 16, 17, 5}; // you can reassign if conflicts

/* Alarm LEDs */
const int PIN_ALARM_INFO = 25;
const int PIN_ALARM_WARN = 26;
const int PIN_ALARM_CRIT = 13;

/* Ultrasonic HC-SR04 */
const int PIN_TRIG = 12;
const int PIN_ECHO = 15;

/* ---------- LCD 4-bit pin mapping---------- */
const int LCD_RS = 21;   // RS pin
const int LCD_EN = 22;   // EN pin
const int LCD_D4 = 23;   // D4
const int LCD_D5 = 19;   // D5
const int LCD_D6 = 18;   // D6
const int LCD_D7 = 5;    // D7

/* create LiquidCrystal instance for 4-bit mode */
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

/* ========== Calibration parameters ========== */
const float ADC_MAX = 4095.0f;
float MAIN_MAX_LPS = 0.2f;
float BRANCH_MAX_LPS = 0.05f;
float UARL_liters_per_day = 14.1f;
float UARL_Lps = 0.0f;

/* Simulated loss counters and thresholds */
int simulated_apparent_level = 0;
int simulated_real_level = 0;
const float WARN_MULT = 1.0f;
const float ACTION_MULT = 3.0f;

/* Telemetry timing */
unsigned long lastBlynkMs = 0;
const unsigned long BLYNK_INTERVAL_MS = 10000;
unsigned long lastLcdUpdate = 0;
const unsigned long LCD_UPDATE_MS = 1500;

/* Debounce */
unsigned long lastBtnAppMillis = 0;
unsigned long lastBtnRealMillis = 0;
const unsigned long DEBOUNCE_MS = 200;

/* Tank height in cm (adjust) */
const float TANK_HEIGHT_CM = 150.0f;

/* Helper prototypes */
float readADCasLps(int adcPin, float maxLps);
float readUltrasonicLevelPct();
void updateAlarmsAndAct(float measuredLossLpd, float branchLps[], float sumBranches);
void sendBlynkTelemetry(float mainLps, float sumBranches, float measuredLossLpd, float levelPct);
void updateLCD(float mainLps, float sumBranches, float measuredLossLpd, float levelPct);

/* ========== setup ========== */
void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(PIN_BTN_APPARENT, INPUT_PULLUP);
  pinMode(PIN_BTN_REAL, INPUT_PULLUP);

  for (int i=0;i<5;i++){
    pinMode(PIN_VALVE_LED[i], OUTPUT);
    digitalWrite(PIN_VALVE_LED[i], HIGH);
  }

  pinMode(PIN_ALARM_INFO, OUTPUT); digitalWrite(PIN_ALARM_INFO, LOW);
  pinMode(PIN_ALARM_WARN, OUTPUT); digitalWrite(PIN_ALARM_WARN, LOW);
  pinMode(PIN_ALARM_CRIT, OUTPUT); digitalWrite(PIN_ALARM_CRIT, LOW);

  pinMode(PIN_TRIG, OUTPUT); digitalWrite(PIN_TRIG, LOW);
  pinMode(PIN_ECHO, INPUT);

  /* LCD init (4-bit) */
  lcd.begin(16,4);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Smart Water System");
  lcd.setCursor(0,1);
  lcd.print("Starting...");
  delay(800);
  lcd.clear();

  UARL_Lps = UARL_liters_per_day / 86400.0f;

  Blynk.begin(auth, ssid, pass);

  Serial.println("Setup complete");
  Serial.print("UARL L/day: "); Serial.println(UARL_liters_per_day);
  Serial.print("UARL L/s: "); Serial.println(UARL_Lps, 6);
}

/* ========== main loop ========== */
void loop() {
  Blynk.run();

  if (digitalRead(PIN_BTN_APPARENT) == LOW && millis() - lastBtnAppMillis > DEBOUNCE_MS) {
    simulated_apparent_level++;
    lastBtnAppMillis = millis();
    Serial.print("Apparent++ => "); Serial.println(simulated_apparent_level);
  }
  if (digitalRead(PIN_BTN_REAL) == LOW && millis() - lastBtnRealMillis > DEBOUNCE_MS) {
    simulated_real_level++;
    lastBtnRealMillis = millis();
    Serial.print("Real++ => "); Serial.println(simulated_real_level);
  }

  float mainLps = readADCasLps(PIN_MAIN_ADC, MAIN_MAX_LPS);

  float branchLps[5];
  float sumBranches = 0.0f;
  for (int i=0;i<5;i++){
    branchLps[i] = readADCasLps(PIN_BRANCH_ADC[i], BRANCH_MAX_LPS);
    branchLps[i] *= (1.0f - 0.05f * simulated_apparent_level);
    sumBranches += branchLps[i];
  }

  float simulated_real_leak_Lps = 0.001f * simulated_real_level;
  float measuredLossLps = mainLps - (sumBranches - simulated_real_leak_Lps);
  if (measuredLossLps < 0.0f) measuredLossLps = 0.0f;
  float measuredLossLpd = measuredLossLps * 86400.0f;

  updateAlarmsAndAct(measuredLossLpd, branchLps, sumBranches);
  float tankLevelPct = readUltrasonicLevelPct();

  if (millis() - lastBlynkMs > BLYNK_INTERVAL_MS) {
    sendBlynkTelemetry(mainLps, sumBranches, measuredLossLpd, tankLevelPct);
    lastBlynkMs = millis();
  }

  if (millis() - lastLcdUpdate > LCD_UPDATE_MS) {
    updateLCD(mainLps, sumBranches, measuredLossLpd, tankLevelPct);
    lastLcdUpdate = millis();
  }

  Serial.print("Main L/s: "); Serial.print(mainLps,6);
  Serial.print(" | SumBranches L/s: "); Serial.print(sumBranches,6);
  Serial.print(" | SimRealLeak L/s: "); Serial.print(simulated_real_leak_Lps,6);
  Serial.print(" | Loss L/day: "); Serial.println(measuredLossLpd,2);

  delay(300);
}

/* ========== helpers ========== */

float readADCasLps(int adcPin, float maxLps) {
  int raw = analogRead(adcPin);
  float ratio = raw / ADC_MAX;
  return ratio * maxLps;
}

float readUltrasonicLevelPct() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (duration == 0) return -1.0f;
  float distance_cm = (duration * 0.0343f) / 2.0f;
  if (distance_cm < 0.0f) distance_cm = 0.0f;
  if (distance_cm > TANK_HEIGHT_CM) distance_cm = TANK_HEIGHT_CM;
  float levelPct = ((TANK_HEIGHT_CM - distance_cm) / TANK_HEIGHT_CM) * 100.0f;
  if (levelPct < 0.0f) levelPct = 0.0f;
  if (levelPct > 100.0f) levelPct = 100.0f;
  return levelPct;
}

void updateAlarmsAndAct(float measuredLossLpd, float branchLps[], float sumBranches) {
  bool critical = false;
  bool warning = false;
  if (measuredLossLpd > UARL_liters_per_day * ACTION_MULT) critical = true;
  else if (measuredLossLpd > UARL_liters_per_day * WARN_MULT) warning = true;

  digitalWrite(PIN_ALARM_INFO, (measuredLossLpd <= UARL_liters_per_day) ? HIGH : LOW);
  digitalWrite(PIN_ALARM_WARN, warning ? HIGH : LOW);
  digitalWrite(PIN_ALARM_CRIT, critical ? HIGH : LOW);

  if (critical) {
    int idxMax = 0;
    float maxv = branchLps[0];
    for (int i=1;i<5;i++){
      if (branchLps[i] > maxv){ maxv = branchLps[i]; idxMax = i; }
    }
    for (int i=0;i<5;i++){
      if (i==idxMax) digitalWrite(PIN_VALVE_LED[i], LOW);
      else digitalWrite(PIN_VALVE_LED[i], HIGH);
    }
    Serial.print("AUTO-CLOSE valve: "); Serial.println(idxMax);
    Blynk.notify(String("Critical loss detected -> auto-closed valve ") + String(idxMax));
  } else {
    for (int i=0;i<5;i++) digitalWrite(PIN_VALVE_LED[i], HIGH);
  }
}

void sendBlynkTelemetry(float mainLps, float sumBranches, float measuredLossLpd, float levelPct) {
  Blynk.virtualWrite(V1, mainLps);
  Blynk.virtualWrite(V2, sumBranches);
  Blynk.virtualWrite(V3, measuredLossLpd);
  Blynk.virtualWrite(V4, levelPct);

  int alarmState = 0;
  if (digitalRead(PIN_ALARM_CRIT) == HIGH) alarmState = 2;
  else if (digitalRead(PIN_ALARM_WARN) == HIGH) alarmState = 1;
  Blynk.virtualWrite(V5, alarmState);

  for (int i=0;i<5;i++){
    int state = (digitalRead(PIN_VALVE_LED[i])==HIGH) ? 1 : 0;
    Blynk.virtualWrite(V10 + i, state);
  }

  Serial.println("Blynk telemetry sent");
}

void updateLCD(float mainLps, float sumBranches, float measuredLossLpd, float levelPct) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Smart Water Sys");
  lcd.setCursor(0,1);
  lcd.print("M:");
  lcd.print(mainLps,3);
  lcd.print(" B:");
  lcd.print(sumBranches,3);
  lcd.setCursor(0,2);
  lcd.print("Loss:");
  lcd.print(measuredLossLpd,1);
  lcd.print("L/d");
  lcd.setCursor(0,3);
  if (levelPct >= 0.0f) {
    lcd.print("Tank:");
    lcd.print(levelPct,0);
    lcd.print("% ");
  } else {
    lcd.print("Tank:N/A ");
  }
  if (digitalRead(PIN_ALARM_CRIT)==HIGH) {
    lcd.print("CRIT");
  } else if (digitalRead(PIN_ALARM_WARN)==HIGH) {
    lcd.print("WARN");
  } else {
    lcd.print("OK");
  }
}
