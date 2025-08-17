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
#include <vector>

class Dexcom
{
public:
    Dexcom(bool ous = false);
    bool createSession(const String &username, const String &password);
    GlucoseData getLastGlucose();
    std::vector<GlucoseData> getGlucose(int minutes, int maxCount);

    int HIGH_THRESHOLD       = 200;  // default value for above range
    int LOW_THRESHOLD        = 80;   // default value for below range
    int URGENT_HIGH_THRESHOLD = 300; // default high threshold urgent alarm
    int URGENT_LOW_THRESHOLD  = 65;  // default low threshold for urgent alarm

    DexcomStatus accountStatus = DexcomStatus::LoggedOut;

private:
    String            post(const char *url, const char *postData);
    String            getAccountId(const String &username, const String &password);
    String            getSessionId(const String &accountId, const String &password);
    String            stripToken(String data);
    GlucoseTrend      getTrendType(const String& trend);
    GlucoseAdvTrend   getAdvTrendType(int glucose, GlucoseTrend trend);
    GlucoseRange      getRange(int glucose);

    String _session_id;
    String _base_url = DEXCOM_BASE_URL;
};

#endif