#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "sd_card.h"
#include "ff.h"

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

// Structure to hold the flash ID information
typedef struct {
    uint8_t buffer1;
    uint16_t device_id;
    char device_id_part[150];
} FlashID;

/* MicroSD Card Reading and Writing */
FRESULT fr;
FATFS fs;
FIL fil;
DIR dir;
void initialise_sd(){
    // Initialize SD card
    //
    if (!sd_init_driver()) {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true);
    }
    sleep_ms(10);
    // Mount drive
    //
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK) {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true);
    }
    sleep_ms(10);
}

void read_file(const char *path) {
    char buf[100];
    // Open file to read, if error print error
    //
    fr = f_open(&fil, path, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file %s (%d)\r\n", path, fr);
        return;
    }

    printf("Reading from file '%s':\r\n", path);
    printf("------\r\n");
    // Printing contents in the file
    //
    while (f_gets(buf, sizeof(buf), &fil)) {
        printf(buf);
    }
    printf("\r\n------\r\n");

    // Close file, if error print error
    //
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file %s (%d)\r\n", path, fr);
    }
}

void list_and_read_all_files(const char *path) {
    static FILINFO filinfo;
    // Full directory path
    //
    char dir_path[100];

    fr = f_opendir(&dir, path);
    if (fr != FR_OK) {
        printf("ERROR: Could not open directory (%d)\r\n", fr);
        return;
    }

    while (true) {
        fr = f_readdir(&dir, &filinfo);
        // Check for errors or end of directory
        //
        if (fr != FR_OK || filinfo.fname[0] == 0) {
            break;
        }
        // Check if it is a directory
        //
        if (filinfo.fattrib & AM_DIR) {
            printf("Directory: %s\r\n", filinfo.fname);
        }
        else {
            // Print statements on serial here
            //
            printf("File: %s\r\n", filinfo.fname);
            // Get and create the full path
            //
            snprintf(dir_path, sizeof(dir_path), "%s/%s", path, filinfo.fname);
            // Read the file
            //
            read_file(dir_path);
        }
    }
}

void write_to_new_file(char *file_name, char *data) { 
    printf(". ");
    // Open file for writing while creating new file
    //
    fr = f_open(&fil, file_name, FA_WRITE | FA_CREATE_ALWAYS); 
    if (fr != FR_OK) { 
        printf("ERROR: Could not open file (%d)\r\n", fr); 
        while (true); 
    }
    printf(". ");
    // Write the content in variable data to the file
    //
    int ret = f_printf(&fil, data); 
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret); 
        f_close(&fil); 
        while (true); 
    }
    printf(". ");
    // Close file 
    //
    fr = f_close(&fil); 
    if (fr != FR_OK) { 
        printf("ERROR: Could not close file (%d)\r\n", fr); 
        while (true); 
    } 
    printf(". ");
    sleep_ms(10);
} 

int day, month, year;
void get_system_time(){
    uint64_t current_time = time_us_64();
    struct tm* time_info = gmtime((time_t*)&current_time);
    sleep_ms(10);
    if (time_info) {
        // Extract the day, month and year within the system, and change to fit current date
        year = time_info->tm_year + 1953; // Years since 1900
        month = time_info->tm_mon + 8; // Month is 0-indexed
        day = time_info->tm_mday + 8;
        // Output print DD:MM:YY
        printf("Current System Time: %02d:%02d:%02d\n", day, month, year);
    } else {
        printf("Error getting system time\n");
    }
}

void write_to_existing_file(char *file_name, char *data) { 
    // Open file for appending or overwriting existing file
    // 
    fr = f_open(&fil, file_name, FA_OPEN_APPEND | FA_WRITE); 
    if (fr != FR_OK) { 
        printf("ERROR: Could not open file (%d)\r\n", fr); 
        while (true); 
    } 
    // Write the content in variable data to the file
    //
    int ret = f_printf(&fil, data); 
    if (ret < 0) { 
        printf("ERROR: Could not write to file (%d)\r\n", ret); 
        f_close(&fil); 
        while (true); 
    } 
    // Close file 
    //
    fr = f_close(&fil); 
    if (fr != FR_OK) { 
        printf("ERROR: Could not close file (%d)\r\n", fr); 
        while (true); 
    } 
}

/* SPI Flash ID Reading */
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
FlashID __not_in_flash_func(read_chip_id)(spi_inst_t *spi, uint cs_pin) {
    FlashID flash_id;
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
    char device_id_part[7];
    snprintf(device_id_part, sizeof(device_id_part), "0x%02X", device_id);
    if (check_item(device_id_part) != NULL){
        printf("Manufacturer ID: 0x%02X\n", (uint16_t)buffer1);
        sleep_ms(200);
        printf("Device ID: 0x%02X\n", device_id);
        sleep_ms(200);
        printf("Winbond Part #: %s\r\n\n", check_item(device_id_part));

        // Values to be returned to structure
        char buffer1_str[10];
        char device_id_str[10];
        char device_id_part_str[150];
        
        flash_id.buffer1 = (uint16_t)buffer1;
        flash_id.device_id = device_id;
        snprintf(flash_id.device_id_part, sizeof(flash_id.device_id_part), "%s\r\n\n", check_item(device_id_part));
    } else {
        printf("Unknown flash type.\r\n");
    }
    return flash_id;    // Return values into the structure
}


int main() {
    stdio_init_all();
    //stdio_usb_init();
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

    sleep_ms(5000);
    printf("\nStarting...\n");
    //printAllItems();

    sleep_ms(2000);
    //read_chip_id(SPI_PORT, SPI_CS_PIN);
    FlashID flashid = read_chip_id(SPI_PORT, SPI_CS_PIN);
    sleep_ms(1000);

    // Initialisation for sd card and gettin system time
    printf("\n== Details of MicroSD Card & Reader ==\n"); 
    initialise_sd();
    get_system_time();
    
    // Collating statements and content of flash id to save into file
    char to_sd_output[20];
    char buffer1_str_output[25];
    char device_id_str_output[20];
    char device_id_part_str_output[170];
    snprintf(to_sd_output, sizeof(to_sd_output), "%02d_%02d_%02d.txt", day, month, year);
    snprintf(buffer1_str_output, sizeof(buffer1_str_output), "Manufacturer ID: 0x%02X\n", (uint16_t)flashid.buffer1);
    snprintf(device_id_str_output, sizeof(device_id_str_output), "Device ID: 0x%02X\n", flashid.device_id);
    snprintf(device_id_part_str_output, sizeof(device_id_part_str_output), "Winbond Part #: %s\r\n\n", flashid.device_id_part);
    
    sleep_ms(10);
    
    // Printing
    printf("\nSaving content to microsd card...\n");
    write_to_new_file(to_sd_output, buffer1_str_output);
    write_to_existing_file(to_sd_output, device_id_str_output);
    write_to_existing_file(to_sd_output, device_id_part_str_output);
    sleep_ms(10);
    printf("\n\n=== Content Saved ===\n");

    sleep_ms(10);
    // Unmount drive
    //
    f_unmount("0:");
    
    return 0;
}