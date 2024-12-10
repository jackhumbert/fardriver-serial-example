#include <fardriver_controller.hpp>
#include <HardwareSerial.h>

#define FARDRIVER_RX 44
#define FARDRIVER_TX 43

FardriverSerial fardriverSerial {
    .write = [](const uint8_t * data, uint32_t length) -> uint32_t {
		// return length;
        // print_data("<<", data, length);
        return Serial2.write(data, length);
    },
    .read = [](uint8_t * data, uint32_t length) -> uint32_t {
        return Serial2.read(data, length);
	},
    .available = [](void) -> uint32_t {
        return Serial2.available();
    }
};

FardriverData data;
FardriverController controller(&fardriverSerial);

unsigned long last_update = micros();
uint64_t messages_received;

void setup() {
    // USB serial to computer for monitoring
    Serial.begin(115200);

    // hardware serial to fardriver controller
    Serial2.begin(19200, SERIAL_8N1, FARDRIVER_RX, FARDRIVER_TX);
}

void loop() {
    if (messages_received == 0)
        controller.Open();

    // check for messages from serial
    auto result = controller.Read(&data);
    if (result.error == FardriverController::Success) {
        last_update = micros();
        messages_received++;
        switch (result.addr) {
            case 0xE2:
            case 0xE8:
            case 0xEE: {
                // could handle more common messages like this
            } break;
            default:
                break;
        }
    }

    // if no message after a second or 300 messages have been received, trigger the open command again
    if (micros() - last_update > 1000000 || messages_received >= 300) {
        messages_received = 0;
    }
  
    delay(5);
}