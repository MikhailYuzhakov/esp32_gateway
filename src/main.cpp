#include "config/HotspotConfig.h"
#include "FileSystem.h"
#include "Network.h"
#include "RealTimeClock.h"
#include "printer/ConsolePrinter.h"
#include "config/ProbeConfigBuilder.h"
#include "WebServer.h"
#include "GSM.h"
#include "Transiever.h"
#include "Sensors.h"

#define SERVER_URL "http://infotis-official.fvds.ru/api/v1/r"
#define DATETIME_REQUEST "{\"t\":\"42\",\"m\":{\"2\":21}}"
#define MAX_PACKET_SIZE 100
#define PROTOCOL_POS 0

ConsolePrinter printer;
FileSystem fileSystem;
Network network;
HotspotConfig hotspotConfig;
ProbeConfigBuilder probesConfigBuilder;
WebServer webServer;
GSM gsm;
Transiever transiever;
RealTimeClock* rtcPtr; // Глобальный указатель на объект RealTimeClock
Sensors sensors;

uint8_t counter = 0;
String result = "";
char data[64]; 
uint8_t dataIn[MAX_PACKET_SIZE];
uint8_t dataOut[MAX_PACKET_SIZE];

String convertIDToString() {
    String hexString = "";
    
    // Элементы с 1 по 8 (индексы 1..8 в dataIn)
    for (int i = 1; i <= 8; i++) {
        if (dataIn[i] < 0x10) {
            hexString += "0";
        }
        hexString += String(dataIn[i], HEX);
    }
    
    hexString.toUpperCase();
    return hexString;
}

void clearInputOutputBuffer() {
  //очищаем буффер
  for (uint8_t i = 0; i < MAX_PACKET_SIZE; i++) {
    dataIn[i] = 0;
    dataOut[i] = 0;
  }
}

void setup() {
  printer.init(); //инициализация последовательного порта

    //Инициализация фалйовой системы и чтения конфига БС, если файловая система не завелась, то дальше выполнять код смысла нет
  if (!fileSystem.init())
    printer.println("\nFile system initialization failed!");
  else {
    printer.println("\nFile system initialization successful");
    
    // String treeOutput;
    // fileSystem.buildFileTree("/", 0, treeOutput);  // Начинаем с корня
    // printer.println("File Tree:\n" + treeOutput);

    hotspotConfig.set(fileSystem.getFile(CONFIG_FILE_PATH)); //читаем конфиг из файла и записываем в объект networkConfig
    printer.println(hotspotConfig.get());

    //задаем настройки wi-fi подключения
    network.setSSID(hotspotConfig.getSsid());
    network.setPassword(hotspotConfig.getPass());
    network.setApSSID(hotspotConfig.getApSsid());
    network.setApPass(hotspotConfig.getApPass());

    // Создание объекта RealTimeClock и сохранение указателя на него
    rtcPtr = new RealTimeClock(hotspotConfig.getTimezone());
    printer.println("Internal RTC was initialized successfully!");

    if (!network.initWiFi()) {
      printer.println("Network initialization failed!");
      // тут запускаем SIM800L и RTC ESP32
    } else {
      printer.println("Network initialization successful");

      // запускаем NTP клиент, если Wi-Fi сеть есть
      if (!rtcPtr->initNTP(hotspotConfig.getTimezone())) {
        printer.println("NTP client initialization failed!");
      } else {
        printer.println("NTP client initialization success!\n");
        NTP.tick();
        printer.println(NTP.toString());
      }
    }

      //Инициализация FTP сервера (независимо от режима работы модуля wi-fi: станция или точка доступа)
      if (!fileSystem.ftpServerInit(hotspotConfig.getFtpLogin(), hotspotConfig.getFtpPass()))
        printer.println("Ftp server initialization failed!");
      else
        printer.println("Ftp server initialization successful!");
  
      //Создаем объект для сбора конфигураций зондов и читаем весь конфиг
      String probesConfig = fileSystem.getProbesConfigs(PROBES_CONFIGS_PATH);
      String result = probesConfigBuilder.buildConfigs(probesConfig);
      printer.println(result);

      printer.println(
          probesConfigBuilder.printProbesConfigs()
      );
  
      //запуск веб-сервера
      webServer.init(&fileSystem);
      printer.println("Web server was started successfully");

      // этот блок будет в том месте, где не получилось запустить Wi-Fi
      if (gsm.init()) 
        printer.println("SIM800L init ok.");

      // при инициализации сразу получаем дату и время от сервера
      String currentDateTime = gsm.sendPOSTRequest(DATETIME_REQUEST, SERVER_URL);

      // записываем актуальные дату и время в часы внутри ESP32, отправляем повторный запрос, если время не получили
      while (!rtcPtr->syncInternalRTCDateTime(currentDateTime))
        currentDateTime = gsm.sendPOSTRequest(DATETIME_REQUEST, SERVER_URL);
      
      printer.printDateTime(rtcPtr->getDateTime());

      printer.println(transiever.init(hotspotConfig.getFrequency(), 
                                      hotspotConfig.getBandwidth(), 
                                      hotspotConfig.getSpreadingFactor(), 
                                      hotspotConfig.getPower()));
      
  }
}

void loop() {
  fileSystem.ftpHandle();
  uint8_t packetSize = transiever.recievePacket(dataIn); 
  
  if (transiever.isHandshake(dataIn[PROTOCOL_POS])) { // если первый байт сообщения 0x8F - рукопожатие
    printer.printAgroprobeDataPacket(dataIn, packetSize, false);
    uint8_t calculatedCrc8 = transiever.crc8_bitwise(dataIn, packetSize - 1);

    printer.printCRC8(dataIn[packetSize - 1], calculatedCrc8);

    if (calculatedCrc8 == dataIn[packetSize - 1]) {
      uint8_t ptr = 0;
      // записываем в массив ответа протокол и ID зонда c [0;8] позиции
      for (uint8_t i = 0; i < packetSize - 1; i++) {
        dataOut[ptr] = dataIn[i];
        Serial.print(dataOut[ptr], HEX);
        ptr++;
      }
      Serial.println();
      
      uint8_t* dateTime = rtcPtr->getDateTime();

      for (uint8_t i = 0; i < 6; i++) {
        dataOut[ptr] = dateTime[i];
        Serial.print(dataOut[ptr], HEX);
        ptr++;
      }
      Serial.println();
      
      uint8_t dataOutCrc8 = transiever.crc8_bitwise(dataOut, ptr-1);
      dataOut[ptr] = dataOutCrc8;
      Serial.println("Ответ с контрольной суммой: ");
      printer.printAgroprobeDataPacket(dataOut, packetSize, true);
      Serial.println(transiever.sendPacket(dataOut, ptr));
    } else {
      // если crc8 не равна той, что пришла в пакете
    }
  } else if (transiever.isDataPacket(dataIn[PROTOCOL_POS])) { // если первый байт сообщения 0x7F - пакет данных
      printer.printAgroprobeDataPacket(dataIn, packetSize, false);
      uint8_t calculatedCrc8 = transiever.crc8_bitwise(dataIn, packetSize - 2);

      if (calculatedCrc8 == dataIn[packetSize - 1]) {
        char buffer[200];
        uint8_t ptr = 0;
        uint8_t* dateTime = rtcPtr->getDateTime();

        String sID = convertIDToString();
        String pressure = sensors.getPressure(dataIn);
        String airBmpTemperature = sensors.getBmpAirTemperature(dataIn);
        String airHtuTemperature = sensors.getHtuAirTemperature(dataIn);
        String airHumidity = sensors.getAirHumidiry(dataIn);
    
        // записываем в массив ответа протокол и ID зонда c [0;8] позиции
        for (uint8_t i = 0; i < 9; i++) 
          dataOut[ptr++] = dataIn[i];

        for (uint8_t i = 0; i < 6; i++) 
          dataOut[ptr++] = dateTime[i];

        uint8_t dataOutCrc8 = transiever.crc8_bitwise(dataOut, ptr-1);
        dataOut[ptr] = dataOutCrc8;

        Serial.println(transiever.sendPacket(dataOut, ptr));

        // Безопасное формирование JSON
        snprintf(buffer, sizeof(buffer), 
                "{\"t\":\"%s\", \"m\":{\"2\":\"%s\", \"3\":\"%s\", \"4\":\"%s\"}}", 
                sID.c_str(), 
                airBmpTemperature.c_str(), 
                pressure.c_str(), 
                airHumidity.c_str());
        
        Serial.print("Отправлен пакет данных на сервер: ");
        printer.println(buffer);
        String currentDateTime = gsm.sendPOSTRequest(buffer, SERVER_URL);

        rtcPtr->syncInternalRTCDateTime(currentDateTime);
        printer.printDateTime(rtcPtr->getDateTime());
        Serial.println();

      } else {
        // если crc8 не равна той, что пришла в пакете
      }
  }

  clearInputOutputBuffer();
}
