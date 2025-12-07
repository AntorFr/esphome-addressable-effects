import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.light.types import AddressableLightEffect
from esphome.components.light.effects import register_addressable_effect

from esphome.const import (
    CONF_NAME,
    CONF_RED,
    CONF_GREEN,
    CONF_BLUE,
    CONF_WHITE,
)

CONF_COLOR = "color"

CONF_STARS_PROBABILITY = "stars_probability"
CONF_CHRISTMASS_BIT_SIZE = "bit_size"
CONF_CHRISTMASS_BLANK_SIZE = "blank_size"

# TwinkleFox configuration
CONF_TWINKLE_SPEED = "twinkle_speed"
CONF_TWINKLE_DENSITY = "twinkle_density"
CONF_COOL_LIKE_INCANDESCENT = "cool_like_incandescent"
CONF_AUTO_BACKGROUND = "auto_background"
CONF_PALETTE = "palette"

# ColorTwinkles configuration
CONF_STARTING_BRIGHTNESS = "starting_brightness"
CONF_FADE_IN_SPEED = "fade_in_speed"
CONF_FADE_OUT_SPEED = "fade_out_speed"
CONF_DENSITY = "density"
CONF_COLOR_TWINKLES_PALETTE = "palette"

light_ns = cg.esphome_ns.namespace("light")
AddressableStarsEffect = light_ns.class_("AddressableStarsEffect", AddressableLightEffect)

ColorStruct = cg.esphome_ns.struct("Color")
AddressableColorStarsEffectColor = light_ns.struct("AddressableColorStarsEffectColor")

AddressableChristmasEffect = light_ns.class_("AddressableChristmasEffect", AddressableLightEffect)
AddressableTwinkleFoxEffect = light_ns.class_("AddressableTwinkleFoxEffect", AddressableLightEffect)
AddressableColorTwinklesEffect = light_ns.class_("AddressableColorTwinklesEffect", AddressableLightEffect)

# TwinkleFox palette enum
TwinkleFoxPaletteType = light_ns.enum("TwinkleFoxPaletteType")
TWINKLEFOX_PALETTES = {
    "party_colors": TwinkleFoxPaletteType.PALETTE_PARTY_COLORS,
    "ocean_colors": TwinkleFoxPaletteType.PALETTE_OCEAN_COLORS,
    "lava_colors": TwinkleFoxPaletteType.PALETTE_LAVA_COLORS,
    "forest_colors": TwinkleFoxPaletteType.PALETTE_FOREST_COLORS,
    "rainbow_colors": TwinkleFoxPaletteType.PALETTE_RAINBOW_COLORS,
    "snow_colors": TwinkleFoxPaletteType.PALETTE_SNOW_COLORS,
    "holly_colors": TwinkleFoxPaletteType.PALETTE_HOLLY_COLORS,
    "ice_colors": TwinkleFoxPaletteType.PALETTE_ICE_COLORS,
    "fairy_light": TwinkleFoxPaletteType.PALETTE_FAIRY_LIGHT,
    "retro_c9": TwinkleFoxPaletteType.PALETTE_RETRO_C9,
}

# ColorTwinkles palette enum
ColorTwinklesPaletteType = light_ns.enum("ColorTwinklesPaletteType")
COLOR_TWINKLES_PALETTES = {
    "cloud_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_CLOUD_COLORS,
    "rainbow_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_RAINBOW_COLORS,
    "snow_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_SNOW_COLORS,
    "incandescent": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_INCANDESCENT,
    "party_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_PARTY_COLORS,
    "ocean_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_OCEAN_COLORS,
    "forest_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_FOREST_COLORS,
    "lava_colors": ColorTwinklesPaletteType.COLOR_TWINKLES_PALETTE_LAVA_COLORS,
}


CONFIG_SCHEMA = cv.All(cv.Schema({}))

@register_addressable_effect(
    "addressable_stars",
    AddressableStarsEffect,
    "Stars",
    {
        cv.Optional(CONF_STARS_PROBABILITY, default="10%"): cv.percentage,
        cv.Optional(
            CONF_COLOR, default={CONF_RED: 0.0,CONF_GREEN: 0.0, CONF_BLUE:0.0},
        ): cv.Schema(
            {
                cv.Optional(CONF_RED, default=0): cv.percentage,
                cv.Optional(CONF_GREEN, default=0): cv.percentage,
                cv.Optional(CONF_BLUE, default=0): cv.percentage,
                cv.Optional(CONF_WHITE, default=0): cv.percentage,
            }
           ),
    },
)
async def addressable_stars_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_stars_probability(config[CONF_STARS_PROBABILITY]))
    color_conf = config[CONF_COLOR]
    color = cg.StructInitializer(
                AddressableColorStarsEffectColor,
                ("r", int(round(color_conf[CONF_RED] * 255))),
                ("g", int(round(color_conf[CONF_GREEN] * 255))),
                ("b", int(round(color_conf[CONF_BLUE] * 255))),
                ("w", int(round(color_conf[CONF_WHITE] * 255))),
            )
    cg.add(var.set_color(color))
    return var

@register_addressable_effect(
    "addressable_christmas",
    AddressableChristmasEffect,
    "Christmas",
    {
        cv.Optional(CONF_CHRISTMASS_BIT_SIZE, default="1"): cv.uint8_t,
        cv.Optional(CONF_CHRISTMASS_BLANK_SIZE, default="0"): cv.uint8_t,
    },

)
async def addressable_christmas_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_bit_size(config[CONF_CHRISTMASS_BIT_SIZE]))
    cg.add(var.set_blank_size(config[CONF_CHRISTMASS_BLANK_SIZE]))
    return var

@register_addressable_effect(
    "addressable_twinklefox",
    AddressableTwinkleFoxEffect,
    "TwinkleFox",
    {
        cv.Optional(CONF_TWINKLE_SPEED, default=4): cv.int_range(min=1, max=8),
        cv.Optional(CONF_TWINKLE_DENSITY, default=5): cv.int_range(min=1, max=8),
        cv.Optional(CONF_COOL_LIKE_INCANDESCENT, default=True): cv.boolean,
        cv.Optional(CONF_AUTO_BACKGROUND, default=False): cv.boolean,
        cv.Optional(CONF_PALETTE, default="party_colors"): cv.enum(TWINKLEFOX_PALETTES, lower=True),
        cv.Optional(
            CONF_COLOR, default={CONF_RED: 0.0, CONF_GREEN: 0.0, CONF_BLUE: 0.0},
        ): cv.Schema(
            {
                cv.Optional(CONF_RED, default=0): cv.percentage,
                cv.Optional(CONF_GREEN, default=0): cv.percentage,
                cv.Optional(CONF_BLUE, default=0): cv.percentage,
            }
        ),
    },
)
async def addressable_twinklefox_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_twinkle_speed(config[CONF_TWINKLE_SPEED]))
    cg.add(var.set_twinkle_density(config[CONF_TWINKLE_DENSITY]))
    cg.add(var.set_cool_like_incandescent(config[CONF_COOL_LIKE_INCANDESCENT]))
    cg.add(var.set_auto_background(config[CONF_AUTO_BACKGROUND]))
    cg.add(var.set_palette(config[CONF_PALETTE]))
    color_conf = config[CONF_COLOR]
    r = int(round(color_conf[CONF_RED] * 255))
    g = int(round(color_conf[CONF_GREEN] * 255))
    b = int(round(color_conf[CONF_BLUE] * 255))
    cg.add(var.set_background_color(cg.RawExpression(f"Color({r}, {g}, {b})")))
    return var


@register_addressable_effect(
    "addressable_color_twinkles",
    AddressableColorTwinklesEffect,
    "Color Twinkles",
    {
        cv.Optional(CONF_STARTING_BRIGHTNESS, default=64): cv.int_range(min=1, max=255),
        cv.Optional(CONF_FADE_IN_SPEED, default=32): cv.int_range(min=1, max=255),
        cv.Optional(CONF_FADE_OUT_SPEED, default=20): cv.int_range(min=1, max=255),
        cv.Optional(CONF_DENSITY, default=255): cv.int_range(min=1, max=255),
        cv.Optional(CONF_COLOR_TWINKLES_PALETTE, default="rainbow_colors"): cv.enum(COLOR_TWINKLES_PALETTES, lower=True),
    },
)
async def addressable_color_twinkles_effect_to_code(config, effect_id):
    var = cg.new_Pvariable(effect_id, config[CONF_NAME])
    cg.add(var.set_starting_brightness(config[CONF_STARTING_BRIGHTNESS]))
    cg.add(var.set_fade_in_speed(config[CONF_FADE_IN_SPEED]))
    cg.add(var.set_fade_out_speed(config[CONF_FADE_OUT_SPEED]))
    cg.add(var.set_density(config[CONF_DENSITY]))
    cg.add(var.set_palette(config[CONF_COLOR_TWINKLES_PALETTE]))
    return var
