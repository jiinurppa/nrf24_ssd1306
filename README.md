## nrf24_ssd1306

C++ Raspberry Pi Pico template project for wireless communication.

### Functionality
1. Boot to programming mode (optional with `enable_programming_mode`, Picoprobe can't wake Pico from dormant power mode)
2. Set dormant power mode
3. Wake from dormant power mode when wireless module sets `IRQ` pin `LOW`
4. Read message from wireless module
5. Respond to `!PING` message (if sent) with `!PONG`
6. Show received message on OLED display

![Functionality demonstration](images/demo.gif)

### Components
* Raspberry Pi Pico [documentation](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)
* SSD1306 128x32 OLED display [documentation](https://artofcircuits.com/product/ssd1306-white-0-91-128x32-oled-display-i2c-interface)
* nRF24L01+ wireless module [documentation](https://lastminuteengineers.com/nrf24l01-arduino-wireless-communication/)

### Wiring
![Wiring schematic](/images/wiring_schematic.svg)
`Picoprobe GP5 / UART1 RX` connection is only required if `enable_debug_logging = true`

### Setup
* pico-sdk [repo](https://github.com/raspberrypi/pico-sdk)
* pico-extras (for `pico/sleep.h`) [repo](https://github.com/raspberrypi/pico-extras)

RF24 library is added as a submodule [repo](https://github.com/nRF24/RF24) & [documentation](https://nrf24.github.io/RF24/md_docs_pico_sdk.html)

Files that might require path tweaks:
* .vscode\launch.json
* .vscode\c_cpp_properties.json

### Annoyances
1. USB power banks will turn off power after a few seconds (current draw too low?)
2. Picoprobe can't wake Pico from dormant power mode (reset required for code upload)
3. Debugging in dormant power mode keeps breaking at main