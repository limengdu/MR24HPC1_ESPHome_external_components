#include "underlyFuc_switch.h"

namespace esphome {
namespace mr24hpc1 {

void UnderlyOpenFunctionSwitch::write_state(bool state) {
  this->publish_state(state);
  if(state){
    uint8_t underlyswitcharr*[] = {0x53, 0x59, 0x08, 0x00, 0x00, 0x01, 0x01, 0xB6, 0x54, 0x43};
    this->parent_->send_query(underlyswitcharr, sizeof(underlyswitcharr));
    this->parent_->keep_away_text_sensor_->publish_state("");
    this->parent_->motion_status_text_sensor_->publish_state("");
  }
  else{
    uint8_t underlyswitcharr*[] = {0x53, 0x59, 0x08, 0x00, 0x00, 0x01, 0x00, 0xB5, 0x54, 0x43};
    this->parent_->send_query(underlyswitcharr, sizeof(underlyswitcharr));
    this->parent_->custom_spatial_static_value_sensor_->publish_state(0.0f);
    this->parent_->custom_spatial_motion_value_sensor_->publish_state(0.0f);
    this->parent_->custom_motion_distance_sensor_->publish_state(0.0f);
    this->parent_->custom_motion_speed_sensor_->publish_state(0.0f);
  }
}

}  // namespace mr24hpc1
}  // namespace esphome
