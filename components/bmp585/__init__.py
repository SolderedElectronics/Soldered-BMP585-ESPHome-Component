import esphome.codegen as cg
from esphome.components import i2c

CODEOWNERS = ["@SolderedElectronics"]

bmp585_ns = cg.esphome_ns.namespace("bmp585")
BMP585Component = bmp585_ns.class_("BMP585Component", cg.PollingComponent, i2c.I2CDevice)
