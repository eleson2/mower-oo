#include <Arduino.h>
class SerialSetup
{
private:
    /* data */
public:
    SerialSetup(long baud);
    ~SerialSetup();
};

SerialSetup::SerialSetup(long baud) {
    Serial.begin(baud);
};

SerialSetup::~SerialSetup() {
};
