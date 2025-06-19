#ifndef PROBECONFIG_H
#define PROBECONFIG_H

#include "DeviceConfiguration.h"
#include <ArduinoJson.h>

/**
 * {"id":13, "transmitionPeriod":9000, "measurementPeriod":1800,  "freq":868000000, "bw":125000, "sf":12, "pwr":20}
 * TODO:
 * Последнее время онлайна и если больше недели, то удалять карточку и настройки для зонда
 */
class ProbeConfig : public DeviceConfiguration
{
    private:
        uint32_t transmitionPeriod;
        uint32_t measurementPeriod;
        
    public:
        void setTransmitPeriod(uint32_t transmitionPeriod) {
            this->transmitionPeriod = transmitionPeriod;
        }

        uint32_t getTransmitPeriod() {
            return this->transmitionPeriod;
        }

        void setMeasurePeriod(uint32_t measurementPeriod) {
            this->measurementPeriod = measurementPeriod;
        }

        uint32_t getMeasurePeriod() {
            return this->measurementPeriod;
        }

        bool set(String jsonConfig) {
            deserializeJson(DeviceConfiguration::jsonBuffer, jsonConfig);
                  
            DeviceConfiguration::setId(DeviceConfiguration::jsonBuffer["id"].as<uint16_t>());
            DeviceConfiguration::setFrequency(DeviceConfiguration::jsonBuffer["freq"].as<uint32_t>());
            DeviceConfiguration::setBandwidth(DeviceConfiguration::jsonBuffer["bw"].as<uint32_t>());
            DeviceConfiguration::setSpreadingFactor(DeviceConfiguration::jsonBuffer["sf"].as<uint8_t>());
            DeviceConfiguration::setPower(DeviceConfiguration::jsonBuffer["pwr"].as<uint8_t>());
            setTransmitPeriod(DeviceConfiguration::jsonBuffer["probeTransTime"].as<uint32_t>());
            setMeasurePeriod(DeviceConfiguration::jsonBuffer["probeReadTime"].as<uint32_t>());

            return true;
        }

        String get() {
            return "Probe id:\t" + (String)DeviceConfiguration::getId() + "\n"
                    + "TransPeriod:\t" + getTransmitPeriod() + "\n"
                    + "MeasPeriod:\t" + getMeasurePeriod() + "\n"
                    + "LoRa FREQ:\t" + DeviceConfiguration::getFrequency() + "\n"
                    + "LoRa BW:\t" + DeviceConfiguration::getBandwidth() + "\n"
                    + "LoRa SF:\t" + DeviceConfiguration::getSpreadingFactor() + "\n"
                    + "LoRa PWR:\t" + DeviceConfiguration::getPower() + "\n";
        }
};

#endif