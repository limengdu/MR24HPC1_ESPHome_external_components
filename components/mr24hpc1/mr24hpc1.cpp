#include "esphome/core/log.h"
#include "mr24hpc1.h"

namespace esphome {
namespace mr24hpc1_text_sensor {

static const char *TAG = "mr24hpc1_text_sensor.text_sensor";

void MR24HPC1_TextSensor::setup() {
  
}

void MR24HPC1_TextSensor::dump_config() { 
    ESP_LOGCONFIG(TAG, "Empty text sensor");
}

}  // namespace empty_text_sensor
}  // namespace esphome