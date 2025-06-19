#include "ESP-FTP-Server-Lib.h"
#include <LittleFS.h>

class FileSystem
{
    private:
        FTPServer ftpSrv; 
    public:
        bool init() {
            if (LittleFS.begin()) {
                return true;
            } else {
                return false;
            }
        }

        String readDirectory(String path) {
            String fileList;
            File root = LittleFS.open(path);
            if (!root) return "Failed to open directory";
            if (!root.isDirectory()) return "Path not a directory";
            File file = root.openNextFile();
            while (file) {
                fileList += "  " + String(file.name()) + " (" + String(file.size()) + " bytes)\n";
                file = root.openNextFile();
            }
            return fileList;
        }

        void buildFileTree(const String &dirPath, int depth, String &output) {
            // Добавляем "/" в начало пути, если его нет
            String absPath = dirPath;
            if (!absPath.startsWith("/")) {
                absPath = "/" + absPath;
            }
        
            File dir = LittleFS.open(absPath.c_str());
            if (!dir || !dir.isDirectory()) {
                output += "[ERROR] Can't open dir: " + absPath + "\n";
                return;
            }
        
            File file = dir.openNextFile();
            while (file) {
                // Добавляем отступы в зависимости от глубины
                for (int i = 0; i < depth; i++) {
                    output += "  ";
                }
        
                String fileName = file.name();
                // Убеждаемся, что имя файла/директории - абсолютный путь
                if (!fileName.startsWith("/")) {
                    fileName = absPath + (absPath.endsWith("/") ? "" : "/") + fileName;
                }
        
                if (file.isDirectory()) {
                    output += "📁 " + fileName + "\n";
                    buildFileTree(fileName, depth + 1, output);  // Передаем абсолютный путь
                } else {
                    output += "📄 " + fileName + " (" + String(file.size()) + " bytes)\n";
                }
        
                file = dir.openNextFile();
            }
            dir.close();
        }

        String getFile(String path) {
            File file = LittleFS.open(path, "r");

            if (!file) {
                return "File not found";
            } else {
                String content = file.readString();
                file.close();
                return content;
            }
        }

        void saveFile(String content, String path) {
            File file = LittleFS.open(path, "w");
            file.print(content);
            file.close();
        }

        String getProbesConfigs(String probesConfigPath) {
            String conf = "[";
            bool firstFile = true; // Флаг для обработки первой записи

            // Нормализация пути - убираем возможные слеши в конце
            if (probesConfigPath.endsWith("/")) {
                probesConfigPath.remove(probesConfigPath.length()-1);
            }

            // Открываем корневой каталог
            File root = LittleFS.open(probesConfigPath);

            // Проверка открытия и что это директория
            if (!root) {
                return String("Error: Failed to open path ") + probesConfigPath;
            }
            if (!root.isDirectory()) {
                root.close();
                return String("Error: Path ") + probesConfigPath + " is not a directory";
            }

            // Перебираем файлы в каталоге
            File file = root.openNextFile();
            while (file) {
                String fileName = file.name();

                // Проверяем условие для файлов конфигурации
                if (fileName.startsWith("id") && fileName.endsWith(".json")) {
                    if (!firstFile) {
                        conf += ", ";
                    }

                    // Читаем содержимое файла
                    String filePath = probesConfigPath + "/" + fileName;
                    String fileContent = getFile(filePath);

                    // Проверяем валидность JSON
                    if (fileContent != "null" && fileContent.length() > 0) {
                        conf += fileContent;
                        firstFile = false;
                    }
                }
                file.close();
                file = root.openNextFile();
            }

            root.close();
            conf += "]";
            
            return conf;
        }

        bool ftpServerInit(String ftpLogin, String ftpPass) {
            ftpSrv.addUser(ftpLogin, ftpPass);
            ftpSrv.addFilesystem("LittleFS", &LittleFS);
            return ftpSrv.begin();
        }

        void ftpHandle() {
            ftpSrv.handle();
        }
};