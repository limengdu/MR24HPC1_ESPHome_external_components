#include "esphome/core/log.h"
#include "mr24hpc1.h"

#include <utility>
#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

namespace esphome {
namespace mr24hpc1 {

static const char *TAG = "mr24hpc1";

static uint8_t get_frame_crc_sum(uint8_t *data, int len)
{
    unsigned int crc_sum = 0;
    for (int i = 0; i < len - 3; i++)
    {
        crc_sum += data[i];
    }
    return crc_sum & 0xff;
}

static int get_frame_check_status(uint8_t *data, int len)
{
    uint8_t crc_sum = get_frame_crc_sum(data, len);
    uint8_t verified = data[len - 3];
    return (verified == crc_sum) ? 1 : 0;
}

static void show_frame_data(uint8_t *data, int len)
{
    printf("[%s] FRAME: %d, ", __FUNCTION__, len);
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", data[i] & 0xff);
    }
    printf("\r\n");
}

void mr24hpc1Component::setup() {
    s_power_on_status = 0;
    sg_init_flag = true;
    ESP_LOGCONFIG(TAG, "uart_settings is 115200");
    this->check_uart_settings(115200);
}

void mr24hpc1Component::loop() {
    uint8_t byte;
    while (this->available())
    {
        this->read_byte(&byte);
        this->R24_split_data_frame(byte);
    }
}

void mr24hpc1Component::update() {
    if (!sg_init_flag)
        return;
    if (sg_init_flag && (255 != sg_heartbeat_flag))
    {
        this->heartbeat_state_text_sensor_->publish_state(s_heartbeat_str[sg_heartbeat_flag]);
        // sg_heartbeat_flag = 0;
    }
}

void mr24hpc1Component::dump_config() { 
    
}

void mr24hpc1Component::R24_split_data_frame(uint8_t value)
{
    switch (sg_recv_data_state)
    {
    case FRAME_IDLE:
        if (FRAME_HEADER1_VALUE == value)
        {
            sg_recv_data_state = FRAME_HEADER2;
        }
        break;
    case FRAME_HEADER2:
        if (FRAME_HEADER2_VALUE == value)
        {
            sg_frame_buf[0] = FRAME_HEADER1_VALUE;
            sg_frame_buf[1] = FRAME_HEADER2_VALUE;
            sg_recv_data_state = FRAME_CTL_WORLD;
        }
        else
        {
            sg_recv_data_state = FRAME_IDLE;
            ESP_LOGD(TAG, "FRAME_IDLE ERROR value:%x", value);
        }
        break;
    case FRAME_CTL_WORLD:
        sg_frame_buf[2] = value;
        sg_recv_data_state = FRAME_CMD_WORLD;
        break;
    case FRAME_CMD_WORLD:
        sg_frame_buf[3] = value;
        sg_recv_data_state = FRAME_DATA_LEN_H;
        break;
    case FRAME_DATA_LEN_H:
        if (value <= 4)
        {
            sg_data_len = value * 256;
            sg_frame_buf[4] = value;
            sg_recv_data_state = FRAME_DATA_LEN_L;
        }
        else
        {
            sg_data_len = 0;
            sg_recv_data_state = FRAME_IDLE;
            ESP_LOGD(TAG, "FRAME_DATA_LEN_H ERROR value:%x", value);
        }
        break;
    case FRAME_DATA_LEN_L:
        sg_data_len += value;
        if (sg_data_len > 32)
        {
            ESP_LOGD(TAG, "len=%d, FRAME_DATA_LEN_L ERROR value:%x", sg_data_len, value);
            sg_data_len = 0;
            sg_recv_data_state = FRAME_IDLE;
        }
        else
        {
            sg_frame_buf[5] = value;
            sg_frame_len = 6;
            sg_recv_data_state = FRAME_DATA_BYTES;
        }
        break;
    case FRAME_DATA_BYTES:
        sg_data_len -= 1;
        sg_frame_buf[sg_frame_len++] = value;
        if (sg_data_len <= 0)
        {
            sg_recv_data_state = FRAME_DATA_CRC;
        }
        break;
    case FRAME_DATA_CRC:
        sg_frame_buf[sg_frame_len++] = value;
        sg_recv_data_state = FRAME_TAIL1;
        break;
    case FRAME_TAIL1:
        if (FRAME_TAIL1_VALUE == value)
        {
            sg_recv_data_state = FRAME_TAIL2;
        }
        else
        {
            sg_recv_data_state = FRAME_IDLE;
            sg_frame_len = 0;
            sg_data_len = 0;
            ESP_LOGD(TAG, "FRAME_TAIL1 ERROR value:%x", value);
        }
        break;
    case FRAME_TAIL2:
        if (FRAME_TAIL2_VALUE == value)
        {
            sg_frame_buf[sg_frame_len++] = FRAME_TAIL1_VALUE;
            sg_frame_buf[sg_frame_len++] = FRAME_TAIL2_VALUE;
            memcpy(sg_frame_prase_buf, sg_frame_buf, sg_frame_len);
            if (get_frame_check_status(sg_frame_prase_buf, sg_frame_len))
            {
                // show_frame_data(sg_frame_prase_buf, sg_frame_len);
                this->R24_parse_data_frame(sg_frame_prase_buf, sg_frame_len);
            }
            else
            {
                ESP_LOGD(TAG, "frame check failer!");
            }
        }
        else
        {
            ESP_LOGD(TAG, "FRAME_TAIL2 ERROR value:%x", value);
        }
        memset(sg_frame_prase_buf, 0, FRAME_BUF_MAX_SIZE);
        memset(sg_frame_buf, 0, FRAME_BUF_MAX_SIZE);
        sg_frame_len = 0;
        sg_data_len = 0;
        sg_recv_data_state = FRAME_IDLE;
        break;
    default:
        sg_recv_data_state = FRAME_IDLE;
    }
}

void mr24hpc1Component::R24_parse_data_frame(uint8_t *data, uint8_t len)
{
    switch (data[FRAME_CONTROL_WORD_INDEX])
    {

    case 0x01:
    {
        if (data[FRAME_COMMAND_WORD_INDEX] == 0x01)
        {
            sg_heartbeat_flag = 1;
            ESP_LOGD(TAG, "Reply: query Heartbeat packet");
        }
        else if (data[FRAME_COMMAND_WORD_INDEX] == 0x02)
        {
            ESP_LOGD(TAG, "Reply: query reset packet");
        }
    }
    break;
    case 0x02:
    {
        // this->R24_frame_parse_product_Information(data);
    }
    break;
    case 0x05:
    {
        // this->R24_frame_parse_work_status(data);
    }
    break;
    case 0x08:
    {
        // this->R24_frame_parse_open_underlying_information(data);
    }
    break;
    case 0x80:
    {
        // this->R24_frame_parse_human_information(data);
    }
    break;
    default:
        ESP_LOGD(TAG, "control world:0x%02X not found", data[FRAME_CONTROL_WORD_INDEX]);
        break;
    }
}

}  // namespace empty_text_sensor
}  // namespace esphome