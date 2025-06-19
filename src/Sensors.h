class Sensors
{
    private:
        String formatDateTime(uint8_t* datetime) {
            char buffer[20]; // Буфер для форматированной строки (достаточно для "dd.mm.yy hh:MM:ss\0")

            // Форматируем дату и время в строку
            snprintf(
                buffer, 
                sizeof(buffer), 
                "%02d.%02d.%02d %02d:%02d:%02d",
                datetime[9],  // День
                datetime[10],  // Месяц
                datetime[11], // Год (уже смещен на 2000)
                datetime[12],  // Часы
                datetime[13],  // Минуты
                datetime[14]   // Секунды
            );

            return String(buffer);
        }

        /**
         * Преобразует два байт в число с плавающей точкой и возвращается строку с ним.
         */
        String convertBytesToString(uint8_t firstByte, uint8_t secondByte, float dividerValue, bool isUnsigned) {
            float floatValue = 0.0f;
            
            if (isUnsigned) {
                uint16_t uintValue = (static_cast<uint16_t>(firstByte) << 8) | secondByte;
                floatValue = static_cast<float>(uintValue) / dividerValue;
            } else {
                int16_t intValue = (static_cast<int16_t>(firstByte) << 8) | secondByte;
                floatValue = static_cast<float>(intValue) / dividerValue;
            }
            
            return String(floatValue, 2);  // 2 знака после запятой
        }

    public:
        String getDateTime(uint8_t* dataIn) {
            return formatDateTime(dataIn);
        }

        String getPressure(uint8_t* dataIn) {
            return convertBytesToString(dataIn[15], dataIn[16], 10.0f, true);
        }

        String getHtuAirTemperature(uint8_t* dataIn) {
            return convertBytesToString(dataIn[19], dataIn[20], 100.0f, false);
        }

        String getBmpAirTemperature(uint8_t* dataIn) {
            return convertBytesToString(dataIn[17], dataIn[18], 100.0f, false);
        }

        String getAirHumidiry(uint8_t* dataIn) {
            return convertBytesToString(dataIn[21], dataIn[22], 100.0f, true);
        }
};
