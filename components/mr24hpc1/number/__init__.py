import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_MR24HPC1_ID, mr24hpc1Component, mr24hpc1_ns

SensitivityNumber = mr24hpc1_ns.class_("SensitivityNumber", number.Number)

CONF_SENSITIVE = "sensitivity"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(mr24hpc1Component),
        cv.Optional(CONF_SENSITIVE): number.number_schema(
            SensitivityNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:archive-check-outline",
        ),
    }
)


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if sensitivity_config := config.get(CONF_SENSITIVE):
        n = await number.new_number(
            sensitivity_config, min_value=0, max_value=3, step=1,
        )
        await cg.register_parented(n, config[CONF_MR24HPC1_ID])
        cg.add(mr24hpc1_component.set_sensitivity_number(n))
