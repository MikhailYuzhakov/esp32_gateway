#include <Arduino.h>
#include <LoRa.h>
#include <SPI.h> 

#define NSS 5 //пин Chip select (slave select) lora
#define RST 14 //пин сброса lora
#define DIO0 13 // цифровой пин для lora
#define TXEN 8 //режим на передачу
#define RXEN 26 //режим на прием
#define PACKET_SIZE 100 //размер буффера приемника
#define HANDSHAKE_PROTOCOL 0x8F
#define DATA_PROTOCOL 0x7F

class Transiever
{
    private:
        void txEnable(){
            digitalWrite(TXEN, HIGH);
            digitalWrite(RXEN, LOW);
        }

        void rxEnable(){
            digitalWrite(TXEN, LOW);
            digitalWrite(RXEN, HIGH);
        }
        


        /*
        Конвертирует uint64_t в 8 элементов массива типа uint8_t
        */
        String uint64_to_array(uint64_t value, uint8_t* array) {
            String result = "";
            for (int i = 0; i < 8; ++i) {
                array[i] = (value >> (8 * (7 - i))) & 0xFF;
                result += String(array[i], HEX);
            }
            return result;
        }


    public:
                /*
        Считает контрольную сумму CRC-8 побитовым сдвигом и полиномом 0x1D
        */
        uint8_t crc8_bitwise(const uint8_t *data, size_t length) {
            uint8_t crc = 0x00;  // Начальное значение 
            const uint8_t poly = 0x1D;  // Полином CRC-8 (SAE J1850)

            for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];  // XOR с текущим байтом

                for (uint8_t bit = 0; bit < 8; ++bit) {
                    if (crc & 0x80) {  // Если старший бит = 1
                        crc = (crc << 1) ^ poly;
                    } else {
                        crc <<= 1;
                    }
                }
            }
            return crc;
        }


        String init(uint32_t freq, uint32_t bw, uint8_t sf, uint8_t power)
        {
            String result = "----------------------------------------------------------\n";

            // pinMode(TXEN, OUTPUT);
            // pinMode(RXEN, OUTPUT);

            LoRa.setPins(NSS, RST, DIO0);
            if (LoRa.begin(868E6)) 
                result += "SX1276 is ok.\n";
            else result += "SX1276 is failed.\n";

            LoRa.setSignalBandwidth(bw);
            LoRa.setSpreadingFactor(sf);
            LoRa.setCodingRate4(4);
            LoRa.setTxPower(power);
            // rxEnable();

            result = result + "Параметры SX1276:\nЧастота = " + freq + " Гц\n";
            result = result + "Ширина полосы = " + bw + " Гц\n";
            result = result + "Spreading factor = " + sf + "\n";
            result = result + "Мощность = " + power + " дБм\n";           
            return result;
        }

        /**
         * Принимает пакет и записывает содержимое в переменную dataIn.
         * return сообщения об отладке в строковом типе.
         */
        uint8_t recievePacket(uint8_t* dataIn) {
            uint8_t i = 0;

            int packetSize = LoRa.parsePacket();
            if (packetSize) {
                while (LoRa.available())
                {
                    dataIn[i] = LoRa.read();
                    i++;
                }
                LoRa.receive();
            }
            
            return packetSize;
        }

        boolean isHandshake(uint8_t firstByte) {
            if (firstByte == HANDSHAKE_PROTOCOL) return true;
            return false;
        }

        boolean isDataPacket(uint8_t firstByte) {
            if (firstByte == DATA_PROTOCOL) return true;
            return false;
        }

        String sendPacket(uint8_t* dataOut, uint8_t packetSize) {
            String result = "";
            LoRa.beginPacket();
            LoRa.write(dataOut, packetSize+1);
            LoRa.endPacket();
            result += "Отправлено байт: ";
            result += packetSize;
            result += " | Данные (HEX):";
            for (uint8_t i = 0; i < packetSize; i++)
            {
                result += String(dataOut[i], HEX);
                result += " ";
            }
            return result;
        }
};

