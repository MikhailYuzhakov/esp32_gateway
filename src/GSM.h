#define RXD2 16
#define TXD2 17



extern AsyncEventSource events;
/**
 * AT+CFUN=1,1 - перезагрузка
 */
class GSM
{
    private:
        String APN = "internet.beeline.ru";
        String APN_USER = "beeline";
        String APN_PASS = "beeline";


        bool checkModule() {
            //пауза на включение модуля
            long int timeout = 3000;
            long int time = millis();
            while (millis() - time < timeout){}
            // Проверка модуля
            sendATCommand("AT", "OK");
            sendATCommand("AT+CPIN?", "READY");
            sendATCommand("AT+CGATT?", "+CGATT: 1");
            return true;
        }

        bool attachGPRS() {
            // Настройка bearer
            sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK");
            sendATCommand("AT+SAPBR=3,1,\"APN\"," + APN, "OK");
            sendATCommand("AT+SAPBR=3,1,\"USER\"," + APN_USER, "OK");
            sendATCommand("AT+SAPBR=3,1,\"PWD\"," + APN_PASS, "OK");
            return true;
        }

        String sendATCommand(const String& command, const String& expectedResponse = "OK", unsigned long timeout = 2000) {
            // Очистка буфера
            while(Serial2.available()) {
                Serial2.read();
            }

            // Отправка команды
            Serial2.println(command);
            Serial2.flush();
            events.send(">" + command, "data", millis());

            String fullResponse;
            unsigned long startTime = millis();
            bool foundExpected = false;

            while(millis() - startTime < timeout) {
                while(Serial2.available()) {
                    String line = Serial2.readStringUntil('\n');
                    line.trim();

                    if(line.length() > 0) {
                        fullResponse += line + "\n";
                        events.send(line, "data", millis());

                        // Проверка на ожидаемый ответ
                        if(line.indexOf(expectedResponse) >= 0) {
                            foundExpected = true;
                        }
                    }
                }

                // Если найден ожидаемый ответ и буфер пуст - завершаем
                if(foundExpected && !Serial2.available()) {
                    break;
                }
            }

            if(!foundExpected) {
                fullResponse += "TIMEOUT\n";
                events.send("TIMEOUT", "data", millis());
            }

            return fullResponse;
        }

    public:
        bool init() {
            Serial2.begin(9600);
            return checkModule() && attachGPRS();
        }

        /**
         * Ответ должне содержать текущее время сервера.
         */
        String sendPOSTRequest(String data, String url) {
            // Активация bearer
            sendATCommand("AT+SAPBR=1,1", "OK", 5000);

            // Проверка статуса
            sendATCommand("AT+SAPBR=2,1", "+SAPBR");

            // Настройка HTTP
            sendATCommand("AT+HTTPINIT", "OK");
            sendATCommand("AT+HTTPPARA=\"CID\",1", "OK");
            sendATCommand("AT+HTTPPARA=\"URL\"," + url, "OK");
            sendATCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK");

            // Отправка POST данных
            char at_command[50]; // Достаточно большой буфер
            uint16_t data_length = data.length();
            sprintf(at_command, "AT+HTTPDATA=%d,5000", data_length);           
            sendATCommand(at_command, "DOWNLOAD", 10000);
            sendATCommand(data, "OK", 10000);
            sendATCommand("AT+HTTPACTION=1", "+HTTPACTION:"); //POST запрос

            // Чтение ответа
            String response = sendATCommand("AT+HTTPREAD", "OK", 5000);
            events.send(response, "data", millis());
         
            // Завершение сессии
            sendATCommand("AT+HTTPTERM", "OK");
            sendATCommand("AT+SAPBR=0,1", "OK");

            return response;
        }
    };

