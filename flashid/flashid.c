#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"

#define SPI_PORT        spi0
#define SPI_TX_PIN      3
#define SPI_RX_PIN      4
#define SPI_SCK_PIN     2
#define SPI_CS_PIN      5

#define READ_DATA       0x03    // Address Read Data
#define CHIP_ID         0x9F    // Address Status Read ID

#define BAUD_RATE       1000*1000
#define PAGE_SIZE       256     // Page Size reading
#define SECTOR_SIZE     4096

#define MAX_SIZE 38

// Array to store keys / hexadecimal instruction returned by flash
char keys[MAX_SIZE][20] = {"0x3010","0x3011","0x3012","0x3013",
                    "0x4012","0x4013","0x4014","0x4015",
                    "0x4016","0x4017","0x4018","0x4019", 
                    "0x4020","0x4021","0x6011","0x6012",
                    "0x6013","0x6014","0x6015","0x6016",
                    "0x6017","0x6018","0x6019","0x6020",
                    "0x6021","0x7015","0x7016","0x7017",
                    "0x7018","0x7019","0x7022","0x8015",
                    "0x8016","0x8017","0x8018","0x8019",
                    "0x8020", "-1"};

// Array to store values / flash type
char values[MAX_SIZE][150] = {"W25X05CL","W25X10CL","W25X20CL","W25X40CL",
                      "W25Q20CL","W25Q40CL","W2525Q80/DV","W25Q16/JL or W25Q16JV-IQ",
                      "W25Q32 or W25Q32JV-IQ/JQ","W25Q64FV (SPI) or W25Q64JV-IQ/JQ","W25Q128BV or W25Q128JV-IQ/JQ","W25Q256FV (SPI Mode) or W25Q256JV-IQ",
                      "W25Q512JV-IQ/IN","W25Q01JV-IQ","W25Q10EW","W25Q20EW",
                      "W25Q40EW","W25Q80EW","W25Q16JW-IQ/JQ","W25Q32FW or W25Q32JW-IQ",
                      "W25Q64FV (QPI) or W25Q64JW-IQ", "W25Q128FV (QPI Mode) or W25Q128FW or W25Q128JW-IQ", "W25Q256FV (QPI Mode) or W25Q257FV (QPI Mode) or W25Q256JW", "W25Q512NW-IQ/IN", 
                      "W25Q01NW-IQ/JQ", "W25Q16JV-IM","W25Q32Jv-IM*/JM*","W25Q64JV-IM/JM",
                      "25Q128JV-IM/JM","W25Q256JV-IM","W25Q02JV-IM","W25Q16JW-IM",
                      "W25Q32JW-IM","W25Q64JW-IM","W25Q128JW-IM","W25Q256JW-IM",
                      "W25Q512NW-IM", "-1"};

static inline void cs_select(uint cs_pin){
    gpio_put(cs_pin, 0);
}
static inline void cs_deselect(uint cs_pin){
    gpio_put(cs_pin, 1);
}

// Function to get the index of a key in the keys array
int getIndex(char key[]){
    for (int i = 0; i < MAX_SIZE; i++) { 
        if (strcmp(keys[i], key) == 0) { 
            return i; 
        } 
    } 
    return -1; // Key not found 
}

// Function to get value of a key in the map
char* check_item(char key[]) 
{ 
    int index = getIndex(key); 
    if (index == -1) {
        // Key not found
        return NULL;
    } 
    else {
        // Key found 
        return values[index]; 
    }
}
// Function to list all the items
void printAllItems()
{ 
    for (int i = 0; i < MAX_SIZE; i++) { 
        printf("%s: %s\n", keys[i], values[i]); 
    } 
} 

void printbuf(uint8_t buf[PAGE_SIZE]) {
    for (int i = 0; i < PAGE_SIZE; ++i) {
        if (i % 16 == 15)
            printf("%02x\n", buf[i]);
        else
            printf("%02x ", buf[i]);
    }
}

// Main function to read the ID of flash chip
void __not_in_flash_func(read_chip_id)(spi_inst_t *spi, uint cs_pin) {
    printf("Command: Reading ID\n");
    cs_select(cs_pin);
    sleep_ms(20);
    
    uint8_t tx_buffer[4] = {CHIP_ID};   // tx to send 9Fh instruction
    uint8_t rx_buffer[4] = {0x00};      // rx to receive replies from the instruction

    spi_write_read_blocking(spi, tx_buffer, rx_buffer, sizeof(tx_buffer));

    // Getting manufacturerID and deviceID
    uint8_t buffer1 = rx_buffer[1]; // manufacturerID
    uint8_t buffer2 = rx_buffer[2]; // 2 and 3 = deviceID
    uint8_t buffer3 = rx_buffer[3]; // 2 and 3 = deviceID
    uint8_t buffer4 = rx_buffer[4]; // chip id address

    // Combining buffer2 and buffer3 together to get deviceID
    uint16_t device_id = ((uint16_t)buffer2 << 8) | buffer3;
    //printf("Device ID: 0x%02X\n", device_id);

    if ( buffer4 & CHIP_ID ){
        printf("\n== Details of connected flash ==\n");
        sleep_ms(200);  // Sleep added to slow down the processing speed of the pico for print statement
        printf("Flash Type: Winbond Serial Flash\n");
    }

    // Check if the device ID is under the list of known flash type as defined above
    char device_id_str[7];
    snprintf(device_id_str, sizeof(device_id_str), "0x%02X", device_id);
    if (check_item(device_id_str) != NULL){
        printf("Manufacturer ID: 0x%02X\n", (uint16_t)buffer1);
        sleep_ms(200);
        printf("Device ID: 0x%02X\n", device_id);
        sleep_ms(200);
        printf("Winbond Part #: %s\r\n", check_item(device_id_str));
    } else {
        printf("Unknown flash type.\r\n");
    }
}

int main() {
    //stdio_init_all();
    stdio_usb_init();
    // set the baud rate to 1MHz
    spi_init(SPI_PORT, BAUD_RATE);
    spi_set_baudrate(SPI_PORT, BAUD_RATE);
    
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);    
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);

    // Set up CS pin, initialisation and setting it to active-high state
    gpio_init(SPI_CS_PIN);
    gpio_put(SPI_CS_PIN, 1);
    gpio_set_dir(SPI_CS_PIN, GPIO_OUT);
    
    sleep_ms(1000);

    sleep_ms(5000);
    printf("Starting...\n");
    //printAllItems();

    sleep_ms(2000);
    read_chip_id(SPI_PORT, SPI_CS_PIN);

    return 0;
}
