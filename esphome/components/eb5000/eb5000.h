#pragma once


#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace eb5000 {

class EB5000 : public PollingComponent, public uart::UARTDevice {
 public:
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }

  void loop() override;

  void update() override;

  void dump_config() override;

 protected:
  sensor::Sensor *voltage_sensor_;

  enum EB5000ReadState {
    SET_ADDRESS = 0xB4,
    READ_VOLTAGE = 0xB0,
    READ_CURRENT = 0xB1,
    READ_POWER = 0xB2,
    READ_ENERGY = 0xB3,
    DONE = 0x00,
  } read_state_{DONE};

  void write_state_(EB5000ReadState state);

  uint32_t last_read_{0};
};

}  // namespace eb5000
}  // namespace esphome
