import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_INTERRUPT_PIN,
    CONF_PRESSURE,
    CONF_TEMPERATURE,
    DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_HECTOPASCAL,
)
from esphome import pins

from . import BMP585Component, bmp585_ns

CODEOWNERS = ["@SolderedElectronics"]
DEPENDENCIES = ["i2c"]

CONF_TEMPERATURE_OVERSAMPLING = "temperature_oversampling"
CONF_PRESSURE_OVERSAMPLING = "pressure_oversampling"
CONF_ODR = "odr"
CONF_IIR_TEMPERATURE = "iir_temperature"
CONF_IIR_PRESSURE = "iir_pressure"

BMP585Oversampling = bmp585_ns.enum("BMP585Oversampling")
OVERSAMPLING_OPTIONS = {
    "1X": BMP585Oversampling.OVERSAMPLING_1X,
    "2X": BMP585Oversampling.OVERSAMPLING_2X,
    "4X": BMP585Oversampling.OVERSAMPLING_4X,
    "8X": BMP585Oversampling.OVERSAMPLING_8X,
    "16X": BMP585Oversampling.OVERSAMPLING_16X,
    "32X": BMP585Oversampling.OVERSAMPLING_32X,
    "64X": BMP585Oversampling.OVERSAMPLING_64X,
    "128X": BMP585Oversampling.OVERSAMPLING_128X,
}

BMP585ODR = bmp585_ns.enum("BMP585ODR")
ODR_OPTIONS = {
    "240HZ": BMP585ODR.ODR_240_HZ,
    "218.5HZ": BMP585ODR.ODR_218_5_HZ,
    "199.1HZ": BMP585ODR.ODR_199_1_HZ,
    "179.2HZ": BMP585ODR.ODR_179_2_HZ,
    "160HZ": BMP585ODR.ODR_160_HZ,
    "149.3HZ": BMP585ODR.ODR_149_3_HZ,
    "140HZ": BMP585ODR.ODR_140_HZ,
    "129.8HZ": BMP585ODR.ODR_129_8_HZ,
    "120HZ": BMP585ODR.ODR_120_HZ,
    "110.1HZ": BMP585ODR.ODR_110_1_HZ,
    "100.2HZ": BMP585ODR.ODR_100_2_HZ,
    "89.6HZ": BMP585ODR.ODR_89_6_HZ,
    "80HZ": BMP585ODR.ODR_80_HZ,
    "70HZ": BMP585ODR.ODR_70_HZ,
    "60HZ": BMP585ODR.ODR_60_HZ,
    "50HZ": BMP585ODR.ODR_50_HZ,
    "45HZ": BMP585ODR.ODR_45_HZ,
    "40HZ": BMP585ODR.ODR_40_HZ,
    "35HZ": BMP585ODR.ODR_35_HZ,
    "30HZ": BMP585ODR.ODR_30_HZ,
    "25HZ": BMP585ODR.ODR_25_HZ,
    "20HZ": BMP585ODR.ODR_20_HZ,
    "15HZ": BMP585ODR.ODR_15_HZ,
    "10HZ": BMP585ODR.ODR_10_HZ,
    "5HZ": BMP585ODR.ODR_05_HZ,
    "4HZ": BMP585ODR.ODR_04_HZ,
    "3HZ": BMP585ODR.ODR_03_HZ,
    "2HZ": BMP585ODR.ODR_02_HZ,
    "1HZ": BMP585ODR.ODR_01_HZ,
    "0.5HZ": BMP585ODR.ODR_0_5_HZ,
    "0.25HZ": BMP585ODR.ODR_0_250_HZ,
    "0.125HZ": BMP585ODR.ODR_0_125_HZ,
}

BMP585IIRFilter = bmp585_ns.enum("BMP585IIRFilter")
IIR_FILTER_OPTIONS = {
    "BYPASS": BMP585IIRFilter.IIR_FILTER_BYPASS,
    "COEFF_1": BMP585IIRFilter.IIR_FILTER_COEFF_1,
    "COEFF_3": BMP585IIRFilter.IIR_FILTER_COEFF_3,
    "COEFF_7": BMP585IIRFilter.IIR_FILTER_COEFF_7,
    "COEFF_15": BMP585IIRFilter.IIR_FILTER_COEFF_15,
    "COEFF_31": BMP585IIRFilter.IIR_FILTER_COEFF_31,
    "COEFF_63": BMP585IIRFilter.IIR_FILTER_COEFF_63,
    "COEFF_127": BMP585IIRFilter.IIR_FILTER_COEFF_127,
}

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BMP585Component),
            cv.Optional(CONF_PRESSURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_HECTOPASCAL,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_ATMOSPHERIC_PRESSURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE_OVERSAMPLING, default="64X"): cv.enum(OVERSAMPLING_OPTIONS, upper=True),
            cv.Optional(CONF_PRESSURE_OVERSAMPLING, default="4X"): cv.enum(OVERSAMPLING_OPTIONS, upper=True),
            cv.Optional(CONF_ODR, default="50HZ"): cv.enum(ODR_OPTIONS, upper=True),
            cv.Optional(CONF_IIR_TEMPERATURE, default="BYPASS"): cv.enum(IIR_FILTER_OPTIONS, upper=True),
            cv.Optional(CONF_IIR_PRESSURE, default="BYPASS"): cv.enum(IIR_FILTER_OPTIONS, upper=True),
            cv.Optional(CONF_INTERRUPT_PIN): pins.internal_gpio_input_pin_schema,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x47))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_temperature_oversampling(config[CONF_TEMPERATURE_OVERSAMPLING]))
    cg.add(var.set_pressure_oversampling(config[CONF_PRESSURE_OVERSAMPLING]))
    cg.add(var.set_odr(config[CONF_ODR]))
    cg.add(var.set_iir_temperature(config[CONF_IIR_TEMPERATURE]))
    cg.add(var.set_iir_pressure(config[CONF_IIR_PRESSURE]))

    if interrupt_pin_config := config.get(CONF_INTERRUPT_PIN):
        cg.add(var.set_interrupt_pin(await cg.gpio_pin_expression(interrupt_pin_config)))

    if conf := config.get(CONF_PRESSURE):
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_pressure_sensor(sens))

    if conf := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(conf)
        cg.add(var.set_temperature_sensor(sens))
