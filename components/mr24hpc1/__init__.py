import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart

DEPENDENCIES = ["uart"]
# 是相关代码库的代码所有者
CODEOWNERS = ["@limengdu"]
# 当前的组件或者平台可以在同一个配置文件中被多次配置或者定义。
MULTI_CONF = True

# 这行代码创建了一个新的名为 ld2410_ns 的命名空间。
# 该命名空间将作为 ld2410_ns 组件相关的所有类、函数和变量的前缀，确保它们不会与其他组件的名称产生冲突。
mr24hpc1_ns = cg.esphome_ns.namespace("mr24hpc1")
# 这个 LD2410Component 类将是一个定期轮询的 UART 设备
mr24hpc1Component = mr24hpc1_ns.class_("mr24hpc1Component", cg.Component, uart.UARTDevice)

CONF_MR24HPC1_ID = "mr24hpc1_id"






    