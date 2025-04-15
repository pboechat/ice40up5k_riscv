# Demo

This is a simple firmware to demonstrate the capabilities of the RISC-V SoC running on the iCE40UP5K-B-EVN FPGA board.

It showcases the use of peripherals such as an LCD display, I2C bus, and serial communication.

It is a good starting point for exploring and developing on the platform.

## Booting up

1. **Connect a Serial Adapter**:  
   Connect a serial adapter to the TX/RX pins of the RISC-V SoC.  
   Use a terminal program (e.g., `minicom`, `Putty`) to connect to the corresponding serial port at a baud rate of 115200.

2. **Open the Terminal**:  
   Once connected, you should see a welcome message displayed in the terminal. This indicates that the firmware has booted up successfully.

3. **Connect the LCD (Optional)**:  
   If you have an ILI9341 TFT display connected to the SPI1 port pins, the firmware will display several screens showcasing the SoC's graphics capabilities. This demonstrates the ability to interface with an LCD for rendering graphics.

4. **Display Image on LCD (Optional)**:  
   - You can store a raw `rgb565`-encoded image (240x320 pixels) at flash location `0x200000`.
   - The firmware will automatically blit this image to the LCD screen.  
   - A helper script to encode the image is available in the `scripts` directory.

5. **I2C Device Detection**:  
   - If you have an I2C device connected to the bus at the expected address, you will see `"."` printed in the terminal.  
   - If no device is detected, `"x"` will be printed instead.