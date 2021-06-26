## Weather station
The purpose of this project was to create a weather station. The project is build on the STM32F411E Discovery development board. The BME280 sensor is used to acquire information 
about temperature, pressure and humidity. Communication between the microcontroller and the sensor is achieved using SPI. To show data to the user, the command line interface
based on UART was implemented. Buffering data using the ring buffer structure was implemented so that user can get both latest measurements and average measurements from last 3 
minutes. 

## Authors
* **Jan Kuliga**: [kuliga](https://github.com/kuliga)
* **Michał Tomacha**: [tomacha](https://github.com/tomacha)
* **Krzysztof Bera**: [krzysiubera](https://github.com/krzysiubera)

## Components used in the project:
- STM32F411E Discovery board
<img src="images/stm32f4.png" />
- BME280 sensor
<img src="images/bme280.png" />
- PL2303 USB-UART converter
<img src="images/usb_uart_conv.png" />

## Physical connections between the board and the sensor
- 3V <-> VCC
- GND <-> GND
- PB12 <-> SCL
- PC12 <-> SDA
- PA8 <-> CSB
- PC11 <-> SD0

## Physical connections between the board and the converter
- 3V <-> VCCIO (not necessary)
- GND <-> GND
- PB6 <-> RXD
- PB7 <-> TXD

## A look on the CLI
To get measurements, a user needs to open a terminal program and open connection. Available commands are:
- getting a list of all available commands
```
\help
```
<img src="images/help.png" />
- getting latest measurements
```
\get hum
\get press
\get temp
\get all
```
<img src="images/get_hum.png" />
<img src="images/get_press.png" />
<img src="images/get_temp.png" />
<img src="images/get_all.png" />
