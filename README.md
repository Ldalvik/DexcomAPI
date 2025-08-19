# DexcomAPI
This library is based off https://github.com/gagebenne/pydexcom

# HOW TO USE
Connect to a WiFi network
```C
  WiFi.mode(WIFI_STA);
  WiFi.begin("your wifi ssid", "your wifi password");
  while (WiFi.status() != WL_CONNECTED);
  // If it gets to here, WiFi was successfully connected
```

Login to a Dexcom account
```C
  if (!dexcom.createSession("dexcom user/email/number", "dexcom password")) {
    // See below...
  }

```

Get the status of a failed login
```C
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
```

Get a single reading
```C
GlucoseData d = dexcom.getLastGlucose();
if (d.glucose == -1) return;
Serial.print("Glucose: ");   Serial.println(d.glucose);
Serial.print("Trend: ");     printTrend(d.trend);
Serial.print("Adv Trend: "); printAdvTrend(d.advTrend);
Serial.print("Range: ");     printRange(d.range);
Serial.print("Timestamp: "); Serial.println(d.timestamp);
```

Get multiple readings
```C
int readings = 270; // get the last 270 readings. With readings every 5min, it will give us 4 hours of data
for (auto& d : dexcom.getGlucose(readings * 5, readings)) {
  if (d.glucose == -1) continue;
  Serial.print("Glucose: ");   Serial.println(d.glucose);
  Serial.print("Trend: ");     printTrend(d.trend);
  Serial.print("Adv Trend: "); printAdvTrend(d.advTrend);
  Serial.print("Range: ");     printRange(d.range);
  Serial.print("Timestamp: "); Serial.println(d.timestamp);
}
```

Get advanced trend type
```C
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
```

Get trend type
```C
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
```

Get glucose range
```C
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
```

# TODO
## Security
- Stop using client.setInsecure and find correct Root Certificate
- Use ArduinoJson + measureJson() and stream login data into the socket
- Fixed buffers, remove copies, zeroize after use
- Encrypt session token when stored (flash encryption + NVS encryption)
## QoL
- Get session token expiration time, automatically re-auth when needed
- Store session token and check for expiration when booted up

- Maybe add safe-guard to stop accidentally sending many requests


