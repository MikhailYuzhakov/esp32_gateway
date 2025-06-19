#ifndef HOTSPOTCONFIG_H
#define HOTSPOTCONFIG_H

#include <ArduinoJson.h>
#include "DeviceConfiguration.h"

/**
 * {"id":1,"ssid":"Room 52", "pass":"10031998", "ftpLogin":"uskd", "ftpPass":"1003", "apssid":"USKD Hotspot", "appass":"10031998", "freq":868000000, "bw":125000, "sf":12, "pwr":20, "timezone":3}
 */
class HotspotConfig : public DeviceConfiguration
{
    private:
        String ssid;
        String pass;
        String apssid;
        String appass;
        String ftpLogin;
        String ftpPass;
        int8_t timezone;

    public:
        String getSsid() {
            return ssid;
        }

        void setSsid(String newSsid) {
            ssid = newSsid;
        }

        String getPass() {
            return pass;
        }

        void setPass(String newPass) {
            pass = newPass;
        }

        String getFtpLogin() {
            return ftpLogin;
        }

        void setFtpLogin(String newFtpLogin) {
            ftpLogin = newFtpLogin;
        }

        String getFtpPass() {
            return ftpPass;
        }

        void setFtpPass(String newFtpPass) {
            ftpPass = newFtpPass;
        }

        String getApPass() {
            return appass;
        }

        void setApPass(String newPass) {
            appass = newPass;
        }

        String getApSsid() {
            return apssid;
        }

        void setApSsid(String newSsid) {
            apssid = newSsid;
        }

        int8_t getTimezone() {
            return timezone;
        }

        void setTimezone(int8_t newTimezone) {
            timezone = newTimezone;
        }

        /**
         * Организовать какую-то проверку на корректоность JSON
         */
        bool set(String jsonConfig) {
            deserializeJson(DeviceConfiguration::jsonBuffer, jsonConfig);
            
            DeviceConfiguration::setId(DeviceConfiguration::jsonBuffer["id"].as<uint16_t>());
            DeviceConfiguration::setFrequency(DeviceConfiguration::jsonBuffer["freq"].as<uint32_t>());
            DeviceConfiguration::setBandwidth(DeviceConfiguration::jsonBuffer["bw"].as<uint32_t>());
            DeviceConfiguration::setSpreadingFactor(DeviceConfiguration::jsonBuffer["sf"].as<uint8_t>());
            DeviceConfiguration::setPower(DeviceConfiguration::jsonBuffer["pwr"].as<uint8_t>());
            setSsid(DeviceConfiguration::jsonBuffer["ssid"].as<String>());
            setPass(DeviceConfiguration::jsonBuffer["pass"].as<String>());
            setFtpLogin(DeviceConfiguration::jsonBuffer["ftpLogin"].as<String>());
            setFtpPass(DeviceConfiguration::jsonBuffer["ftpPass"].as<String>());
            setApSsid(DeviceConfiguration::jsonBuffer["apssid"].as<String>());
            setApPass(DeviceConfiguration::jsonBuffer["appass"].as<String>());
            setTimezone(DeviceConfiguration::jsonBuffer["timezone"].as<int8_t>());

            return true;
        }

        String get() {
            return  "Wi-Fi SSID:\t" + getSsid() + "\n" 
                    + "Wi-Fi PSWD:\t" + getPass() + "\n"
                    + "FTP Login:\t" + getFtpLogin() + "\n"
                    + "FTP Password:\t" + getFtpPass() + "\n"
                    + "Wi-Fi AP SSID:\t" + getApSsid() + "\n"
                    + "Wi-Fi AP PSWD:\t" + getApPass() + "\n"
                    + "LoRa FREQ:\t" + DeviceConfiguration::getFrequency() + "\n"
                    + "LoRa BW:\t" + DeviceConfiguration::getBandwidth() + "\n"
                    + "LoRa SF:\t" + DeviceConfiguration::getSpreadingFactor() + "\n"
                    + "LoRa PWR:\t" + DeviceConfiguration::getPower() + "\n"
                    + "Timezone:\t" + getTimezone() + "\n";
        }
};

#endif