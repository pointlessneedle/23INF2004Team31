#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"

#define SPI_PORT        spi0
#define SPI_TX_PIN      3
#define SPI_RX_PIN      4
#define SPI_SCK_PIN     2
#define SPI_CS_PIN      5

#define WRITE_ENABLE    0x06    // Address Write Enable
#define WRITE_DISABLE   0x04    // Address Write Disable
#define CHIP_ERASE      0xC7    // Address Chip Erase   --> is also 60H (to note)
#define SECTOR_ERASE    0x20    // Address Sector Erase
#define READ_STATUS_REG 0x05    // Address Read Status
#define READ_DATA       0x03    // Address Read Data
#define PAGE_PROGRAM    0x02    // Address Status Page Program
#define BUSY_STATUS     0x01    // Address Busy Status

#define CHIP_COMMAND_ID 0x9F    // Address Status Read ID

#define BAUD_RATE       1000*1000
#define PAGE_SIZE       256     // Page Size reading
#define SECTOR_SIZE     4096

void wait_busy(spi_inst_t *spi, uint cs_pin);

static inline void cs_select(uint cs_pin){
    gpio_put(cs_pin, 0);
}
static inline void cs_deselect(uint cs_pin){
    gpio_put(cs_pin, 1);
}

void printbuf(uint8_t buf[PAGE_SIZE]) {
    for (int i = 0; i < PAGE_SIZE; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }
}

void __not_in_flash_func(read_flash)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t *buf, size_t len) {
    printf("Command: Reading Flash...\n");
    cs_select(cs_pin);
    
    // Data to be read divided into 3 bytes, high | middle | low byte of address respectively
    //
    uint8_t cmdbuf[4] = {READ_DATA, addr >> 16, addr >> 8, addr};
    spi_write_blocking(spi, cmdbuf, 4);
    spi_read_blocking(spi, 0, buf, len);
    sleep_ms(20);
    cs_deselect(cs_pin);

    printf("Ready\n");
}

void __not_in_flash_func(write_flash_enable)(spi_inst_t *spi, uint cs_pin) {
    printf("Command: Enable write to flash\n");
    
    uint8_t en = WRITE_ENABLE;
    
    cs_select(cs_pin);
    
    spi_write_blocking(spi, &en, 1);
    sleep_ms(20);

    cs_deselect(cs_pin);
    
    printf("Ready\n");
}

void __not_in_flash_func(write_program)(spi_inst_t *spi, uint cs_pin, uint32_t addr, uint8_t data[]){
    uint8_t cmdbuf[4] = {PAGE_PROGRAM, addr >> 16, addr >> 8, addr};

    cs_select(cs_pin);
    write_flash_enable(spi, cs_pin);
    sleep_ms(20);
    spi_write_blocking(spi, cmdbuf, 4);
    spi_write_blocking(spi, data, PAGE_SIZE);
    sleep_ms(20);

    cs_deselect(cs_pin);
    wait_busy(spi, cs_pin);    
}

void __not_in_flash_func(sector_erase)(spi_inst_t *spi, uint cs_pin, uint32_t addr){
    uint8_t cmdbuf[4] = { SECTOR_ERASE, addr >> 16, addr >> 8, addr };
    write_flash_enable(spi, cs_pin);
    cs_select(cs_pin);
    spi_write_blocking(spi, cmdbuf, 4);
    sleep_ms(20);
    cs_deselect(cs_pin);
    wait_busy(spi, cs_pin);
}

void __not_in_flash_func(wait_busy)(spi_inst_t *spi, uint cs_pin){
    uint8_t flag;
    do{
        cs_select(cs_pin);
        uint8_t buf[2] = {READ_STATUS_REG, 0};
        spi_write_read_blocking(spi, buf, buf, 2);
        sleep_ms(20);
        cs_deselect(cs_pin);
        flag = buf[1];
    }
    while(flag & BUSY_STATUS);  // Binary check if flag and busy status is same
}

void __not_in_flash_func(read_chip_id)(spi_inst_t *spi, uint cs_pin) {
    printf("Set Command: chipCmdIda\n");

    cs_select(cs_pin);
    
    cs_deselect(cs_pin);
    
}

int main() {
    stdio_init_all();
    // set the baud rate to 1MHz
    spi_init(SPI_PORT, 1000000);
    spi_set_baudrate(SPI_PORT, 1000000);
    
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);    
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);

    bi_decl(bi_3pins_with_func(SPI_RX_PIN, SPI_TX_PIN, SPI_SCK_PIN, GPIO_FUNC_SPI));

    // Set up CS pin, initialisation and setting it to active-high state
    gpio_init(SPI_CS_PIN);
    gpio_put(SPI_CS_PIN, 1);
    gpio_set_dir(SPI_CS_PIN, GPIO_OUT);
    bi_decl(bi_1pin_with_name(SPI_CS_PIN, "SPI CS"));

    uint8_t page_buf[PAGE_SIZE];
    const uint32_t target_address = 0;

    sleep_ms(5000);
    printf("Starting...\n");
    sleep_ms(3000);

    printf("\nFirst Reading:\n");
    sector_erase(SPI_PORT, SPI_CS_PIN, target_address); // Sector erase to start clean state
    read_flash(SPI_PORT, SPI_CS_PIN, target_address, page_buf, PAGE_SIZE);
    printbuf(page_buf);

    printf("\nWriting to Flash...\n");
    // Writing to flash
    for(int i = 0; i < PAGE_SIZE; i++){
        // For each bytes in size of page, write 100 (decimal integer) which is 64 in hexadecimal
        page_buf[i] = 100;
    }
    sector_erase(SPI_PORT, SPI_CS_PIN, target_address);
    write_program(SPI_PORT, SPI_CS_PIN, target_address, page_buf);
    sleep_ms(1000);
    printf("\nAfter Writing to Flash:\n");
    printbuf(page_buf);

    printf("\nErase Flash and Read Again:\n");
    sleep_ms(1000);
    sector_erase(SPI_PORT, SPI_CS_PIN, target_address);
    read_flash(SPI_PORT, SPI_CS_PIN, target_address, page_buf, PAGE_SIZE);
    printbuf(page_buf);

    return 0;
}
