/**
 * @file bmp585.cpp
 * @brief Implementation for the bmp585 ESPHome sensor component
 * @author Soldered Electronics
 */

#include "bmp585.h"

#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome {
namespace bmp585 {

static const char *const TAG = "bmp585";

void BMP585Component::setup() {
  ESP_LOGCONFIG(TAG, "Running setup");

  this->dev_.intf = BMP5_I2C_INTF;
  this->dev_.read = BMP585Component::i2c_read;
  this->dev_.write = BMP585Component::i2c_write;
  this->dev_.delay_us = BMP585Component::delay_us;
  this->dev_.intf_ptr = this;

  int8_t status = bmp5_soft_reset(&this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Soft reset failed: %s", BMP585Component::status_string(status));
    this->mark_failed();
    return;
  }

  status = bmp5_init(&this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Initialization failed: %s", BMP585Component::status_string(status));
    this->mark_failed();
    return;
  }

  if (!this->configure_sensor_()) {
    this->mark_failed();
    return;
  }

  status = bmp5_set_power_mode(BMP5_POWERMODE_NORMAL, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Setting normal power mode failed: %s", BMP585Component::status_string(status));
    this->mark_failed();
    return;
  }

  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->setup();
    this->interrupt_pin_->attach_interrupt(&BMP585Component::data_ready_isr, this, gpio::INTERRUPT_RISING_EDGE);

    status =
        bmp5_configure_interrupt(BMP5_PULSED, BMP5_ACTIVE_HIGH, BMP5_INTR_PUSH_PULL, BMP5_INTR_ENABLE, &this->dev_);
    if (status != BMP5_OK) {
      ESP_LOGE(TAG, "Configuring the interrupt pin failed: %s", BMP585Component::status_string(status));
      this->mark_failed();
      return;
    }

    struct bmp5_int_source_select int_source {};
    int_source.drdy_en = BMP5_ENABLE;
    status = bmp5_int_source_select(&int_source, &this->dev_);
    if (status != BMP5_OK) {
      ESP_LOGE(TAG, "Selecting the interrupt source failed: %s", BMP585Component::status_string(status));
      this->mark_failed();
      return;
    }
  }
}

bool BMP585Component::configure_sensor_() {
  bmp5_osr_odr_press_config osr_odr_cfg{};

  int8_t status = bmp5_get_osr_odr_press_config(&osr_odr_cfg, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Reading the OSR/ODR configuration failed: %s", BMP585Component::status_string(status));
    return false;
  }

  osr_odr_cfg.osr_t = this->temperature_oversampling_;
  osr_odr_cfg.osr_p = this->pressure_oversampling_;
  osr_odr_cfg.odr = this->odr_;
  osr_odr_cfg.press_en = BMP5_ENABLE;

  status = bmp5_set_osr_odr_press_config(&osr_odr_cfg, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Writing the OSR/ODR configuration failed: %s", BMP585Component::status_string(status));
    return false;
  }

  bmp5_iir_config iir_cfg{};
  iir_cfg.set_iir_t = this->iir_temperature_;
  iir_cfg.set_iir_p = this->iir_pressure_;
  iir_cfg.shdw_set_iir_t = BMP5_ENABLE;
  iir_cfg.shdw_set_iir_p = BMP5_ENABLE;

  status = bmp5_set_iir_config(&iir_cfg, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGE(TAG, "Writing the IIR configuration failed: %s", BMP585Component::status_string(status));
    return false;
  }

  return true;
}

void BMP585Component::update() {
  if (this->is_failed())
    return;

  if (this->interrupt_pin_ != nullptr) {
    if (!this->data_ready_)
      return;
    this->data_ready_ = false;
  }

  bmp5_osr_odr_press_config osr_odr_cfg{};
  int8_t status = bmp5_get_osr_odr_press_config(&osr_odr_cfg, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGW(TAG, "Reading the OSR/ODR configuration failed: %s", BMP585Component::status_string(status));
    this->status_set_warning();
    return;
  }

  bmp5_sensor_data raw{};
  status = bmp5_get_sensor_data(&raw, &osr_odr_cfg, &this->dev_);
  if (status != BMP5_OK) {
    ESP_LOGW(TAG, "Reading sensor data failed: %s", BMP585Component::status_string(status));
    this->status_set_warning();
    return;
  }

  // Bosch's API returns pressure in Pa; ESPHome barometric pressure sensors
  // conventionally publish hPa.
  const float pressure_hpa = raw.pressure / 100.0f;

  ESP_LOGD(TAG, "Got pressure=%.2f hPa temperature=%.2f degC", pressure_hpa, raw.temperature);

  if (this->pressure_sensor_ != nullptr)
    this->pressure_sensor_->publish_state(pressure_hpa);
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(raw.temperature);

  this->status_clear_warning();
}

BMP5_INTF_RET_TYPE BMP585Component::i2c_read(uint8_t reg_addr, uint8_t *reg_data, uint32_t length, void *intf_ptr) {
  auto *component = static_cast<BMP585Component *>(intf_ptr);
  if (component == nullptr)
    return BMP5_E_NULL_PTR;

  if (component->read_register(reg_addr, reg_data, length) != i2c::ERROR_OK)
    return BMP5_E_COM_FAIL;

  return BMP5_OK;
}

BMP5_INTF_RET_TYPE BMP585Component::i2c_write(uint8_t reg_addr, const uint8_t *reg_data, uint32_t length,
                                              void *intf_ptr) {
  auto *component = static_cast<BMP585Component *>(intf_ptr);
  if (component == nullptr)
    return BMP5_E_NULL_PTR;

  if (component->write_register(reg_addr, reg_data, length) != i2c::ERROR_OK)
    return BMP5_E_COM_FAIL;

  return BMP5_OK;
}

void BMP585Component::delay_us(uint32_t period_us, void *intf_ptr) {
  (void) intf_ptr;
  delay_microseconds_safe(period_us);
}

void IRAM_ATTR BMP585Component::data_ready_isr(BMP585Component *self) { self->data_ready_ = true; }

const char *BMP585Component::status_string(int8_t status) {
  switch (status) {
    case BMP5_OK:
      return "OK";
    case BMP5_E_NULL_PTR:
      return "Null pointer";
    case BMP5_E_COM_FAIL:
      return "Communication failure";
    case BMP5_E_DEV_NOT_FOUND:
      return "Sensor not found";
    case BMP5_E_INVALID_CHIP_ID:
      return "Invalid chip id";
    case BMP5_E_NVM_NOT_READY:
      return "NVM not ready";
    case BMP5_E_POR_SOFTRESET:
      return "Power-on reset/softreset failure";
    case BMP5_E_INVALID_POWERMODE:
      return "Invalid powermode";
    case BMP5_E_INVALID_THRESHOLD:
      return "Invalid threshold";
    case BMP5_E_FIFO_FRAME_EMPTY:
      return "FIFO frame empty";
    case BMP5_E_NVM_INVALID_ADDR:
      return "Invalid NVM address";
    default:
      return "Undefined error code";
  }
}

void BMP585Component::dump_config() {
  ESP_LOGCONFIG(TAG, "BMP585:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with BMP585 failed!");
    return;
  }
  LOG_UPDATE_INTERVAL(this);

  ESP_LOGCONFIG(TAG,
                "  Temperature oversampling: %u\n"
                "  Pressure oversampling: %u\n"
                "  ODR: %u\n"
                "  IIR temperature: %u\n"
                "  IIR pressure: %u",
                (unsigned) this->temperature_oversampling_, (unsigned) this->pressure_oversampling_,
                (unsigned) this->odr_, (unsigned) this->iir_temperature_, (unsigned) this->iir_pressure_);

  LOG_SENSOR("  ", "Pressure", this->pressure_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
}

}  // namespace bmp585
}  // namespace esphome
