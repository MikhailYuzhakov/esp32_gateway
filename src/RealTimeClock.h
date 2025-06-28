#include <GyverNTP.h>
#include <ESP32Time.h>

#define NTP_REFRESH_PERIOD 60000

class RealTimeClock {
private:
    uint32_t timer;
    int8_t timezone;
    ESP32Time rtc; // Объявляем rtc как член класса

public:
    RealTimeClock(int8_t timezone) : rtc(timezone * 3600) { // Инициализируем rtc в конструкторе
        // Установка начального времени для rtc
        rtc.setTime(30, 24, 15, 17, 1, 2000); // 17th Jan 2000 15:24:30
    }

    bool initNTP(int8_t timezone) {
        NTP.setPeriod(10);
        return NTP.begin(timezone);
    }

    void ntpClientHandler() {
        NTP.tick();
        if (millis() - timer >= NTP_REFRESH_PERIOD) {
            timer = millis();
            Serial.println(NTP.toString());
        }
    }

    /**
     * Получает строку с датой и временем, парсит и записывает в RTC
     */
    bool syncInternalRTCDateTime(String input) {
        // Проверяем минимальную структуру ответа
        int firstNewline = input.indexOf('\n');
        if (firstNewline == -1) return false;
        
        int secondNewline = input.indexOf('\n', firstNewline + 1);
        if (secondNewline == -1) return false;
        
        int okPos = input.indexOf("OK", secondNewline);
        if (okPos == -1) return false;

        // Извлекаем строку с датой между вторым переводом строки и OK
        String datetimeStr = input.substring(secondNewline + 1, okPos);
        datetimeStr.trim();

        // Проверяем базовый формат (YYYY-MM-DD HH:MM:SS)
        if (datetimeStr.length() < 19) return false;
        
        // Проверяем позиции разделителей
        if (datetimeStr[4] != '-' || datetimeStr[7] != '-' || 
            datetimeStr[10] != ' ' || datetimeStr[13] != ':' || 
            datetimeStr[16] != ':') {
            return false;
        }

        // Парсим компоненты даты и времени
        int year = datetimeStr.substring(0, 4).toInt();
        int month = datetimeStr.substring(5, 7).toInt();
        int day = datetimeStr.substring(8, 10).toInt();
        int hour = datetimeStr.substring(11, 13).toInt();
        int minute = datetimeStr.substring(14, 16).toInt();
        int second = datetimeStr.substring(17, 19).toInt();

        // Проверяем валидность значений
        if (year < 2023 || year > 2099 ||
            month < 1 || month > 12 ||
            day < 1 || day > 31 ||
            hour > 23 || minute > 59 || second > 59) {
            return false;
        }

        // Устанавливаем время в RTC
        rtc.setTime(second, minute, hour, day, month, year);
        return true;
    }

    uint8_t* getDateTime() {
        static uint8_t datetime[6];  // Статический массив для хранения результата
        
        // Получаем текущее время
        tm timeinfo = rtc.getTimeStruct();
        
        // Заполняем массив
        datetime[0] = (uint8_t)timeinfo.tm_mday;          // День (1-31)
        datetime[1] = (uint8_t)(timeinfo.tm_mon + 1);     // Месяц (1-12)
        datetime[2] = (uint8_t)(timeinfo.tm_year - 100);  // Год (текущий год - 2000)
        datetime[3] = (uint8_t)timeinfo.tm_hour;          // Часы (0-23)
        datetime[4] = (uint8_t)timeinfo.tm_min;           // Минуты (0-59)
        datetime[5] = (uint8_t)timeinfo.tm_sec;           // Секунды (0-59)
        
        return datetime;
    }

};
