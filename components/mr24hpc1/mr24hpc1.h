#pragma once

#include "esphome/core/component.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace mr24hpc1_text_sensor {

class MR24HPC1_TextSensor : public text_sensor::TextSensor, public Component {
 public:
  void setup() override;
  void dump_config() override;
};

}  // namespace empty_text_sensor
}  // namespace esphome