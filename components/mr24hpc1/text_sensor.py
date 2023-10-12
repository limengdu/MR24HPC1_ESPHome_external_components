import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_MOTION,
    DEVICE_CLASS_MOVING,
)
from . import CONF_MR24HPC1_ID, mr24hpc1Component

CONF_HEARTBEAT = 'heartbeat'
CONF_PRODUCTMODEL = 'productmodel'
CONF_PRODUCTID = 'productid'
CONF_HARDWAREMODEL = 'hardwaremodel'
CONF_FIRWAREVERSION = 'hardwareversion'


AUTO_LOAD = ["mr24hpc1"]

# The entity category for read only diagnostic values, for example RSSI, uptime or MAC Address
CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(mr24hpc1Component),
    cv.Optional(CONF_HEARTBEAT): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:connection"
    ),
    cv.Optional(CONF_PRODUCTMODEL): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:information-outline"
    ),
    cv.Optional(CONF_PRODUCTID): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:information-outline"
    ),
    cv.Optional(CONF_HARDWAREMODEL): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:information-outline"
    ),
    cv.Optional(CONF_FIRWAREVERSION): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:information-outline"
    ),    
}

async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if heartbeat_config := config.get(CONF_HEARTBEAT):
        sens = await text_sensor.new_text_sensor(heartbeat_config)
        cg.add(mr24hpc1_component.set_heartbeat_state_text_sensor(sens))
    if productmodel_config := config.get(CONF_PRODUCTMODEL):
        sens = await text_sensor.new_text_sensor(productmodel_config)
        cg.add(mr24hpc1_component.set_product_model_text_sensor(sens))
    if productid_config := config.get(CONF_PRODUCTID):
        sens = await text_sensor.new_text_sensor(productid_config)
        cg.add(mr24hpc1_component.set_product_id_text_sensor(sens))
    if hardwaremodel_config := config.get(CONF_HARDWAREMODEL):
        sens = await text_sensor.new_text_sensor(hardwaremodel_config)
        cg.add(mr24hpc1_component.set_hardware_model_text_sensor(sens))
    if firwareversion_config := config.get(CONF_FIRWAREVERSION):
        sens = await text_sensor.new_text_sensor(firwareversion_config)
        cg.add(mr24hpc1_component.set_firware_version_text_sensor(sens))
    
    
    