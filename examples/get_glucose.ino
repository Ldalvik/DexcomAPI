#include <Dexcom.h>

// Set to your wifi credentials. Its possible to setup a full captive login portal via wifi too, even the dexcom login credentials, try it out!
// One day I will make this library more secure, using root certs, credential scrubbing, web portal encryption, etc. which will make it safer.
// Remember that the MITM attacks are unlikely but possible! Do your best to secure sensitive info.
String SSID = "your wifi ssid";
String SSID_PASSWORD = "your wifi password";

// Check this repo for more info on setting up and using the correct login credentials
// https://github.com/gagebenne/pydexcom/blob/main/README.md
String DEXCOM_USERNAME = "dexcom username/email/phone number";
String DEXCOM_PASSWORD = "dexcom password";

Dexcom dexcom = Dexcom();

hw_timer_t* timer = NULL;
volatile bool check = true;

void IRAM_ATTR checkGlucose() {
  check = true;
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PASSWORD);

  Serial.print("Attempting to connect to ");
  Serial.print(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("connected!");

  Serial.print("Logging into dexcom as ");
  Serial.println(DEXCOM_USERNAME);

  if (!dexcom.createSession(DEXCOM_USERNAME, DEXCOM_PASSWORD)) {
    printError();
    while (true);
  }
  Serial.println("Logged in!");

  // You can set the high and low threshold, otherwise the default is high = 200, low = 80
  // You can also set urgent high/low thresholds, with a default of urgent high = 300, urgent low = 65
  // dexcom.HIGH_THRESHOLD = 300;
  // dexcom.URGENT_HIGH_THRESHOLD = 350;
  // dexcom.LOW_THRESHOLD = 90;
  // dexcom.URGENT_LOW_THRESHOLD = 75;

  timer = timerBegin(1);
  timerAttachInterrupt(timer, &checkGlucose);
  timerAlarm(timer, 300, true, 0); // NEVER less than 5 minutes (300 seconds)
}

void loop() {
  if (check) {
    check = false;
    getBatchGlucose();
    // getSingleGlucose();
  }
}

// Example code to get multiple glucose data sets
void getBatchGlucose() {
  int readings = 270; // get the last 270 readings. With readings every 5min, it will give us 4 hours of data
  for (auto& d : dexcom.getGlucose(readings * 5, readings)) {
    if (d.glucose == -1) continue;

    Serial.print("Glucose: ");   Serial.println(d.glucose);
    Serial.print("Trend: ");     printTrend(d.trend);
    Serial.print("Adv Trend: "); printAdvTrend(d.advTrend);
    Serial.print("Range: ");     printRange(d.range);
    Serial.print("Timestamp: "); Serial.println(d.timestamp);
  }
}

// Example code to get a single glucose data set
void getSingleGlucose() {
  GlucoseData d = dexcom.getLastGlucose();
  if (d.glucose == -1) return;

  Serial.print("Glucose: ");   Serial.println(d.glucose);
  Serial.print("Trend: ");     printTrend(d.trend);
  Serial.print("Adv Trend: "); printAdvTrend(d.advTrend);
  Serial.print("Range: ");     printRange(d.range);
  Serial.print("Timestamp: "); Serial.println(d.timestamp);
}

// Use the enums in the example code below to help keep track of different statuses
void printAdvTrend(GlucoseAdvTrend advTrend) {
  switch (advTrend) {
    case GlucoseAdvTrend::DoubleUpHigh:         Serial.println("HIGH and RISING FAST"); break;
    case GlucoseAdvTrend::DoubleUpInRange:      Serial.println("IN RANGE and RISING FAST"); break;
    case GlucoseAdvTrend::DoubleUpLow:          Serial.println("LOW and RISING FAST"); break;

    case GlucoseAdvTrend::SingleUpHigh:         Serial.println("HIGH and RISING"); break;
    case GlucoseAdvTrend::SingleUpInRange:      Serial.println("IN RANGE and RISING"); break;
    case GlucoseAdvTrend::SingleUpLow:          Serial.println("LOW and RISING"); break;

    case GlucoseAdvTrend::FortyFiveUpHigh:      Serial.println("HIGH and SLIGHTLY RISING"); break;
    case GlucoseAdvTrend::FortyFiveUpInRange:   Serial.println("IN RANGE and SLIGHTLY RISING"); break;
    case GlucoseAdvTrend::FortyFiveUpLow:       Serial.println("LOW and SLIGHTLY RISING"); break;

    case GlucoseAdvTrend::FlatHigh:             Serial.println("HIGH and STEADY"); break;
    case GlucoseAdvTrend::FlatInRange:          Serial.println("IN RANGE and STEADY"); break;
    case GlucoseAdvTrend::FlatLow:              Serial.println("LOW and STEADY"); break;

    case GlucoseAdvTrend::FortyFiveDownHigh:    Serial.println("HIGH and SLIGHTLY FALLING"); break;
    case GlucoseAdvTrend::FortyFiveDownInRange: Serial.println("IN RANGE and SLIGHTLY FALLING"); break;
    case GlucoseAdvTrend::FortyFiveDownLow:     Serial.println("LOW and SLIGHTLY FALLING"); break;

    case GlucoseAdvTrend::SingleDownHigh:       Serial.println("HIGH and FALLING"); break;
    case GlucoseAdvTrend::SingleDownInRange:    Serial.println("IN RANGE and FALLING"); break;
    case GlucoseAdvTrend::SingleDownLow:        Serial.println("LOW and FALLING"); break;

    case GlucoseAdvTrend::DoubleDownHigh:       Serial.println("HIGH and FALLING FAST"); break;
    case GlucoseAdvTrend::DoubleDownInRange:    Serial.println("IN RANGE and FALLING FAST"); break;
    case GlucoseAdvTrend::DoubleDownLow:        Serial.println("LOW and FALLING FAST"); break;

    case GlucoseAdvTrend::Unknown:
    default: Serial.println("Unknown trend"); break;
  }
}

void printTrend(GlucoseTrend trend) {
  switch (trend) {
    case GlucoseTrend::DoubleUp:      Serial.println("Rising fast"); break;
    case GlucoseTrend::SingleUp:      Serial.println("Rising"); break;
    case GlucoseTrend::FortyFiveUp:   Serial.println("Slightly rising"); break;
    case GlucoseTrend::Flat:          Serial.println("Steady :)"); break;
    case GlucoseTrend::FortyFiveDown: Serial.println("Slightly falling"); break;
    case GlucoseTrend::SingleDown:    Serial.println("Falling"); break;
    case GlucoseTrend::DoubleDown:    Serial.println("Falling fast"); break;
    case GlucoseTrend::NotComputable:
    case GlucoseTrend::RateOutOfRange:
    default: Serial.println("Not computable/Value out of range"); break;
  }
}

void printRange(GlucoseRange range) {
  switch (range) {
    case GlucoseRange::UrgentHigh: Serial.println("Urgent high"); break;
    case GlucoseRange::TooHigh:    Serial.println("Too high"); break;
    case GlucoseRange::InRange:    Serial.println("In range :)"); break;
    case GlucoseRange::TooLow:     Serial.println("Too Low"); break;
    case GlucoseRange::UrgentLow:  Serial.println("Urgent low"); break;
    default: break;
  }
}

void printError() {
  switch (dexcom.accountStatus) {
    case DexcomStatus::SessionNotValid:   Serial.println("Session ID invalid"); break;
    case DexcomStatus::SessionNotFound:   Serial.println("Session not found"); break;
    case DexcomStatus::AccountNotFound:   Serial.println("Account not found"); break;
    case DexcomStatus::PasswordInvalid:   Serial.println("Password invalid"); break;
    case DexcomStatus::MaxAttempts:       Serial.println("Maximum authentication attempts exceeded"); break;
    case DexcomStatus::UsernameNullEmpty: Serial.println("Username NULL or empty"); break;
    case DexcomStatus::PasswordNullEmpty: Serial.println("Password NULL or empty"); break;
    default: Serial.println("Unknown error"); break;
  }
}