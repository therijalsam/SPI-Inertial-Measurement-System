# SPI Inertial Measurement System (LSM9DS1)

## Overview
This project implements a real-time inertial measurement system using the **LSM9DS1 sensor over SPI (Serial Peripheral Interface)**. The system reads temperature, gyroscope, and accelerometer data directly from the sensor registers and outputs the processed values through both a hardware display and a serial interface.

The project focuses on low-level SPI communication, sensor data acquisition, and multi-axis motion tracking in embedded systems.

---

## Features
- SPI communication with LSM9DS1 sensor  
- Real-time temperature sensing  
- 3-axis gyroscope data acquisition (X, Y, Z)  
- 3-axis accelerometer data acquisition (X, Y, Z)  
- UART output for monitoring sensor data  
- 7-segment display integration for temperature visualization  
- Direct register-level hardware interaction  

---

## System Architecture
- **Sensor:** LSM9DS1 (Accelerometer + Gyroscope + Temperature)  
- **Processor:** ARM-based embedded system (Zynq / Blackboard)  
- **Communication Protocols:**  
  - SPI for sensor communication  
  - UART for serial output  
- **Output Interfaces:**  
  - 7-segment display  
  - Serial terminal  

---

## SPI Register Mapping

The system interacts with the following key registers:

- `WHO_AM_I (0x0F)` → Device identification  
- `CTRL_REG1_G (0x10)` → Gyroscope configuration  
- `OUT_TEMP (0x15, 0x16)` → Temperature data  
- `OUT_X_G (0x18, 0x19)` → Gyroscope X-axis  
- `OUT_Y_G (0x1A, 0x1B)` → Gyroscope Y-axis  
- `OUT_Z_G (0x1C, 0x1D)` → Gyroscope Z-axis  
- `CTRL_REG6_XL (0x20)` → Accelerometer configuration  
- `OUT_X_XL (0x28, 0x29)` → Accelerometer X-axis  
- `OUT_Y_XL (0x2A, 0x2B)` → Accelerometer Y-axis  
- `OUT_Z_XL (0x2C, 0x2D)` → Accelerometer Z-axis  

---

## Data Processing
- Uses 2’s complement representation  
- Combines MSB and LSB for full resolution  
- Converts raw values into physical units (temperature, angular rate, acceleration)  

---

## Functionality

### Temperature Monitoring
- Reads temperature data from SPI registers  
- Displays temperature on a 7-segment display when triggered  
- Sends formatted temperature data to UART  

### Gyroscope Data
- Reads angular velocity for X, Y, Z axes  
- Outputs real-time motion data via UART  

### Accelerometer Data
- Reads linear acceleration for X, Y, Z axes  
- Enables motion tracking and orientation detection  

---

## How It Works

1. Initialize SPI interface  
2. Configure LSM9DS1 sensor registers  
3. Verify device communication using `WHO_AM_I`  

### Continuous Operation
- Read temperature data  
- Read gyroscope data (X, Y, Z)  
- Read accelerometer data (X, Y, Z)  
- Process raw sensor values  
- Send formatted data to UART  
- Display temperature on 7-segment display when triggered  

---

## Key Concepts Demonstrated
- SPI protocol implementation at register level  
- Multi-sensor data acquisition  
- Bitwise operations and data reconstruction  
- Real-time embedded data processing  
- Hardware-software integration  

---

## Performance Considerations
- Efficient SPI transactions with minimal overhead  
- Low-latency sensor data acquisition  
- Deterministic execution for real-time systems  
- Minimal memory usage  

---

## Potential Improvements
- Add sensor calibration for higher accuracy  
- Implement filtering (e.g., Kalman or low-pass filter)  
- Use interrupts instead of polling  
- Integrate visualization dashboard  
- Extend to full IMU-based motion tracking system  



