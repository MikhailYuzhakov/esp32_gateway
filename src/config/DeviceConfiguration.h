#ifndef DEVICECONFIGURATION_H
#define DEVICECONFIGURATION_H

#include <Arduino.h>

class DeviceConfiguration
{
    private:
        
    public:
        JsonDocument jsonBuffer;
        uint16_t id;
        uint32_t frequency;
        uint32_t bandwidth;
        uint8_t spreadingFactor;
        uint8_t power;

        DeviceConfiguration() {Serial.println("constructor");}

        void setId(uint16_t newId) {
            this->id = newId;
        }

        uint16_t getId() {
            return id;
        }

        uint32_t getFrequency() {
            return frequency;
        }

        void setFrequency(uint32_t newFreq) {
            uint32_t minFreq = 850000000;
            uint32_t maxFreq = 931000000;

            if (newFreq > maxFreq)
                frequency = maxFreq;
            else if (newFreq < minFreq)
                frequency = minFreq;
            else
                frequency = newFreq;
        }

        uint8_t getSpreadingFactor() {
            return spreadingFactor;
        }

        void setSpreadingFactor(uint8_t newSpreadingFactor) {
            enum {SIZE = 7};
            uint8_t availableSf[SIZE] = {6, 7, 8, 9, 10, 11, 12};

            for (uint8_t i = 0; i < SIZE; i++)
            {
                if (newSpreadingFactor == availableSf[i]) {
                    spreadingFactor = newSpreadingFactor;
                    break;
                } else {
                    spreadingFactor = availableSf[SIZE - 1];
                }
            }
        }

        uint32_t getBandwidth() {
            return bandwidth;
        }

        void setBandwidth(uint32_t newBandwidth) {
            enum {SIZE = 10};
            uint32_t availableBw[SIZE] = {7800, 10400, 15600, 20800, 31200, 41700, 62500, 125000, 250000, 500000};

            for (uint8_t i = 0; i < SIZE; i++)
            {
                if (newBandwidth == availableBw[i]) {
                    bandwidth = newBandwidth;
                    break;
                } else {
                    bandwidth = availableBw[SIZE - 2];
                }
            }
        }

        uint8_t getPower() {
            return power;
        }

        void setPower(uint8_t newPower) {
            uint8_t minPower = 0;
            uint8_t maxPower = 20;

            if (newPower > maxPower)
                power = maxPower;
            else if (newPower < minPower)
                power = minPower;
            else
                power = newPower;
        }

        String getJsonConfig() {
            String jsonConfig;
            serializeJson(this->jsonBuffer, jsonConfig);
            return jsonConfig;
        }

        virtual bool set(String jsonConfig) = 0;

        virtual String get() = 0;
};

#endif