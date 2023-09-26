// 参考例程：https://github.com/jesserockz/esphome-external-component-examples/blob/master/custom_components/empty_uart_sensor/


#include "esphome/core/log.h"
#include "mr24hpc1.h"

namespace esphome {
namespace mr24hpc1 {   // 对应sensor.py中的命名空间

static const char *TAG = "mr24hpc1";


// 这个函数在设备启动时被调用，通常用于初始化传感器或设备。
void mr24hpc1Sensor::setup() {
    ESP_LOGCONFIG(TAG, "uart_settings is 115200");
    this->check_uart_settings(115200);
    sg_init_flag = true;

    memset(this->c_product_mode, 0, PRODUCT_BUF_MAX_SIZE);      // 初始化或清除内存
    memset(this->c_product_id, 0, PRODUCT_BUF_MAX_SIZE);
    memset(this->c_firmware_version, 0, PRODUCT_BUF_MAX_SIZE);
    memset(this->c_hardware_model, 0, PRODUCT_BUF_MAX_SIZE);
}


// 这个函数在每次设备更新时被调用，通常用于从传感器读取数据或执行定期的操作。
// 在这个函数中编写代码以读取传感器的数据，并通过调用 publish_state() 方法将数据发布出去。
void mr24hpc1Sensor::update() {

}


// 这个函数在设备的主循环中被持续调用，通常用于处理需要持续运行或检查的任务。
// 避免在这个函数中放置会阻塞主线程的代码，例如长时间的 delay()。
void mr24hpc1Sensor::loop() {
    uint8_t byte;
    while (this->available())
    {
        this->read_byte(&byte);
        this->R24_split_data_frame(byte);  // 拆分上报的数据帧
    }
    if (!s_output_info_switch_flag && sg_start_query_data == CUSTOM_FUNCTION_QUERY_RADAR_OUITPUT_INFORMATION_SWITCH)
    {
        this->get_radar_output_information_switch();
        sg_start_query_data++;
    }
    if ((s_output_info_switch_flag == OUTPUT_SWTICH_OFF) && (sg_start_query_data <= sg_start_query_data_max) && (sg_start_query_data >= STANDARD_FUNCTION_QUERY_PRODUCT_MODE))
    {
        switch (sg_start_query_data)
        {
            case STANDARD_FUNCTION_QUERY_PRODUCT_MODE:
                if (strlen(this->c_product_mode) > 0)
                {
                    id(product_mode).publish_state(this->c_product_mode);
                }
                else
                {
                    this->get_product_mode();
                }
                break;
            case STANDARD_FUNCTION_QUERY_PRODUCT_ID:
                if (strlen(this->c_product_id) > 0)
                {
                    id(product_id).publish_state(this->c_product_id);
                }
                else
                {
                    this->get_product_id();
                }
                break;
            case STANDARD_FUNCTION_QUERY_FIRMWARE_VERDION:
                if (strlen(this->c_firmware_version) > 0)
                {
                    id(firmware_version).publish_state(this->c_firmware_version);
                }
                else
                {
                    this->get_firmware_version();
                }
                break;
            case STANDARD_FUNCTION_QUERY_HARDWARE_MODE:
                if (strlen(this->c_hardware_model) > 0)
                {
                    id(hardware_model).publish_state(this->c_hardware_model);
                }
                else
                {
                    this->get_hardware_model();
                }
                break;
            default:
                break;
        }
        sg_start_query_data++;
    }
}


// 这个函数在设备启动时被调用，用于打印设备或传感器的配置信息。
// ESP_LOGCONFIG() 是一个用于记录的宏，第一个参数通常是一个标签，第二个参数是要打印的消息。
void mr24hpc1Sensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}



// 拆分数据帧函数
void mr24hpc1Sensor::mr24hpc1_split_data_frame(uint8_t value)
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


// 解析雷达上报的模组信息，有模组固件、ID、固件版本等
void mr24hpc1Sensor::mr24hpc1_frame_parse_product_Information(uint8_t *data)
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
            id(product_mode).publish_state(this->c_product_mode);
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
            id(product_id).publish_state(this->c_product_id);
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
            id(hardware_model).publish_state(this->c_hardware_model);
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
            id(firmware_version).publish_state(this->c_firmware_version);
            ESP_LOGD(TAG, "Reply: get firmware_version :%s", this->c_firmware_version);
        }
        else
        {
            ESP_LOGD(TAG, "Reply: get firmwareVersion length too long!");
        }
    }
}


// 开放性底层信息输出开关查询
void mr24hpc1Sensor::get_radar_output_information_switch(void)
{
    unsigned char send_data_len = 10;
    unsigned char send_data[10] = {0x53, 0x59, 0x08, 0x80, 0x00, 0x01, 0x0F, 0x00, 0x54, 0x43};
    send_data[FRAME_DATA_INDEX + 1] = get_frame_crc_sum(send_data, send_data_len);
    this->send_query(send_data, send_data_len);
}







}  // namespace mr24hpc1
}  // namespace esphome