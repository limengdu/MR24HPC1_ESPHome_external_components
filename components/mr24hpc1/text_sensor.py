import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import CONF_MR24HPC1_ID, mr24hpc1Component

CONF_HEARTBEAT = 'heartbeat'

DEPENDENCIES = ["mr24hpc1"]

# The entity category for read only diagnostic values, for example RSSI, uptime or MAC Address
CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(mr24hpc1Component),
    cv.Optional(CONF_HEARTBEAT): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC, icon="mdi:connection"
    ),
}

async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if heartbeat_config := config.get(CONF_HEARTBEAT):
        sens = await text_sensor.new_text_sensor(heartbeat_config)
        cg.add(mr24hpc1_component.set_heartbeat_state_text_sensor(sens))
    
    