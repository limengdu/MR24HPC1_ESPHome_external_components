import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_MR24HPC1_ID, mr24hpc1Component, mr24hpc1_ns

SceneModeSelect = mr24hpc1_ns.class_("SceneModeSelect", select.Select)
UnmanTimeSelect = mr24hpc1_ns.class_("UnmanTimeSelect", select.Select)

CONF_SCENEMODE = "scene_mode"
CONF_UNMANTIME = "unman_time"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(mr24hpc1Component),
    cv.Optional(CONF_SCENEMODE): select.select_schema(
        SceneModeSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:hoop-house",
    ),
    cv.Optional(CONF_UNMANTIME): select.select_schema(
        UnmanTimeSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:timeline-clock",
    )
}


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if scenemode_config := config.get(CONF_SCENEMODE):
        s = await select.new_select(
            scenemode_config, options=["None", "Living Room", "Bedroom", "Washroom", "Area Detection"]
        )
        await cg.register_parented(s, config[CONF_MR24HPC1_ID])
        cg.add(mr24hpc1_component.set_scene_mode_select(s))
    if unmantime_config := config.get(CONF_UNMANTIME):
        s = await select.new_select(
            unmantime_config, options=["None", "10s", "30s", "1min", "2min", "5min", "10min", "30min", "60min"]
        )
        await cg.register_parented(s, config[CONF_MR24HPC1_ID])
        cg.add(mr24hpc1_component.set_unman_time_select(s))
