#include <fardriver_controller.hpp>
#include <HardwareSerial.h>
#include <NeoPixelBus.h>

// pin definitions
#define FARDRIVER_TX 13
#define FARDRIVER_RX 14
#define RGB_LED 48

NeoPixelBus<NeoGrbFeature, NeoSk6812Method> strip(1, RGB_LED);

FardriverSerial fardriverSerial {
    .write = [](const uint8_t * data, uint32_t length) -> uint32_t {
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

    Serial.println("Starting up");

    strip.Begin();
    strip.SetPixelColor(0, RgbColor(0, 10, 0));
    strip.Show();

    Serial.println("LED Started");

    // hardware serial to fardriver controller
    // we need to receive from the TX, and transmit to the RX
    Serial2.begin(19200, SERIAL_8N1, FARDRIVER_TX, FARDRIVER_RX);
}

void loop() {
    if (messages_received == 0) {
        Serial.println("(Re)opening Fardriver connection");
        controller.Open();

        strip.SetPixelColor(0, RgbColor(10, 5, 0));
        strip.Show();
    }

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
    } else {
        strip.SetPixelColor(0, RgbColor(10, 0, 0));
        strip.Show();
    }

    // if no message after a second or 300 messages have been received, trigger the open command again
    if (micros() - last_update > 1000000 || messages_received >= 300) {
        messages_received = 0;
    }
  
    delay(50);
}