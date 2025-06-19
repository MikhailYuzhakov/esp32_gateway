#include <WString.h>

class Printer
{
    private:
        
    public:
        virtual void init() = 0;

        virtual void println(String message) = 0;
};

