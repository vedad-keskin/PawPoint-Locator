/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

//This example shows how to set and delete data with checking the matching between node path ETag (unique identifier string)
//and provided Etag

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the RTDB URL */
#define DATABASE_URL "URL" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

void setup()
{

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    //Or use legacy authenticate method
    //config.database_url = DATABASE_URL;
    //config.signer.tokens.legacy_token = "<database secret>";

    Firebase.begin(&config, &auth);

    Firebase.reconnectWiFi(true);
}

void loop()
{
    if (Firebase.ready() && !taskCompleted)
    {
        taskCompleted = true;

        String ETag = "";
        String wrong_ETag = "ANY_WRONG_ETag";

        Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "/test/int/data", 100) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
        {
            Serial.printf("etag, %s\n\n", fbdo.ETag().c_str());
            ETag = fbdo.ETag();
        }

        Serial.printf("Set int with valid ETag... %s\n", Firebase.RTDB.setInt(&fbdo, "/test/int/data", 200, ETag.c_str()) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
            Serial.printf("etag, %s\n\n", fbdo.ETag().c_str());

        Serial.printf("Set int with invalid ETag... %s\n", Firebase.RTDB.setInt(&fbdo, "/test/int/data", 200, wrong_ETag.c_str()) ? "ok" : fbdo.errorReason().c_str());

        if (fbdo.httpCode() == FIREBASE_ERROR_HTTP_CODE_OK)
            Serial.printf("etag, %s\n\n", fbdo.ETag().c_str());
    }
}
