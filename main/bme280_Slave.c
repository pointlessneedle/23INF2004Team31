// Reference from pico-examples/spi/bme280_spi

// To be run on seperate pico connected to another pico acting as the master
 
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#define SPI_RX_PIN 16
#define SPI_CS_PIN 17
#define SPI_SCK_PIN 18
#define SPI_TX_PIN 19
#define FIRST_INDEX 0
#define SEND_REG_BUFFER_SIZE 1
#define READ_REG_BUFFER_SIZE 1
#define READ_BIT 0x80
#define READING_BUFFER_LENGTH 8
#define BME280_CHIP_ID 0x60

#ifdef PICO_DEFAULT_SPI_CSN_PIN
// Set chip select pin to active low to start communication
static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_CS_PIN, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

// Set chip select pin to active high to stop communication
static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(SPI_CS_PIN, 1); // Active low
    asm volatile("nop \n nop \n nop");
}
#endif

#if defined(spi_default) && defined(SPI_CS_PIN)
#endif

int main() {
    stdio_init_all();
#if !defined(spi_default) || !defined(PICO_DEFAULT_SPI_SCK_PIN) || !defined(PICO_DEFAULT_SPI_TX_PIN) || !defined(PICO_DEFAULT_SPI_RX_PIN) || !defined(PICO_DEFAULT_SPI_CSN_PIN)
#warning spi/bme280_spi example requires a board with SPI pins
    puts("Default SPI pins were not defined");
#else

    sleep_ms(5000);

    // Using SPI0 at 0.5MHz.
    spi_init(spi_default, 500 * 1000);
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function (SPI_CS_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    spi_set_slave (spi_default, true); // Set device as slave

    // Make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PICO_DEFAULT_SPI_RX_PIN, PICO_DEFAULT_SPI_TX_PIN, PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Make the CS pin available to picotool
    bi_decl(bi_1pin_with_name(PICO_DEFAULT_SPI_CSN_PIN, "SPI CS"));

    uint8_t chip_id[SEND_REG_BUFFER_SIZE];
    chip_id[FIRST_INDEX] = BME280_CHIP_ID;

    uint8_t buf[READING_BUFFER_LENGTH];
    uint8_t reg_buf[SEND_REG_BUFFER_SIZE];
    reg_buf[FIRST_INDEX] = 0;
    for (int i = 0; i < READING_BUFFER_LENGTH; i++) { // Fill fake readings to the buffer to be sent to master
        buf[i] = 99;
    }
    while (1) { // Endless loop
        spi_read_blocking (spi_default, 0, reg_buf, READ_REG_BUFFER_SIZE); // Read requested register containing chip ID

        spi_write_blocking (spi_default, chip_id, READ_REG_BUFFER_SIZE); // Reply back with chip ID of BME280
        printf("Sending chip ID 0x60...\n\n");

        // Once the master received the chip ID, start receiving the register requests from it and send the fake readings
        if (spi_is_writable(spi_default)) { 
            printf("Master detected\n");
            printf("Start receiving read requests\n\n");
            while (1) { // Endless loop
                spi_read_blocking (spi_default, 0, reg_buf, READ_REG_BUFFER_SIZE); // Receive the register requests from master
                printf("Requested register: %x\n", reg_buf[FIRST_INDEX]);
                spi_write_blocking (spi_default, buf, 8); // Send the fake readings
                printf("Sending false data\n");
            }
        }
        sleep_ms(2000);
    }
#endif
}
