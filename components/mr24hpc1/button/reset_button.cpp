#include "reset_button.h"

namespace esphome {
namespace mr24hpc1 {

void ResetButton::press_action() {
    uint8_t resetArray[] = {0x53, 0x59, 0x01, 0x02, 0x00, 0x01, 0x0F, 0xBF, 0x54, 0x43};
    this->parent_->send_query(resetArray, sizeof(resetArray));
    bool check_dev_inf_sign = true;
}

}  // namespace mr24hpc1
}  // namespace esphome