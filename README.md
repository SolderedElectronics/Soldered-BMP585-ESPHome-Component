# Soldered BMP585 ESPHome Component

| ![Soldered BMP585 Barometric Pressure Sensor breakout](TODO_PRODUCT_IMAGE_URL) |
| :-----------------------------------------------------------------------------------------------------------------------: |
|                                  [Soldered BMP585 Barometric Pressure Sensor breakout](https://www.solde.red/333189)                                  |

<!-- TODO: product not released yet (SKU 333189), swap the image URL above once the listing is live -->

Breakout board for the Bosch BMP585 barometric pressure sensor, measuring absolute pressure with ±6 Pa relative
accuracy at output data rates up to 240 Hz, with configurable oversampling and an on-chip IIR filter. The board
communicates over I2C only and is part of the
[Qwiic ecosystem](https://soldered.com/collections/qwiic-ecosystem).

External ESPHome component for the Soldered BMP585 breakout board. It is a port of the
[Soldered BMP585 Arduino library](https://github.com/SolderedElectronics/Soldered-BMP585-Arduino-Library) and wraps
the official Bosch BMP5_SensorAPI, which is vendored unmodified in `components/bmp585/bmp5_api.c`, `bmp5_api.h` and
`bmp5_api_defs.h` (renamed only to avoid a filename clash with this component's own `bmp585.h`, the file contents are
untouched other than the two `#include` lines that follow the rename).

## Repository Contents

- **components/** - the ESPHome external component (Python config + C++ implementation)
- **examples/** - example YAML configs showing how to use the component

## Usage

Reference this repo directly from your own ESPHome YAML (no need to clone it locally):

```yaml
external_components:
  - source: github://SolderedElectronics/Soldered-BMP585-ESPHome-Component
    components: [bmp585]

i2c:
  sda: GPIO21
  scl: GPIO22

sensor:
  - platform: bmp585
    address: 0x47
    update_interval: 10s
    pressure:
      name: "BMP585 Pressure"
    temperature:
      name: "BMP585 Temperature"
```

See [`examples/basic.yaml`](examples/basic.yaml) for a full working example.

### Configuration variables

- **address** (*Optional*, int): I2C address of the sensor. Defaults to `0x47`.
- **update_interval** (*Optional*, [Time](https://esphome.io/guides/configuration-types#config-time)): how often the
  values are published. Defaults to `60s`.
- **temperature_oversampling** (*Optional*, string): temperature oversampling, one of `1X`, `2X`, `4X`, `8X`, `16X`,
  `32X`, `64X`, `128X`. Defaults to `64X`.
- **pressure_oversampling** (*Optional*, string): pressure oversampling, same options as above. Defaults to `4X`.
- **odr** (*Optional*, string): output data rate, e.g. `240HZ`, `100.2HZ`, `50HZ`, `10HZ`, `1HZ`, `0.125HZ` (see
  [`sensor.py`](components/bmp585/sensor.py) for the full list). Defaults to `50HZ`.
- **iir_temperature** (*Optional*, string): temperature IIR filter coefficient, one of `BYPASS`, `COEFF_1`,
  `COEFF_3`, `COEFF_7`, `COEFF_15`, `COEFF_31`, `COEFF_63`, `COEFF_127`. Defaults to `BYPASS`.
- **iir_pressure** (*Optional*, string): pressure IIR filter coefficient, same options as above. Defaults to
  `BYPASS`.
- **pressure** (*Optional*): [Sensor](https://esphome.io/components/sensor/) publishing pressure in hPa.
- **temperature** (*Optional*): [Sensor](https://esphome.io/components/sensor/) publishing temperature in °C.
- **interrupt_pin** (*Optional*, [Pin](https://esphome.io/guides/configuration-types#config-pin)): the GPIO the
  sensor's physical interrupt pin is wired to. When set, a new reading is only published once the pin has actually
  fired since the last update, instead of unconditionally polling over I2C every `update_interval`.

### Examples

- [`examples/basic.yaml`](examples/basic.yaml) - reads pressure and temperature once per update interval.
- [`examples/data_ready_interrupt.yaml`](examples/data_ready_interrupt.yaml) - uses the sensor's physical interrupt
  pin instead of unconditionally polling over I2C every update interval.

The examples are written for an ESP32-S3 board (`esp32-s3-devkitc-1`) with the sensor on `GPIO8`/`GPIO9`.

### Development

Run the following before committing to auto-format the component against ESPHome's own style:

```sh
pip install clang-format==13.0.1
find components \( -name "*.cpp" -o -name "*.h" \) -not -name "bmp5_api*" | xargs clang-format -i
```

The vendored Bosch BMP5_SensorAPI (`bmp5_api*`) is kept unmodified and is excluded from formatting. CI runs the
same check on every push/PR via `.github/workflows/format_check.yml` and fails on unformatted code.
`.github/workflows/build.yml` compiles every YAML under `examples/` on every push/PR.

### Hardware design

You can find hardware design for this board in the _Soldered BMP585 Barometric Pressure Sensor breakout_ hardware
repository.

### Documentation

Access library documentation [here](https://docs.soldered.com/).

### About Soldered

<img src="https://raw.githubusercontent.com/SolderedElectronics/Soldered-Generic-Arduino-Library/dev/extras/Soldered-logo-color.png" alt="soldered-logo" width="500"/>

At Soldered, we design and manufacture a wide selection of electronic products to help you turn your ideas into acts and bring you one step closer to your final project. Our products are intented for makers and crafted in-house by our experienced team in Osijek, Croatia. We believe that sharing is a crucial element for improvement and innovation, and we work hard to stay connected with all our makers regardless of their skill or experience level. Therefore, all our products are open-source. Finally, we always have your back. If you face any problem concerning either your shopping experience or your electronics project, our team will help you deal with it, offering efficient customer service and cost-free technical support anytime. Some of those might be useful for you:

- [Web Store](https://www.soldered.com/shop)
- [Tutorials & Projects](https://soldered.com/learn)
- [Documentation](https://docs.soldered.com)

### Open-source license

Soldered invests vast amounts of time into hardware & software for these products, which are all open-source. Please support future development by buying one of our products.

Check license details in the LICENSE file. Long story short, use these open-source files for any purpose you want to, as long as you apply the same open-source licence to it and disclose the original source. No warranty - all designs in this repository are distributed in the hope that they will be useful, but without any warranty. They are provided "AS IS", therefore without warranty of any kind, either expressed or implied. The entire quality and performance of what you do with the contents of this repository are your responsibility. In no event, Soldered (TAVU) will be liable for your damages, losses, including any general, special, incidental or consequential damage arising out of the use or inability to use the contents of this repository.

## Have fun!

And thank you from your fellow makers at Soldered Electronics.
