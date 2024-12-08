#include <fardriver_controller.hpp>
#include <HardwareSerial.h>

const uint8_t request_data[] = {0xAA, 0x13, 0xEC, 0x07, 0x5F, 0x5F, 0x6E, 0x91};
// const uint8_t request_data2[] = {0xAA, 0x13, 0xEC, 0x07, 0x01, 0x1F, 0x6E, 0x91};

volatile size_t received_bytes = 0;

bool message_started = false;

// 6 byte packets
const uint8_t flash_read_addr[55] = {
  0xE2, 0xE8, 0xEE, 0x00, 0x06, 0x0C, 0x12, 
  0xE2, 0xE8, 0xEE, 0x18, 0x1E, 0x24, 0x2A, 
  0xE2, 0xE8, 0xEE, 0x30, 0x5D, 0x63, 0x69, 
  0xE2, 0xE8, 0xEE, 0x7C, 0x82, 0x88, 0x8E, 
  0xE2, 0xE8, 0xEE, 0x94, 0x9A, 0xA0, 0xA6, 
  0xE2, 0xE8, 0xEE, 0xAC, 0xB2, 0xB8, 0xBE, 
  0xE2, 0xE8, 0xEE, 0xC4, 0xCA, 0xD0,
  0xE2, 0xE8, 0xEE, 0xD6, 0xDC, 0xF4, 0xFA
};


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

FardriverData fardriverData;
FardriverMessage message;
FardriverController controller(&fardriverSerial);

bool controller_initialized = false;
uint32_t controller_bytes_received = 0;

unsigned long last_update = micros();

int no_data = 0;
unsigned long last_speed_update = 0;
float last_speed = 0;
unsigned long average_delta = 125000;
uint64_t messages_received;

void setup() {
  Serial.begin(115200);
  Serial2.begin(19200, SERIAL_8N1, 44, 43);
}

void loop() {
  
  size_t available = Serial2.available();
  if (available >= 16) {
    auto has_data = false;
    while (available-- && Serial2.available()) {
        if (Serial2.read() == 0xAA) {
            controller_bytes_received++;
            controller_initialized = true;
            has_data = true;
            break;
        } else {
          controller_bytes_received++;
        }
    }
    
    if (has_data && Serial2.available() >= 15) {
        no_data = 0;
        controller_bytes_received += Serial2.read((uint8_t*)&message.header, sizeof(message.header));
        messages_received++;
        if (message.header.flag = 2) {
            if (message.header.id < 0x37) {
                // Serial.printf("Got: %02X", message.header.id);
                // Serial2.read(fardriverData.GetAddr(addr), 12);
                controller_bytes_received += Serial2.read(message.data, sizeof(message.data));
                controller_bytes_received += Serial2.read(message.crc, sizeof(message.crc));
                if (message.VerifyCRC()) {
                    auto addr = flash_read_addr[message.header.id];
                    // Serial.printf(" Verified, addr: %02X ", addr);
                    if (addr == 0xE2) {
                      last_speed = fardriverData.MeasureSpeed();
                      last_speed_update = micros();
                      // Serial.print(fardriverData.speed);
                    }
                    memcpy(fardriverData.GetAddr(addr), &message.data, 12);
                    // memcpy((void *)&fardriverData + (addr << 1), &message.data, 12);
                    // for (int i = 0; i < 12; i++) {
                    //   fardriverData.GetAddr(addr)[i] = message.data[i];
                    // }

                }
                // Serial.println();
            }
        }
    }
  // } else if (Serial2.available() == 0) {
  } else if (controller_bytes_received >= (4800 - 32)) {
      controller_bytes_received = 0;
      controller.SendRS323Data(0x13, 0x07, 0x01, 0xF1);
      // FardriverData::SendRS323Data(0x13, 0x07, 0x5F, 0x5F);
  }

      // if ( this_1->countToTenRefresh < 10 )
      // {
      //   ++this_1->countToTenRefresh;
      // }
      // else
      // {
      //   if ( this_1->messagesProcessed < 252000 )
      //   {
      //     if ( this_1->alternateUpdateRequest )
      //     {
      //       SendSerial_F0s(this_1->motorview->unk00);
      //       this_1->alternateUpdateRequest = 0;
      //     }
      //     else
      //     {
      //       SendSerial_AA_13_07_01_xx(this_1->motorview->unk00, 0xF1);
      //       this_1->alternateUpdateRequest = 1;
      //     }
      //   }
      //   else
      //   {
      //     this_1->messagesProcessed = 0;
      //     if ( this_1->version0_maybe == 73 )
      //       SendSysCmd(this_1->motorview, 0xFu);
      //     else
      //       SendSysCmd(this_1->motorview, 5u);
      //   }
      //   this_1->countToTenRefresh = 0;
      // }

  
  delay(5);

}