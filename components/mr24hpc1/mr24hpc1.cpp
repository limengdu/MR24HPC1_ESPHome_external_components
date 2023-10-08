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

// 计算CRC校验码
static uint8_t get_frame_crc_sum(uint8_t *data, int len)
{
    unsigned int crc_sum = 0;
    for (int i = 0; i < len - 3; i++)
    {
        crc_sum += data[i];
    }
    return crc_sum & 0xff;
}

// 检查校验码是否正确
static int get_frame_check_status(uint8_t *data, int len)
{
    uint8_t crc_sum = get_frame_crc_sum(data, len);
    uint8_t verified = data[len - 3];
    return (verified == crc_sum) ? 1 : 0;
}

// 打印数据帧
static void show_frame_data(uint8_t *data, int len)
{
    printf("[%s] FRAME: %d, ", __FUNCTION__, len);
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", data[i] & 0xff);
    }
    printf("\r\n");
}

// 打印组件的配置数据。dump_config()会以易读的格式将组件的所有配置项打印出来,包括配置的键值对。
void mr24hpc1Component::dump_config() { 
    ESP_LOGCONFIG(TAG, "MR24HPC1:");
#ifdef USE_TEXT_SENSOR
    LOG_TEXT_SENSOR("  ", "HeartbeatTextSensor", this->heartbeat_state_text_sensor_);
    LOG_TEXT_SENSOR(" ", "ProductModelTextSensor", this->product_model_text_sensor_);
    LOG_TEXT_SENSOR(" ", "ProductIDTextSensor", this->product_id_text_sensor_);
    LOG_TEXT_SENSOR(" ", "HardwareModelTextSensor", this->hardware_model_text_sensor_);
    LOG_TEXT_SENSOR(" ", "FirwareVerisonTextSensor", this->firware_version_text_sensor_);
#endif
}

// 初始化函数
void mr24hpc1Component::setup() {
    s_power_on_status = 0;
    sg_init_flag = true;
    ESP_LOGCONFIG(TAG, "uart_settings is 115200");
    this->check_uart_settings(115200);
}

// 组件回调函数,它会在每次循环被调用
void mr24hpc1Component::update() {
    if (!sg_init_flag)
        return;
    if (sg_init_flag && (255 != sg_heartbeat_flag))
    {
        this->heartbeat_state_text_sensor_->publish_state(s_heartbeat_str[sg_heartbeat_flag]);
        // sg_heartbeat_flag = 0;
    }
}

// 主循环
void mr24hpc1Component::loop() {
    uint8_t byte;

    // 串口是否有数据
    while (this->available())
    {
        this->read_byte(&byte);
        this->R24_split_data_frame(byte);  // 拆分数据帧
    }
    if (!s_output_info_switch_flag && sg_start_query_data == CUSTOM_FUNCTION_QUERY_RADAR_OUITPUT_INFORMATION_SWITCH)
    {
        // 检查底层开放参数的按钮是否开启，如果开启
        // this->get_radar_output_information_switch();
        sg_start_query_data++;
    }
    if ((s_output_info_switch_flag == OUTPUT_SWTICH_OFF) && (sg_start_query_data <= sg_start_query_data_max) && (sg_start_query_data >= STANDARD_FUNCTION_QUERY_PRODUCT_MODE))
    {
        // 未开启底层开放参数的基础数据刷新
        switch (sg_start_query_data)
        {
            case STANDARD_FUNCTION_QUERY_PRODUCT_MODE:
                if (strlen(this->c_product_mode) > 0)
                {
                    this->product_model_text_sensor_->publish_state(this->c_product_mode);  // 发布产品型号
                }
                else
                {
                    this->get_product_mode();  // 查询产品型号
                }
                break;
            case STANDARD_FUNCTION_QUERY_PRODUCT_ID:
                if (strlen(this->c_product_id) > 0)
                {
                    this->product_id_text_sensor_->publish_state(this->c_product_id);  // 发布产品ID
                }
                else
                {
                    this->get_product_id();  // 查询产品ID
                }
                break;
            case STANDARD_FUNCTION_QUERY_FIRMWARE_VERDION:
                if (strlen(this->c_firmware_version) > 0)
                {
                    this->firware_version_text_sensor_->publish_state(this->c_firmware_version);  // 发布固件版本号
                }
                else
                {
                    this->get_firmware_version();  // 查询估计版本号
                }

                break;
            case STANDARD_FUNCTION_QUERY_HARDWARE_MODE:
                if (strlen(this->c_hardware_model) > 0)
                {
                    this->hardware_model_text_sensor_->publish_state(this->c_hardware_model);  // 发布硬件型号
                }
                else
                {
                    this->get_hardware_model();  // 查询硬件型号
                }
                break;
        }
        sg_start_query_data++;
    }
}

// 拆分数据帧
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

// 解析产品信息相关的数据帧
void mr24hpc1Component::R24_frame_parse_product_Information(uint8_t *data)
{
    uint8_t product_len = 0;
    if (data[FRAME_COMMAND_WORD_INDEX] == 0xA1)
    {
        product_len = data[FRAME_COMMAND_WORD_INDEX + 1] * 256 + data[FRAME_COMMAND_WORD_INDEX + 2];
        if (product_len < PRODUCT_BUF_MAX_SIZE)
        {
            memset(this->c_product_mode, 0, PRODUCT_BUF_MAX_SIZE);
            memcpy(this->c_product_mode, &data[FRAME_DATA_INDEX], product_len);
            ESP_LOGD(TAG, "Reply: get product_mode :%s", this->c_product_mode);
            this->product_model_text_sensor_->publish_state(this->c_product_mode);
        }
        else
        {
            ESP_LOGD(TAG, "Reply: get product_mode length too long!");
        }
    }
    else if (data[FRAME_COMMAND_WORD_INDEX] == 0xA2)
    {
        product_len = data[FRAME_COMMAND_WORD_INDEX + 1] * 256 + data[FRAME_COMMAND_WORD_INDEX + 2];
        if (product_len < PRODUCT_BUF_MAX_SIZE)
        {
            memset(this->c_product_id, 0, PRODUCT_BUF_MAX_SIZE);
            memcpy(this->c_product_id, &data[FRAME_DATA_INDEX], product_len);
            this->product_id_text_sensor_->publish_state(this->c_product_id);
            ESP_LOGD(TAG, "Reply: get productId :%s", this->c_product_id);
        }
        else
        {
            ESP_LOGD(TAG, "Reply: get productId length too long!");
        }
    }
    else if (data[FRAME_COMMAND_WORD_INDEX] == 0xA3)
    {
        product_len = data[FRAME_COMMAND_WORD_INDEX + 1] * 256 + data[FRAME_COMMAND_WORD_INDEX + 2];
        if (product_len < PRODUCT_BUF_MAX_SIZE)
        {
            memset(this->c_hardware_model, 0, PRODUCT_BUF_MAX_SIZE);
            memcpy(this->c_hardware_model, &data[FRAME_DATA_INDEX], product_len);
            this->hardware_model_text_sensor_->publish_state(this->c_hardware_model);
            ESP_LOGD(TAG, "Reply: get hardware_model :%s", this->c_hardware_model);
        }
        else
        {
            ESP_LOGD(TAG, "Reply: get hardwareModel length too long!");
        }
    }
    else if (data[FRAME_COMMAND_WORD_INDEX] == 0xA4)
    {
        product_len = data[FRAME_COMMAND_WORD_INDEX + 1] * 256 + data[FRAME_COMMAND_WORD_INDEX + 2];
        if (product_len < PRODUCT_BUF_MAX_SIZE)
        {

            memset(this->c_firmware_version, 0, PRODUCT_BUF_MAX_SIZE);
            memcpy(this->c_firmware_version, &data[FRAME_DATA_INDEX], product_len);
            this->firware_version_text_sensor_->publish_state(this->c_firmware_version);
            ESP_LOGD(TAG, "Reply: get firmware_version :%s", this->c_firmware_version);
        }
        else
        {
            ESP_LOGD(TAG, "Reply: get firmwareVersion length too long!");
        }
    }
}

// 解析数据帧
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
            this->R24_frame_parse_product_Information(data);
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

// 发送数据帧
void mr24hpc1Component::send_query(uint8_t *query, size_t string_length)
{
    int i;
    for (i = 0; i < string_length; i++)
    {
        write(query[i]);
    }
    show_frame_data(query, i);
}

// 下发产品型号命令
void mr24hpc1Component::get_product_mode(void)
{
    unsigned char send_data_len = 10;
    unsigned char send_data[10] = {0x53, 0x59, 0x02, 0xA1, 0x00, 0x01, 0x0F, 0x00, 0x54, 0x43};
    send_data[FRAME_DATA_INDEX + 1] = get_frame_crc_sum(send_data, send_data_len);
    this->send_query(send_data, send_data_len);
}

// 下发获得产品ID命令
void mr24hpc1Component::get_product_id(void)
{
    unsigned char send_data_len = 10;
    unsigned char send_data[10] = {0x53, 0x59, 0x02, 0xA2, 0x00, 0x01, 0x0F, 0x00, 0x54, 0x43};
    send_data[FRAME_DATA_INDEX + 1] = get_frame_crc_sum(send_data, send_data_len);
    this->send_query(send_data, send_data_len);
}

// 下发硬件型号命令
void mr24hpc1Component::get_hardware_model(void)
{
    unsigned char send_data_len = 10;
    unsigned char send_data[10] = {0x53, 0x59, 0x02, 0xA3, 0x00, 0x01, 0x0F, 0x00, 0x54, 0x43};
    send_data[FRAME_DATA_INDEX + 1] = get_frame_crc_sum(send_data, send_data_len);
    this->send_query(send_data, send_data_len);
}

// 下发软件版本命令
void mr24hpc1Component::get_firmware_version(void)
{
    unsigned char send_data_len = 10;
    unsigned char send_data[10] = {0x53, 0x59, 0x02, 0xA4, 0x00, 0x01, 0x0F, 0x00, 0x54, 0x43};
    send_data[FRAME_DATA_INDEX + 1] = get_frame_crc_sum(send_data, send_data_len);
    this->send_query(send_data, send_data_len);
}

}  // namespace empty_text_sensor
}  // namespace esphome