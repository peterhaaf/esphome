#include "eb5000.h"
#include "esphome/core/log.h"

namespace esphome {
namespace eb5000 {

static const char *TAG = "eb5000";

void EB5000::loop() {
  const uint32_t now = millis();
  if (now - this->last_read_ > 500 && this->available() < 7) {
    while (this->available())
      this->read();
    this->last_read_ = now;
  }
  ESP_LOGD(TAG, "EB5000 welcome");
  // PZEM004T packet size is 7 byte
  while (this->available() >= 7) {
    auto resp = *this->read_array<7>();
    // packet format:
    // 0: packet type
    // 1-5: data
    // 6: checksum (sum of other bytes)
    // see https://github.com/olehs/PZEM004T
    ESP_LOGD(TAG,"array %s",resp);
    uint8_t sum = 0;
    for (int i = 0; i < 6; i++)
      sum += resp[i];

    if (sum != resp[6]) {
      ESP_LOGD(TAG, "EB5000 invalid checksum! 0x%02X != 0x%02X", sum, resp[6]);
      continue;
    }

    switch (resp[0]) {
      case 0xA4: {  // Set Module Address Response
        this->write_state_(READ_VOLTAGE);
        break;
      }
      case 0xA0: {  // Voltage Response
        uint16_t int_voltage = (uint16_t(resp[1]) << 8) | (uint16_t(resp[2]) << 0);
        float voltage = int_voltage + (resp[3] / 10.0f);
        if (this->voltage_sensor_ != nullptr)
          this->voltage_sensor_->publish_state(voltage);
        ESP_LOGD(TAG, "Got Voltage %.1f V", voltage);
        break;
      }



      default:
        break;
    }

    this->last_read_ = now;
  }
}
void EB5000::update() { this->write_state_(READ_VOLTAGE); }
void EB5000::write_state_(EB5000::EB5000ReadState state) {
  if (state == DONE) {
    this->read_state_ = state;
    return;
  }
  std::array<uint8_t, 7> data{};
  data[0] = state;
  data[1] = 192;
  data[2] = 168;
  data[3] = 1;
  data[4] = 1;
  data[5] = 0;
  data[6] = 0;
  for (int i = 0; i < 6; i++)
    data[6] += data[i];

  this->write_array(data);
  this->read_state_ = state;
}
void EB5000::dump_config() {
  ESP_LOGCONFIG(TAG, "EB5000:");
  LOG_SENSOR("", "Voltage", this->voltage_sensor_);
}  

}  // namespace eb5000
}  // namespace esphome
