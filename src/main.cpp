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

    // use on-board LED for some status stuff
    strip.Begin();
    strip.SetPixelColor(0, RgbColor(0, 10, 0));
    strip.Show();

    Serial.println("LED Started");

    // hardware serial to fardriver controller
    Serial2.begin(19200, SERIAL_8N1, FARDRIVER_RX, FARDRIVER_TX);
}

void loop() {
    if (messages_received == 0) {
        Serial.println("(Re)opening Fardriver connection");
        controller.Open();
        delay(100);

        strip.SetPixelColor(0, RgbColor(10, 5, 0));
        strip.Show();
    }

    // check for messages from serial
    auto result = controller.Read(&data);
    if (result.error == FardriverController::Success) {
        strip.SetPixelColor(0, RgbColor(0, 10, 0));
        strip.Show();
        last_update = micros();
        messages_received++;
        switch (result.addr) {
            case 0xE2:
            case 0xE8:
            case 0xEE: {
                // could handle more common messages here (these are sent often)
            } break;
            case 0xD6: {
                Serial.printf("MosTemp: %d\n", data.addrD6.MosTemp);
            } break;
            case 0xF4: {
                Serial.printf("motor_temp: %d\n", data.addrF4.motor_temp);            
            } break;
            default:
                break;
        }
    // if CRC doesn't match, there was some error, like EMI
    } else if (result.error == FardriverController::CouldNotVerifyCRC) {
        strip.SetPixelColor(0, RgbColor(10, 0, 0));
        strip.Show();
    }

    // if no message after a second or 300 messages have been received, trigger the open command again
    if (micros() - last_update > 1000000 || messages_received >= 300) {
        messages_received = 0;
    }
}