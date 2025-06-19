#include <ESPAsyncWebServer.h>
#include "config/HotspotConfig.h"
#include "config/ProbeConfigBuilder.h"

#define CONFIG_FILE_PATH "/configs/hotspot_config.json"

AsyncWebServer server(80);
AsyncEventSource events("/events");
extern HotspotConfig hotspotConfig;
extern ProbeConfigBuilder probesConfigBuilder;
/**
 * Нарушение SRP, делегировать работу с фаловой системе отдельному классу FileSystem
 */
class WebServer
{
    private:
        void writeConfigToMemory(AsyncWebServerRequest *request) {
            JsonDocument configHotspot;
            uint16_t i = 0;
            configHotspot["id"] = "1";
            configHotspot["ssid"] = request->arg(i).c_str(); i++;
            configHotspot["pass"] = request->arg(i).c_str(); i++;
            configHotspot["apssid"] = request->arg(i).c_str(); i++;
            configHotspot["appass"] = request->arg(i).c_str(); i++;
            configHotspot["bw"] = request->arg(i).c_str(); i++;
            configHotspot["sf"] = request->arg(i).c_str(); i++;
            configHotspot["freq"] = request->arg(i).c_str(); i++;
            configHotspot["pwr"] = request->arg(i).c_str(); i++;
            configHotspot["timezone"] = "3";
        }
    public:
        void init(FileSystem *fileSystem) {
            server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/index.htm", String());
            });

            server.on("/css/main.css", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/css/main.css", "text/css");
            });

            server.on("/css/index.css", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/css/index.css", "text/css");
            });

            server.on("/css/loader.css", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/css/loader.css", "text/css");
            });

            server.on("/css/config.css", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/css/config.css", "text/css");
            });

            server.on("/img/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/img/logo.png");
            });

            server.on("/js/main.js", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/js/main.js", "text/js");
            });

            server.on("/js/config.js", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/js/config.js", "text/js");
            });

            server.on("/config.live.json", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/config.live.json", "application/json");
            });

            server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/html/config.html", String());
            });

            server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
                request->send(LittleFS, "/favicon.ico", String());
            });

            server.on("/hotspot-config", HTTP_GET, [fileSystem](AsyncWebServerRequest *request){
                request->send(200, "text/plain", hotspotConfig.getJsonConfig());
            });

            server.on("/probes-configs", HTTP_GET, [fileSystem](AsyncWebServerRequest *request){ 
                request->send(200, "text/plain", probesConfigBuilder.getJsonProbesConfig());
            });

            server.on("/terminal", HTTP_GET, [fileSystem](AsyncWebServerRequest *request){ 
                request->send(LittleFS, "/html/terminal.html", String());
            });

            server.on(
                "/terminal/send",
                HTTP_POST, 
                [](AsyncWebServerRequest *request){},
                NULL,
                [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total)
                {
                    String bodyData;
                    for (size_t i = 0; i < len; i++) {
                        bodyData += (char) data[i];
                        // Serial.printf("0x%02X ", data[i]);
                    }
                    Serial2.println(bodyData);
                    
                    request->send(200);
                });

            // Настройка EventSource для асинхронных обновлений
            events.onConnect([](AsyncEventSourceClient *client){
                if(client->lastId()){
                    Serial.printf("Client reconnected! Last message ID: %u\n", client->lastId());
                }
                client->send("hello!", NULL, millis(), 1000);
            });

            server.on("/hotspot", HTTP_GET, [fileSystem](AsyncWebServerRequest *request){
                //тут парсим ответ от фронта и записываем в память
                request->send(200, "text/plain", "Ok");
            });

            server.addHandler(&events);
            server.begin();
        }
};