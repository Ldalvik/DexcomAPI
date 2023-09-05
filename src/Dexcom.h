#ifndef Dexcom_h
#define Dexcom_h

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include "HTTPClient.h"
#include <WiFi.h>
#endif

#include <Const.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

class Dexcom
{
public:
    Dexcom(const char *username, const char *password, bool ous = false);
    bool createSession();
    bool update();
    // bool updateList(int minutes, int maxCount);
    void setLowThreshold(int threshold);
    void setHighThreshold(int threshold);
    void setWarningThreshold(int threshold);
      
    int currentGlucose = 0;
    TrendTypes currentTrend;
    AdvTrendTypes currentAdvTrend;
    RangeTypes currentRange;
    StatusTypes currentStatus = LOGGED_OUT;

    int lowThreshold = 90;     // default value for below range
    int highThreshold = 250;   // default value for above range
    int warningThreshold = 20; // threshold above or below range for warning 
    const char *_username;

private:
    String post(const char *url, const char *postData);
    String getAccountId();
    String getSessionId(String accountId);
    String stripToken(String data);
    TrendTypes getTrendType(String trend);
    AdvTrendTypes getAdvTrendType();
    RangeTypes getRange();

    String _session_id;
    String _base_url = DEXCOM_BASE_URL;

    const char *_password;
};

#endif