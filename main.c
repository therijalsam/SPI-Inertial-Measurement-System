#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sleep.h"

// UART defines
#define UART1_CR        *((uint32_t *)0xE0001000)
#define UART1_MR        *((uint32_t *)0xE0001004)
#define UART1_BAUDGEN   *((uint32_t *)0xE0001018)
#define UART1_SR        *((uint32_t *)0xE000102C)
#define UART1_DATA      *((uint32_t *)0xE0001030)
#define UART1_BAUDDIV   *((uint32_t *)0xE0001034)

#define BaudGen115200   0x7c
#define BaudDiv115200   6

// Button / LED / 7-segment defines
#define Button_Data     *((uint32_t *)0x41200000)
#define LED_Data        *((uint32_t *)0x41210000)
#define SEG_CTL         *((uint32_t *)0x43C10000)
#define SEG_DATA        *((uint32_t *)0x43C10004)

// SPI / SLCR defines
#define SLCR_UNLOCK     *((uint32_t *)0xF8000008)
#define SLCR_LOCK       *((uint32_t *)0xF8000004)
#define SLCR_SPI_RST    *((uint32_t *)0xF800021C)
#define UNLOCK_KEY      0xDF0D
#define LOCK_KEY        0x767B

#define SPI0_CR         *((uint32_t *)0xE0006000)
#define SPI0_SR         *((uint32_t *)0xE0006004)
#define SPI0_TXD        *((uint32_t *)0xE000601C)
#define SPI0_RXD        *((uint32_t *)0xE0006020)
#define SPI0_SSR        *((uint32_t *)0xE0006070)
#define SPI0_EN         *((uint32_t *)0xE0006014)
#define SPI0_CFG        SPI0_CR

#define CFG_NoSS        0x0000BC27
#define CFG_SS0         0x00008027
#define CFG_SS0_Start   0x00018027

// LSM9DS1 register map
#define WHO_AM_I        0x0F
#define CTRL_REG1_G     0x10
#define OUT_TEMP_L      0x15
#define OUT_TEMP_H      0x16
#define CTRL_REG6_XL    0x20
#define OUT_X_G_L       0x18
#define OUT_X_G_H       0x19
#define OUT_Y_G_L       0x1A
#define OUT_Y_G_H       0x1B
#define OUT_Z_G_L       0x1C
#define OUT_Z_G_H       0x1D
#define OUT_X_XL_L      0x28
#define OUT_X_XL_H      0x29
#define OUT_Y_XL_L      0x2A
#define OUT_Y_XL_H      0x2B
#define OUT_Z_XL_L      0x2C
#define OUT_Z_XL_H      0x2D

// different than Mark Walker: added scale factors from demo code
#define ACCEL_SCALE     0.000061f
#define GYRO_SCALE      0.00875f

char SendString[200];

// UART functions
int uart1_tx_full()
{
    return (UART1_SR & 16) != 0;
}

void uart1_put_char(char dat)
{
    while (uart1_tx_full());
    UART1_DATA = dat;
}

void uart1_putstr(char *msg)
{
    while (*msg != 0) {
        uart1_put_char(*msg);
        msg++;
    }
}

void ResetUART1()
{
    UART1_CR = 3;
    while ((UART1_CR & 0x2) == 2) {}
    while ((UART1_CR & 0x1) == 1) {}
}

void Configure_UART1()
{
    uint32_t UARTData;
    UART1_MR = 0x20;
    UARTData = UART1_CR;
    UARTData |= 0x14;
    UART1_CR = UARTData;
}

void SetBaudrate1()
{
    UART1_BAUDGEN = BaudGen115200;
    UART1_BAUDDIV = BaudDiv115200;
}

void initUart1()
{
    ResetUART1();
    SetBaudrate1();
    Configure_UART1();
}

// 7-segment functions
void Display_Digit(uint8_t pos, uint8_t val)
{
    uint32_t temp = 0;
    SEG_CTL = 1;
    temp = SEG_DATA;

    switch (pos) {
        case 1: temp &= 0xFFFFFFF0; break;
        case 2: temp &= 0xFFFFF0FF; break;
        case 3: temp &= 0xFFF0FFFF; break;
        case 4: temp &= 0xF0FFFFFF; break;
        default: break;
    }

    temp |= (val & 0xF) << ((pos - 1) * 8);
    temp |= 0x80808080;
    SEG_DATA = temp;
}

void Disp_BCD(uint16_t value)
{
    char bcdstr[20];
    int numchars, strlen_local;

    numchars = sprintf(bcdstr, "%d", value);
    strlen_local = numchars;

    SEG_DATA = 0x80808080;

    while (numchars != 0) {
        Display_Digit(numchars, (bcdstr[strlen_local - numchars] - '0'));
        numchars--;
    }
}

// SPI functions
void reset_SPI(void)
{
    int i = 0;
    SLCR_UNLOCK = UNLOCK_KEY;
    SLCR_SPI_RST = 0xF;
    for (i = 0; i < 1000; i++);
    SLCR_SPI_RST = 0;
    SLCR_LOCK = LOCK_KEY;
}

// to write routine using demo SPI wait logic
void WRITE_SPI_Byte(uint8_t adr, uint8_t WRITE_BYTE)
{
    SPI0_CFG = CFG_SS0;
    SPI0_TXD = (adr & 0x7F);
    SPI0_TXD = WRITE_BYTE;
    SPI0_CFG |= 0x10000;

    while (SPI0_SR & 0x08);

    SPI0_CFG = CFG_SS0;
    (void)SPI0_RXD;
    (void)SPI0_RXD;
}

// to read routine using demo SPI wait logic
uint8_t READ_SPI_Byte(uint8_t adr)
{
    uint8_t return_read;

    SPI0_CFG = CFG_SS0;
    SPI0_TXD = (adr | 0x80);
    SPI0_TXD = 0x00;
    SPI0_CFG |= 0x10000;

    while (SPI0_SR & 0x08);

    SPI0_CFG = CFG_SS0;
    (void)SPI0_RXD;
    return_read = (uint8_t)SPI0_RXD;

    return return_read;
}

// added 16-bit read helper
int16_t READ_SPI_Word(uint8_t low_addr, uint8_t high_addr)
{
    uint8_t lo;
    uint8_t hi;

    lo = READ_SPI_Byte(low_addr);
    hi = READ_SPI_Byte(high_addr);

    return (int16_t)((hi << 8) | lo);
}

// Sensor helper functions
void init_LSM9DS1()
{
    // for readings using demo init values for more stable sensor readings
    WRITE_SPI_Byte(CTRL_REG1_G, 0xC0);
    WRITE_SPI_Byte(CTRL_REG6_XL, 0xC0);

    usleep(200000);
}

// added temperature read helper
int16_t read_temperature_raw()
{
    return READ_SPI_Word(OUT_TEMP_L, OUT_TEMP_H);
}

// updated temp conversion to match demo handling of 12 valid bits
int convert_temp_c(int16_t raw_temp)
{
    raw_temp = raw_temp >> 4;
    return (raw_temp / 16) + 25;
}

// added accelerometer raw helpers
int16_t read_accel_x_raw()
{
    return READ_SPI_Word(OUT_X_XL_L, OUT_X_XL_H);
}

int16_t read_accel_y_raw()
{
    return READ_SPI_Word(OUT_Y_XL_L, OUT_Y_XL_H);
}

int16_t read_accel_z_raw()
{
    return READ_SPI_Word(OUT_Z_XL_L, OUT_Z_XL_H);
}

// added gyro raw helpers for lab requirements
int16_t read_gyro_x_raw()
{
    return READ_SPI_Word(OUT_X_G_L, OUT_X_G_H);
}

int16_t read_gyro_y_raw()
{
    return READ_SPI_Word(OUT_Y_G_L, OUT_Y_G_H);
}

int16_t read_gyro_z_raw()
{
    return READ_SPI_Word(OUT_Z_G_L, OUT_Z_G_H);
}

// added converted accel helpers
float read_accel_x()
{
    return read_accel_x_raw() * ACCEL_SCALE;
}

float read_accel_y()
{
    return read_accel_y_raw() * ACCEL_SCALE;
}

float read_accel_z()
{
    return read_accel_z_raw() * ACCEL_SCALE;
}

// kept gyro read functions for functionality, but not printed
float read_gyro_x()
{
    return read_gyro_x_raw() * GYRO_SCALE;
}

float read_gyro_y()
{
    return read_gyro_y_raw() * GYRO_SCALE;
}

float read_gyro_z()
{
    return read_gyro_z_raw() * GYRO_SCALE;
}

int main()
{
    uint8_t whoami;
    int16_t temp_raw;
    int temp_c;

    float ax, ay, az;
    float gx, gy, gz;

    initUart1();
    uart1_putstr("Program started\r\n");
    uart1_putstr("Inertial Module Lab\r\n");

    SEG_CTL = 0x1;

    reset_SPI();
    SPI0_CFG = CFG_NoSS;
    SPI0_EN = 1;

    init_LSM9DS1();

    // added WHO_AM_I check for debugging
    whoami = READ_SPI_Byte(WHO_AM_I);
    sprintf(SendString, "WHO_AM_I = 0x%02X\r\n", whoami);
    uart1_putstr(SendString);

    while (1) {
        // continuous temperature read
        temp_raw = read_temperature_raw();
        temp_c = convert_temp_c(temp_raw);

        // continuous accelerometer read
        ax = read_accel_x();
        ay = read_accel_y();
        az = read_accel_z();

        // gyro still read for lab functionality
        gx = read_gyro_x();
        gy = read_gyro_y();
        gz = read_gyro_z();

        // UART output does not show GX, GY, GZ
        sprintf(SendString,
                "Temp: %d C | Accel: X=%.1f mg  Y=%.1f mg  Z=%.1f mg\r\n",
                temp_c, ax * 1000, ay * 1000, az * 1000);
        uart1_putstr(SendString);

        // Button 0 displays temperature on 7-segment
        if (Button_Data & 0x1) {
            if (temp_c < 0) {
                Disp_BCD((uint16_t)(-temp_c));
            } else {
                Disp_BCD((uint16_t)temp_c);
            }
        }

        // LED feedback using temperature
        LED_Data = (uint32_t)(temp_c & 0x3FF);

        sleep(1);
    }

    return 0;
}
