#include "Printer.h"
#include <HardwareSerial.h>

#define BAUD_RATE 9600
#define START_ID_POS 1
#define END_ID_POS 8

class ConsolePrinter : public Printer
{
    private:
        /**
         * Подсчитывает количество ненулевых элементов в массиве uint8_t
         * @param dataIn Указатель на массив данных
         * @param size Размер массива
         * @return Количество ненулевых элементов
         */
        uint16_t countNonZeroElements(const uint8_t* dataIn, uint8_t size) {
            uint16_t count = 0;
            for (uint16_t i = 0; i < size; i++) {
                if (dataIn[i] != 0) {
                    count++;
                }
            }
            return count;
        } 
    public:
        void init() {
            Serial.begin(BAUD_RATE);
        }

        void println(String message) {
            Serial.println(message);
        }

        void printAgroprobeID(uint8_t* dataIn) {
            Serial.print("Агрозонд ID: ");
            for (uint8_t i = START_ID_POS; i <= END_ID_POS; i++) {
                Serial.print(dataIn[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }

        uint16_t printAgroprobeDataPacket(uint8_t* dataIn, uint8_t maxPacketSize) {
            uint16_t count = countNonZeroElements(dataIn, maxPacketSize);
            Serial.print("Принято байт: ");
            Serial.print(count);
            Serial.print(" | Данные (HEX): ");
            for (uint8_t i = 0; i < count; i++) {
                Serial.print(dataIn[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
            return count;
        }

        void printCRC8(uint8_t crc8, uint8_t calculatedCrc8) {
            Serial.print("Контрольная сумма внутри принятого пакета = ");
            Serial.println(crc8, HEX);
            Serial.print("Рассчитанная контрольная сумма = ");
            Serial.println(calculatedCrc8, HEX);
        }
};
