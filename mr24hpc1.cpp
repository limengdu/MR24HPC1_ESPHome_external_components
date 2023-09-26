#include "esphome/core/log.h"
#include "mr24hpc1.h"

namespace esphome {
namespace mr24hpc1 {

static const char *TAG = "mr24hpc1.sensor";

void mr24hpc1Sensor::setup() {

}

void mr24hpc1Sensor::update() {

}

void mr24hpc1Sensor::loop() {

}

void mr24hpc1Sensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}

}  // namespace empty_UART_sensor
}  // namespace esphome
