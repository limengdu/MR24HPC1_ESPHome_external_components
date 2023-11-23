#include "custom_mode_end_button.h"

namespace esphome {
namespace mr24hpc1 {

void CustomModeEndButton::press_action() {
    this->parent_->set_custom_end_mode();
}

}  // namespace mr24hpc1
}  // namespace esphome
