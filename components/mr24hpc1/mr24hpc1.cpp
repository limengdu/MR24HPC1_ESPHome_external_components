#include "esphome/core/log.h"
#include "mr24hpc1.h"

namespace esphome {
namespace mr24hpc1_text_sensor {

static const char *TAG = "mr24hpc1_text_sensor.text_sensor";

void mr24hpc1TextSensor::setup() {
  
}

void mr24hpc1TextSensor::dump_config() { 
    ESP_LOGCONFIG(TAG, "Empty text sensor");
}

}  // namespace empty_text_sensor
}  // namespace esphome