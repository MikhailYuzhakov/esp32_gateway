#include <WiFi.h>

class Network
{
    private:
        uint8_t retryCnt = 25;
        String ssid, pass, apssid, appass;

        String toStringAPInfo(IPAddress ip) {
            return "\nWi-Fi AP SSID:\t" + this->apssid + "\n"
                    + "Wi-Fi AP PSWD:\t" + this->appass + "\n"
                    + "Web IP:\t" + ip.toString() + "\n";
        }

        bool connectToNetwork() {
            WiFi.begin(ssid, pass);
            while (WiFi.status() != WL_CONNECTED) //пытаемся подключиться к Wi-Fi
            {
                Serial.print(".");
                delay(1000);
            }
            return WiFi.isConnected();
        }

        void createWiFiAccessPoint() {
            WiFi.disconnect(); // Отключаем WIFI
            WiFi.mode(WIFI_AP_STA); // Меняем режим на режим точки доступа
            IPAddress apIP(192, 168, 1, 1); //задаем IP точки доступа
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0)); // Задаем настройки сети
            WiFi.softAP(this->apssid, this->appass); //запускаем точку доступа

            Serial.print(toStringAPInfo(apIP));
        }

    public:
        void setSSID(String ssid) {
            this->ssid = ssid;
        }

        void setPassword(String pass) {
            this->pass = pass;
        }

        void setApSSID(String apssid) {
            this->apssid = apssid;
        }

        void setApPass(String appass) {
            this->appass = appass;
        }

        bool initWiFi() {
            if (!connectToNetwork()) {
                createWiFiAccessPoint();
            } else {
                Serial.println("\n" + WiFi.localIP().toString());
            }
            return WiFi.isConnected();
        }

        bool isConnect() {
            return WiFi.isConnected();
        }
};
