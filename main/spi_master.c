// Reference from pico-examples/spi/bme280_spi
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "spimaster.h"

#define SPI_RX_PIN 16
#define SPI_CS_PIN 17
#define SPI_SCK_PIN 18
#define SPI_TX_PIN 19
#define ACTIVE_LOW 0
#define ACTIVE_HIGH 1
#define SEND_REG_BUFFER_SIZE 1
#define FIRST_INDEX 0
#define BME280_CHIP_ID 0x60
#define READ_BIT 0x80
#define BUF_LEN 128
#define BOARDS_RECOGNIZE 1

uint16_t chip_ID [BME280_CHIP_ID];

int32_t t_fine;

uint16_t dig_T1 = 0x81;
int16_t dig_T2 = 0x92;
int16_t dig_T3 = 0xFA;

uint16_t dig_P1 = 0xEC;
int16_t dig_P2 = 0xCA;
int16_t dig_P3 = 0x56;
int16_t dig_P4 = 0xAA;
int16_t dig_P5 = 0xBC;
int16_t dig_P6 = 0x35;
int16_t dig_P7 = 0x68;
int16_t dig_P8 = 0xED;
int16_t dig_P9 = 0x9D;

uint16_t dig_H1 = 0xCA;
uint16_t dig_H3 = 0x39;
int16_t dig_H2 = 0xAC;
int16_t dig_H4 = 0x79;
int16_t dig_H5 = 0x65;
int8_t dig_H6 = 0xEB;

// Returns temperature in Degree Celcius in resolution of 0.01 Degree Celcius. e.g. Output is 4123, temperature is 41.23 Degree Celcius
int32_t compensate_temp(int32_t adc_T) {
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t) dig_T1 << 1))) * ((int32_t) dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t) dig_T1)) * ((adc_T >> 4) - ((int32_t) dig_T1))) >> 12) * ((int32_t) dig_T3))
            >> 14;

    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa in Q24.8 format. e.g. Output is 256000, pressure is 256000 / 256 = 1000 Pa
uint32_t compensate_pressure(int32_t adc_P) {
    int32_t var1, var2;
    uint32_t p;
    var1 = (((int32_t) t_fine) >> 1) - (int32_t) 64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) dig_P6);
    var2 = var2 + ((var1 * ((int32_t) dig_P5)) << 1);
    var2 = (var2 >> 2) + (((int32_t) dig_P4) << 16);
    var1 = (((dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t) dig_P2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t) dig_P1)) >> 15);
    if (var1 == 0)
        return 0;

    p = (((uint32_t) (((int32_t) 1048576) - adc_P) - (var2 >> 12))) * 3125;
    if (p < 0x80000000)
        p = (p << 1) / ((uint32_t) var1);
    else
        p = (p / (uint32_t) var1) * 2;

    var1 = (((int32_t) dig_P9) * ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
    var2 = (((int32_t) (p >> 2)) * ((int32_t) dig_P8)) >> 13;
    p = (uint32_t) ((int32_t) p + ((var1 + var2 + dig_P7) >> 4));

    return p;
}

// Returns humidity in %RH in Q22.10 format. e.g. Output is 102400, humidity is 102400 / 1024 = 100 %RH
uint32_t compensate_humidity(int32_t adc_H) {
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t) 76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t) dig_H4) << 20) - (((int32_t) dig_H5) * v_x1_u32r)) +
                   ((int32_t) 16384)) >> 15) * (((((((v_x1_u32r * ((int32_t) dig_H6)) >> 10) * (((v_x1_u32r *
                                                                                                  ((int32_t) dig_H3))
            >> 11) + ((int32_t) 32768))) >> 10) + ((int32_t) 2097152)) *
                                                 ((int32_t) dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t) dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
    v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);

    return (uint32_t) (v_x1_u32r >> 12);
}


#ifdef PICO_DEFAULT_SPI_CSN_PIN
// Set chip select pin to active low to start communication
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_CS_PIN, ACTIVE_LOW);  // Active low
    asm volatile("nop \n nop \n nop");
}

// Set chip select pin to active high to stop communication
static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_CS_PIN, ACTIVE_HIGH); // Active high
    asm volatile("nop \n nop \n nop");
}
#endif

#if defined(spi_default) && defined(SPI_CS_PIN)
// Send slave the registers to be read and await for reply with their value
static void read_registers(uint8_t reg, uint8_t *buf, uint16_t len) {
    reg |= READ_BIT;
    uint8_t reg_readings[SEND_REG_BUFFER_SIZE];
    reg_readings[FIRST_INDEX] = reg;
    cs_select();
    spi_write_blocking(spi_default, reg_readings, SEND_REG_BUFFER_SIZE);
    sleep_ms(10);
    spi_read_blocking(spi_default, 0, buf, len);
    cs_deselect();
    sleep_ms(10);
}

/* This function reads the manufacturing assigned compensation parameters from the device */
void read_compensation_parameters() {
    uint8_t buffer[26];

    read_registers(0x88, buffer, 24);

    dig_T1 = buffer[0] | (buffer[1] << 8);
    dig_T2 = buffer[2] | (buffer[3] << 8);
    dig_T3 = buffer[4] | (buffer[5] << 8);

    dig_P1 = buffer[6] | (buffer[7] << 8);
    dig_P2 = buffer[8] | (buffer[9] << 8);
    dig_P3 = buffer[10] | (buffer[11] << 8);
    dig_P4 = buffer[12] | (buffer[13] << 8);
    dig_P5 = buffer[14] | (buffer[15] << 8);
    dig_P6 = buffer[16] | (buffer[17] << 8);
    dig_P7 = buffer[18] | (buffer[19] << 8);
    dig_P8 = buffer[20] | (buffer[21] << 8);
    dig_P9 = buffer[22] | (buffer[23] << 8);

    dig_H1 = buffer[25];

    read_registers(0xE1, buffer, 8);

    dig_H2 = buffer[0] | (buffer[1] << 8);
    dig_H3 = (int8_t) buffer[2];
    dig_H4 = buffer[3] << 4 | (buffer[4] & 0xf);
    dig_H5 = (buffer[5] >> 4) | (buffer[6] << 4);
    dig_H6 = (int8_t) buffer[7];
}

// Send request to slave for readings
static void bme280_read_raw(int32_t *humidity, int32_t *pressure, int32_t *temperature) {
    uint8_t buffer[8];
    read_registers(0xF7, buffer, 8);
    *pressure = ((uint32_t) buffer[0] << 12) | ((uint32_t) buffer[1] << 4) | (buffer[2] >> 4);
    *temperature = ((uint32_t) buffer[3] << 12) | ((uint32_t) buffer[4] << 4) | (buffer[5] >> 4);
    *humidity = (uint32_t) buffer[6] << 8 | buffer[7];
}
#endif

int bme280_master() {
    stdio_init_all();
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/bme280_spi example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else
    sleep_ms(5000);
    printf("Hello, bme280! Reading raw data from registers via SPI...\n");

    // Using SPI0 at 0.5MHz.
    spi_init(spi_default, 500 * 1000);
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function (SPI_CS_PIN, GPIO_FUNC_SPI);
    spi_set_slave (spi_default, false); // Set device as master

    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    uint8_t id;
    int32_t humidity, pressure, temperature;
    bool board_detected = false;

    while (1) { // Endless loop
        if (!board_detected) { // When master have not detected any recognized board
            read_registers(0xD0, &id, 5);
            printf("Chip ID is 0x%x\n", id);

            // for (int i = 0; i < BOARDS_RECOGNIZE; i++) {
            //     if (id == chip_ID[i]) {
            //         board_detected = true;
            //     }
            // }

            if (id == BME280_CHIP_ID) { // When BME280 detected
                board_detected = true;
                printf("BME280 Sensor detected\n");
                printf("Begin request for readings\n");
            }
        } else { // When master has detected a recognized board
            if (id == BME280_CHIP_ID) { // When BME280 detected
                bme280_read_raw(&humidity, &pressure, &temperature); // Getting raw values from slave

                // Compensation to get readable values
                temperature = compensate_temp(temperature);
                pressure = compensate_pressure(pressure);
                humidity = compensate_humidity(humidity);

                // Printing of readings
                printf("Humidity = %.2f%%\n", humidity / 1024.0);
                printf("Pressure = %dPa\n", pressure);
                printf("Temp. = %.2fC\n\n", temperature / 100.0);
                }
        }
      sleep_ms(3000);
    }
#endif
}

int exec_bme280_master() {
    bme280_master();
}
