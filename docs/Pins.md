## Pins Connection Guide

The fully connected setup should look like this:

![full](camera_manuca.jpg)

The SPI and I2C pin configurations for the MANUCA DK board are shown here:

![spi](spi.png)
![i2c](i2c.png)

They are located on the board as shown:  

![manuca_pins](manuca_dk_pins.jpg)

These must be connected to the SPI and I2C interface of the camera as shown here:

![camera_pins](camera_pins.jpg)

Concretely, you should connect:

- Camera CS to SPI2 CS (pin 4)
- Camera MOSI to SPI2 MOSI (pin 8)
- Camera MISO to SPI2 MISO (pin 10)
- Camera SCK to SPI2 SCK (pin 6)
- Camera GND to I2C2 GND (pin 5)
- Camera VCC to I2C2 5V_PER (pin 8)
- Camera SDA to I2C2 DATA (pin 2)
- Camera SCL to I2C2 CLK (pin 1)


