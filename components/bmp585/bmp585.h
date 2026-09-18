/**
 * @file bmp585.h
 * @brief Public API for the bmp585 ESPHome sensor component
 * @author Soldered Electronics
 */

#pragma once

#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include "esphome/core/gpio.h"

#include "bmp5_api.h"
#include "bmp5_api_defs.h"

namespace esphome {
namespace bmp585 {

// The enumerator names below deliberately avoid the exact spelling of the
// Bosch API's own BMP5_* macros (bmp5_api_defs.h) -- since those are
// preprocessor #defines, a member named e.g. BMP5_ODR_50_HZ would also be
// textually substituted, producing invalid C++.

/// Oversampling rate, used for both temperature and pressure.
enum BMP585Oversampling : uint8_t {
  OVERSAMPLING_1X = BMP5_OVERSAMPLING_1X,
  OVERSAMPLING_2X = BMP5_OVERSAMPLING_2X,
  OVERSAMPLING_4X = BMP5_OVERSAMPLING_4X,
  OVERSAMPLING_8X = BMP5_OVERSAMPLING_8X,
  OVERSAMPLING_16X = BMP5_OVERSAMPLING_16X,
  OVERSAMPLING_32X = BMP5_OVERSAMPLING_32X,
  OVERSAMPLING_64X = BMP5_OVERSAMPLING_64X,
  OVERSAMPLING_128X = BMP5_OVERSAMPLING_128X,
};

/// Output data rate.
enum BMP585ODR : uint8_t {
  ODR_240_HZ = BMP5_ODR_240_HZ,
  ODR_218_5_HZ = BMP5_ODR_218_5_HZ,
  ODR_199_1_HZ = BMP5_ODR_199_1_HZ,
  ODR_179_2_HZ = BMP5_ODR_179_2_HZ,
  ODR_160_HZ = BMP5_ODR_160_HZ,
  ODR_149_3_HZ = BMP5_ODR_149_3_HZ,
  ODR_140_HZ = BMP5_ODR_140_HZ,
  ODR_129_8_HZ = BMP5_ODR_129_8_HZ,
  ODR_120_HZ = BMP5_ODR_120_HZ,
  ODR_110_1_HZ = BMP5_ODR_110_1_HZ,
  ODR_100_2_HZ = BMP5_ODR_100_2_HZ,
  ODR_89_6_HZ = BMP5_ODR_89_6_HZ,
  ODR_80_HZ = BMP5_ODR_80_HZ,
  ODR_70_HZ = BMP5_ODR_70_HZ,
  ODR_60_HZ = BMP5_ODR_60_HZ,
  ODR_50_HZ = BMP5_ODR_50_HZ,
  ODR_45_HZ = BMP5_ODR_45_HZ,
  ODR_40_HZ = BMP5_ODR_40_HZ,
  ODR_35_HZ = BMP5_ODR_35_HZ,
  ODR_30_HZ = BMP5_ODR_30_HZ,
  ODR_25_HZ = BMP5_ODR_25_HZ,
  ODR_20_HZ = BMP5_ODR_20_HZ,
  ODR_15_HZ = BMP5_ODR_15_HZ,
  ODR_10_HZ = BMP5_ODR_10_HZ,
  ODR_05_HZ = BMP5_ODR_05_HZ,
  ODR_04_HZ = BMP5_ODR_04_HZ,
  ODR_03_HZ = BMP5_ODR_03_HZ,
  ODR_02_HZ = BMP5_ODR_02_HZ,
  ODR_01_HZ = BMP5_ODR_01_HZ,
  ODR_0_5_HZ = BMP5_ODR_0_5_HZ,
  ODR_0_250_HZ = BMP5_ODR_0_250_HZ,
  ODR_0_125_HZ = BMP5_ODR_0_125_HZ,
};

/// IIR filter coefficient, used for both temperature and pressure.
enum BMP585IIRFilter : uint8_t {
  IIR_FILTER_BYPASS = BMP5_IIR_FILTER_BYPASS,
  IIR_FILTER_COEFF_1 = BMP5_IIR_FILTER_COEFF_1,
  IIR_FILTER_COEFF_3 = BMP5_IIR_FILTER_COEFF_3,
  IIR_FILTER_COEFF_7 = BMP5_IIR_FILTER_COEFF_7,
  IIR_FILTER_COEFF_15 = BMP5_IIR_FILTER_COEFF_15,
  IIR_FILTER_COEFF_31 = BMP5_IIR_FILTER_COEFF_31,
  IIR_FILTER_COEFF_63 = BMP5_IIR_FILTER_COEFF_63,
  IIR_FILTER_COEFF_127 = BMP5_IIR_FILTER_COEFF_127,
};

/**
 * @brief Soldered BMP585 barometric pressure sensor breakout board.
 *
 * Thin ESPHome wrapper around the official Bosch BMP5_SensorAPI, which is vendored
 * unmodified in bmp5_api.c, bmp5_api.h and bmp5_api_defs.h (renamed only to avoid a
 * filename clash with this component's own bmp585.h).
 */
class BMP585Component : public PollingComponent, public i2c::I2CDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_pressure_sensor(sensor::Sensor *sensor) { this->pressure_sensor_ = sensor; }
  void set_temperature_sensor(sensor::Sensor *sensor) { this->temperature_sensor_ = sensor; }

  void set_temperature_oversampling(BMP585Oversampling osr) { this->temperature_oversampling_ = osr; }
  void set_pressure_oversampling(BMP585Oversampling osr) { this->pressure_oversampling_ = osr; }
  void set_odr(BMP585ODR odr) { this->odr_ = odr; }
  void set_iir_temperature(BMP585IIRFilter iir) { this->iir_temperature_ = iir; }
  void set_iir_pressure(BMP585IIRFilter iir) { this->iir_pressure_ = iir; }

  /// Sets the pin wired to the sensor's physical interrupt pin. When set, a new
  /// reading is only published once the pin has actually fired since the last
  /// update, instead of unconditionally polling over I2C every update_interval.
  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }

 protected:
  /// Read callback handed to the Bosch API.
  static BMP5_INTF_RET_TYPE i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr);
  /// Write callback handed to the Bosch API.
  static BMP5_INTF_RET_TYPE i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length, void *intf_ptr);
  /// Delay callback handed to the Bosch API.
  static void delay_us(uint32_t period_us, void *intf_ptr);
  /// Returns a brief text description of a Bosch API status code.
  static const char *status_string(int8_t status);
  /// ISR attached to interrupt_pin_, just raises data_ready_ -- the actual I2C
  /// read happens in update(), never in interrupt context.
  static void IRAM_ATTR data_ready_isr(BMP585Component *self);

  /// Applies the oversampling, ODR and IIR configuration to the sensor.
  bool configure_sensor_();

  sensor::Sensor *pressure_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};

  BMP585Oversampling temperature_oversampling_{OVERSAMPLING_64X};
  BMP585Oversampling pressure_oversampling_{OVERSAMPLING_4X};
  BMP585ODR odr_{ODR_50_HZ};
  BMP585IIRFilter iir_temperature_{IIR_FILTER_BYPASS};
  BMP585IIRFilter iir_pressure_{IIR_FILTER_BYPASS};

  InternalGPIOPin *interrupt_pin_{nullptr};
  volatile bool data_ready_{false};

  ::bmp5_dev dev_{};
};

}  // namespace bmp585
}  // namespace esphome
