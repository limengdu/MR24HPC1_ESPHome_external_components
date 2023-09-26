import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, sensor
from esphome.const import CONF_ID, ICON_EMPTY, UNIT_EMPTY

DEPENDENCIES = ['uart']
CODEOWNERS = ["@limengdu"]

# 这行代码创建了一个新的名为 mr24hpc1_ns 的命名空间。
# 该命名空间将作为 mr24hpc1_ns 组件相关的所有类、函数和变量的前缀，确保它们不会与其他组件的名称产生冲突。
mr24hpc1_ns = cg.esphome_ns.namespace('mr24hpc1')
# 这个 MyCustomTextSensor 类将是一个定期轮询的 UART 设备
MyCustomTextSensor = mr24hpc1_ns.class_('MyCustomTextSensor', cg.PollingComponent, TextSensor)
UartReadLineSensor = mr24hpc1_ns.class_('UartReadLineSensor', UARTDevice, Sensor)

# sensor.sensor_schema(UNIT_EMPTY, ICON_EMPTY, 1) 创建了一个基础的传感器模式，设置了单位 (UNIT_EMPTY)，图标 (ICON_EMPTY)，以及数据的小数点位数（1）。
# .extend({ cv.GenerateID(): cv.declare_id(mr24hpc1Sensor), }) 扩展了基础模式，添加了一个必需的 ID。
# cv.GenerateID() 是一个函数，它生成一个唯一的 ID，cv.declare_id(mr24hpc1Sensor) 声明了一个新的 mr24hpc1Sensor ID。
# .extend(cv.polling_component_schema('60s')) '60s' 指定了默认的轮询间隔为 60 秒。
# .extend(uart.UART_DEVICE_SCHEMA) 这允许用户在配置文件中设置 UART 设备的参数，如波特率和接收/发送引脚。
CONFIG_SCHEMA = sensor.sensor_schema(
    {
        cv.GenerateID(): cv.declare_id(MyCustomTextSensor),
    }
)

CONFIG_SCHEMA = sensor.sensor_schema(
    {
        cv.GenerateID(): cv.declare_id(UartReadLineSensor),
    }
)

def to_code(config):
    # 这行代码创建了一个新的 Pvariable（一个代表 C++ 变量的 Python 对象），变量的 ID 是从配置中取出的。
    var = cg.new_Pvariable(config[CONF_ID])
    # 这行代码将新创建的 Pvariable 注册为一个组件，这样 ESPHome 就能在运行时管理它。
    yield cg.register_component(var, config)
    # 这行代码将新创建的 Pvariable 注册为一个传感器。
    yield sensor.register_sensor(var, config)
    # 这行代码将新创建的 Pvariable 注册为一个 UART 设备。
    yield uart.register_uart_device(var, config)
