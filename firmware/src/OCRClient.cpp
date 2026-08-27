#include "OCRClient.h"
#include "Config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

String OCRClient::sendImageForOCR(camera_fb_t *fb) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return "__ERROR__";
    }

    HTTPClient http;
    http.begin(API_URL);
    // Boundary for multipart data
    String boundary = "----ESP32BoundaryUpload";
    http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);

    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"file\"; filename=\"capture.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    size_t payloadLength = bodyStart.length() + fb->len + bodyEnd.length();
    
    // We cannot construct the entire payload in memory for large images easily
    // So we use HTTPClient send stream. Wait, HTTPClient only accepts a stream or exact size.
    // For simplicity, we can load it to buffer or send exact chunks.
    // Actually, http.sendRequest requires the complete stream.
    // Given PSRAM, we could allocate memory, but let's do a basic pointer write if possible.
    uint8_t *payload = (uint8_t *)ps_malloc(payloadLength);
    if (!payload) {
        Serial.println("Failed to allocate PSRAM for payload");
        return "__ERROR__";
    }
    
    memcpy(payload, bodyStart.c_str(), bodyStart.length());
    memcpy(payload + bodyStart.length(), fb->buf, fb->len);
    memcpy(payload + bodyStart.length() + fb->len, bodyEnd.c_str(), bodyEnd.length());

    int httpResponseCode = http.POST(payload, payloadLength);
    String extractedText = "";

    if (httpResponseCode > 0) {
        String response = http.getString();
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, response);
        if (!error) {
            extractedText = doc["text"].as<String>();
        } else {
            Serial.println("Failed to parse JSON");
            extractedText = "__ERROR__";
        }
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        extractedText = "__ERROR__";
    }

    free(payload);
    http.end();
    
    return extractedText;
}
