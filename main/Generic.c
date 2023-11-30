#include <stdio.h>
#include <pico/stdlib.h> 
#include <pico/i2c_slave.h>
#include <string.h>
#include <hardware/i2c.h>
#include "pico/time.h"
#include "config.h"
#include "Generic.h"

// This slave device only mimics I2C 
// Baud Rate is fixed to make 
static const uint I2C_BAUDRATE = 100 * 100;

// Memory map for generic slave emulator
static struct
{
    uint8_t mem[MEM_SIZE]; // based on config.h
    uint8_t target_mem_address;
    bool address_written;
} GenericMemoryMap;

// Initialises the generic memory map based on the inputs from config.h
static void init_memory_map(){
    // Checks that the number of registers and values match
    if (sizeof(registers) == sizeof(reg_value)){
        for (int i = 0; i < sizeof(registers); i++){
            GenericMemoryMap.mem[registers[i]] = reg_value[i];
        }
    } else {
        while (1){
            sleep_ms(1000);
            printf("Error: Register values do not match");
        }
    }
}

// Checks if the data received is actually a command
static bool verifyCommand(uint8_t input){
    for (int i = 0; i < sizeof(commands); i++){
        if (input == commands[i]){
            return true;
        }
    }
    return false;
}

// Checks if the current address can be read from based on the array writeOnly in config.h
static bool verifyReadAllowed(uint8_t input){
    for (int i = 0; i < sizeof(writeOnly); i++){
        if (input == writeOnly[i]){
            return false;
        }
    }
    return true;
}

// Checks if the current address can be written to based on the array readOnly in config.h
static bool verifyWriteAllowed(uint8_t input){
    for (int i = 0; i < sizeof(readOnly); i++){
        if (input == readOnly[i]){
            return false;
        }
    }
    return true;
}

// Callback for the generic slave device
static void Generic_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    switch (event){
        uint8_t data;
        case I2C_SLAVE_RECEIVE: // For when the master wants to write to slave
            data = i2c_read_byte_raw(i2c);
            // First checks if an address has been written, which will be used to access the memory map
            if (GenericMemoryMap.address_written){
                if (verifyWriteAllowed(GenericMemoryMap.target_mem_address)){
                    GenericMemoryMap.mem[GenericMemoryMap.target_mem_address] = data;
                    GenericMemoryMap.target_mem_address++;
                } else {
                    printf("%02X is not allowed to be written to\n");
                }
            } else {
                // Checks if current data is a command
                if (verifyCommand(data)){
                    printf("Received command %02X\n", data);
                } else {
                    // target_mem_address is set to be used for reading or writing
                    GenericMemoryMap.target_mem_address = data;
                    GenericMemoryMap.address_written = true;
                }
            }
            break;
        case I2C_SLAVE_REQUEST: // For when the master wants to read from slave
            printf("Slave receive for %02X\n", GenericMemoryMap.target_mem_address);
            if (verifyReadAllowed(GenericMemoryMap.target_mem_address)){
                i2c_write_byte_raw(i2c, GenericMemoryMap.mem[GenericMemoryMap.target_mem_address]);
                GenericMemoryMap.target_mem_address++;
            } else {
                printf("%02X is not allowed to be read from\n");
            }
            break;
        case I2C_SLAVE_FINISH: // For when the master is done with the transfer
            // reset the slave to be ready for next transfer
            GenericMemoryMap.address_written = false;
            break;
        default:
            break;
    }
}

// Sets up the pico w to turn on it's respective GPIO pins based on config.h
void setup_slave_Generic(){
    gpio_init(SLAVE_SDA);
    gpio_set_function(SLAVE_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SDA);

    gpio_init(SLAVE_SCL);
    gpio_set_function(SLAVE_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL);

    // Calls the I2C Slave Init Function
    i2c_slave_init_Caller_Generic();

    // Initialises the memory map
    init_memory_map();
}

void i2c_slave_init_Caller_Generic(){

    // I2C interface is chosen in config.h
    i2c_inst_t *interface;
    if (INTERFACE == 0){
        interface = i2c0;
    } else {
        interface = i2c1;
    }
    i2c_init(interface, I2C_BAUDRATE);

    // Initialises slave using the i2c slave library
    i2c_slave_init(interface, DEV_ADDR, &Generic_i2c_slave_handler);

}

// Sets up and runs the master feature of the generic slave, only if enabled in config.h
static void runMaster() {
    gpio_init(MASTER_SDA);
    gpio_set_function(MASTER_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA);

    gpio_init(MASTER_SCL);
    gpio_set_function(MASTER_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL);

    // I2C interface is chosen in config.h, it should be opposite of the slave interface; i.e. 0 = i2c1, 1 = i2c0
    i2c_inst_t *interface;
    if (INTERFACE == 0){
        interface = i2c1;
    } else {
        interface = i2c0;
    }

    i2c_init(interface, I2C_BAUDRATE);
    sleep_ms(1000);
    
    // While loop is used to constantly keep the master running to read and write to the slave
    while(1) {
        // Based on config.h, the master will read from the slave device
        for (int i = 0; i < sizeof(masterRead); i++){
            uint8_t buf[1] = { 0 };
            uint8_t reg = masterRead[i];
            // Sets the memory address for where the master wants to read
            i2c_write_blocking(interface, DEV_ADDR, &reg, 1, true);
            // Reads and stores the data of the address into a buffer based on buffer size
            i2c_read_blocking(interface, DEV_ADDR, buf, 1, false);
            printf("Master read at %02X: %02X\n", reg, buf[0]);

            // Sleep to prevent errors and for better readability
            sleep_ms(1000);
        }
        // Based on config.h, the master will write to the slave device
        for (int i = 0; i < sizeof(masterWrite_addr); i++){
            // Sets the first byte in the buffer to be the address where data is to be written to
            // The following bytes are data to be written to the device
            uint8_t buf[2] = {masterWrite_addr[i], masterWrite_value[i]};
            i2c_write_blocking(interface, DEV_ADDR, buf, 2, false);
            printf("Master write value %02X at %02X\n", buf[0], buf[1]);

            // Sleep to prevent errors and for better readability
            sleep_ms(1000);
        }
    }
}


// int main() {
//     stdio_init_all(); 

//     setup_slave_Generic();

//     // Creates master if specified by config.h
//     if (CREATE_MASTER == 1){
//         runMaster();
//     }

//     while(1){
//         sleep_ms(1000);
//     }
// }