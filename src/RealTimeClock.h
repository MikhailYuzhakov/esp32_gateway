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
    void syncInternalRTCDateTime(String input) {
        // Находим индекс начала и конца строки с датой и временем
        int firstEnter = input.indexOf("\n"); //ищем первый знак переноса
        int startIndex = input.indexOf("\n", firstEnter + 1); // Ищем второй знак переноса
        int endIndex = input.indexOf("OK"); // Находим следующий символ новой строки

        // Извлекаем строку с датой и временем
        String datetimeStr = input.substring(startIndex, endIndex);

        // Разделяем строку на дату и время
        int spaceIndex = datetimeStr.indexOf(" ");
        String dateStr = datetimeStr.substring(1, spaceIndex);
        String timeStr = datetimeStr.substring(spaceIndex + 1);
    
        // Парсим дату
        int year = dateStr.substring(0, 4).toInt();
        int month = dateStr.substring(5, 7).toInt();
        int day = dateStr.substring(8, 10).toInt();
    
        // Парсим время
        int hours = timeStr.substring(0, 2).toInt();
        int minutes = timeStr.substring(3, 5).toInt();
        int seconds = timeStr.substring(6, 8).toInt();
        
        // Устанавливаем время в RTC
        rtc.setTime(seconds, minutes, hours, day, month, year);
        Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
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
