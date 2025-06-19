#ifndef PROBECONFIGBUILDER_H
#define PROBECONFIGBUILDER_H

#include "ProbeConfig.h"
#include <ArduinoJson.h>

#define MAX_PROBES 20
#define PROBES_CONFIGS_PATH "/configs/"

/**
 * Класс преобразующий JSON массив, считанный из файлов конфигурации зондов в массив объектов ProbesConfig.
 */
class ProbeConfigBuilder
{
    private:
        String sProbesConfig;
        ProbeConfig probesConfigs[MAX_PROBES];
        uint8_t probesNumber;

    public:
        String buildConfigs(String sProbesConfig) {
            String warnings = "";
            JsonDocument jProbesConfig; //ссылка на массив Json
            DeserializationError error = deserializeJson(jProbesConfig, sProbesConfig); //парсим строку в объект типа JsonDocument

            if (error) 
                return String("JSON parse error: ") + error.c_str(); // обработка ошибки парсинга JSON

            if (!jProbesConfig.is<JsonArray>()) 
                return "Error: Expected JSON array"; // обработка ошибки, что на вход пришла строка формата отличного от JSON
            
            JsonArray jArrayProbesConfig = jProbesConfig.as<JsonArray>(); //создаем массив конфигураций
            this->probesNumber = jArrayProbesConfig.size(); //получаем размер массива

            if (this->probesNumber > MAX_PROBES) {
                warnings += "numbers of probes in flash memory greater than MAX_PROBES! Edit ProbesConfigBuilder.h/MAX_PROBES\n";
                this->probesNumber = MAX_PROBES;
            }

            //перебираем массив Json и записываем в объект ProbeConfig конфигурацию каждого зонда
            for (uint8_t i = 0; i < probesNumber; i++) {
                if (!probesConfigs[i].set(jArrayProbesConfig[i])) 
                    return String("Failed to set config for probe ") + String(i);
                
            }

            return warnings; // возвращаем предупреждения, если были
        }

        String getJsonProbesConfig() {
            return this->sProbesConfig;
        }

        String printProbesConfigs() {
            String result = "";
            for (uint8_t i = 0; i < probesNumber; i++)
            {
                result += probesConfigs[i].get();
                result += "===========================\n";
            }
            return result;
        }
};

#endif
