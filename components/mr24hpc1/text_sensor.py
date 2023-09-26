import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID

CODEOWNERS = ["@limengdu"]

# 这行代码创建了一个新的名为 mr24hpc1_ns 的命名空间。
# 该命名空间将作为 mr24hpc1_ns 组件相关的所有类、函数和变量的前缀，确保它们不会与其他组件的名称产生冲突。
mr24hpc1_text_sensor_ns = cg.esphome_ns.namespace('mr24hpc1_text_sensor')
# 这个 MyCustomTextSensor 类将是一个定期轮询的 UART 设备
mr24hpc1TextSensor = mr24hpc1_text_sensor_ns.class_('mr24hpc1TextSensor', text_sensor.TextSensor, cg.Component)

# sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1) 创建了一个基础的传感器模式，设置了单位 (UNIT_EMPTY)，图标 (ICON_EMPTY)，以及数据的小数点位数（1）。
# .extend({ cv.GenerateID(): cv.declare_id(mr24hpc1Sensor), }) 扩展了基础模式，添加了一个必需的 ID。
# cv.GenerateID() 是一个函数，它生成一个唯一的 ID，cv.declare_id(mr24hpc1Sensor) 声明了一个新的 mr24hpc1Sensor ID。
# .extend(cv.polling_component_schema('60s')) '60s' 指定了默认的轮询间隔为 60 秒。
# .extend(uart.UART_DEVICE_SCHEMA) 这允许用户在配置文件中设置 UART 设备的参数，如波特率和接收/发送引脚。
CONFIG_SCHEMA = text_sensor.TEXT_SENSOR_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(mr24hpc1TextSensor)
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    # 这行代码创建了一个新的 Pvariable（一个代表 C++ 变量的 Python 对象），变量的 ID 是从配置中取出的。
    var = cg.new_Pvariable(config[CONF_ID])
    # 注册了一个文本传感器
    yield text_sensor.register_text_sensor(var, config)
    # 这个生成器负责生成注册组件所需要的 C++ 代码。
    yield cg.register_component(var, config)
    
    