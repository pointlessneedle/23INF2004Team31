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

#define MAX_SIZE 15

int size = 0;   // current number of elements in map
char keys[MAX_SIZE][20];    // array to store keys
char values[MAX_SIZE][50];      // array to store values

static inline void cs_select(uint cs_pin){
    gpio_put(cs_pin, 0);
}
static inline void cs_deselect(uint cs_pin){
    gpio_put(cs_pin, 1);
}

// Get the index of a key in the keys array
int getIndex(char key[]){
    for (int i = 0; i < size; i++) { 
        if (strcmp(keys[i], key) == 0) { 
            return i; 
        } 
    } 
    return -1; // Key not found 
}

// to insert a key-value pair into the map, when wanting to add new flash series to the list
void add_item(char key[], char value[]){
    int index = getIndex(key); 
    if (index == -1) { // Key not found 
        strcpy(keys[size], key); 
        strcpy(values[size], value);
        size++; 
    } 
    else { // Key found 
        strcpy(values[index], value); 
    }    
}

// To get value of a key in the map
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

void insert_all_to_list(){
    add_item("-1","-1");    // Key not found
    add_item("0x4014","W25Q80");
    add_item("0x4015","W25Q16");
    add_item("0x4016","W25Q32 or W25Q32JV-IQ/JQ");
    add_item("0x4017","W25Q64FV (SPI)");
    add_item("0x4018","W25Q128BV");
    add_item("0x4019","W25Q256FV (SPI Mode) or W25Q256FV (SPI Mode)");
    add_item("0x4021","W25Q01JV-IQ");

    add_item("0x6016","W25Q32FW");
    add_item("0x6017","W25Q64FV (QPI)");
    add_item("0x6018","W25Q128FV (QPI Mode) or W25Q128FW");
    add_item("0x6019","W25Q256FV (QPI Mode) or W25Q257FV (QPI Mode)");
    add_item("0x6021","W25Q01NW-IQ/JQ");
    
    add_item("0x7016","W25Q32Jv-IM*/JM*");
    add_item("0x7022","W25Q02JV-IM");
}

void printMap() 
{ 
    for (int i = 0; i < size; i++) { 
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

    /* Checks */
    printf("check: 0x%02X\n", buffer1);
    printf("check2: 0x%02X\n", buffer2);
    printf("check3: 0x%02X\n", buffer3);
    printf("check4: 0x%02X\n", buffer4);

    // Combining buffer2 and buffer3 together to get deviceID
    uint16_t device_id = ((uint16_t)buffer2 << 8) | buffer3;
    printf("Device ID: 0x%02X\n", device_id);

    if ( buffer4 & CHIP_ID ){
        printf("Flash Type: Winbond Serial Flash\n");
    }

    /* HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE */
    char device_id_str[7];
    snprintf(device_id_str, sizeof(device_id_str), "0x%02X", device_id);

    if (check_item(device_id_str) != NULL){
        printf("Device ID: 0x%02X\nWinbond Part #: %s\n", device_id, check_item(device_id_str));
    } else {
        printf("Unknown flash type.");
    }

    sleep_ms(20);
    printf("Ready\n");
}

int main() {
    stdio_init_all();
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
    // Inserting the list
    insert_all_to_list();

    sleep_ms(5000);
    printf("Starting...\n");

    printMap();

    sleep_ms(2000);
    read_chip_id(SPI_PORT, SPI_CS_PIN);



}
