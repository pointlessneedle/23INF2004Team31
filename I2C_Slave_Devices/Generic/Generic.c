#include <stdio.h>
#include <pico/stdlib.h> 
#include <pico/i2c_slave.h>
#include <string.h>
#include <hardware/i2c.h>
#include "pico/time.h"
#include "config.h"

// This slave device only mimics I2C 

// I2C slave address
//static const uint SLAVE_ADDR = 0x76;
#define SLAVE_ADDR _u(0x76)
static const uint I2C_BAUDRATE = 100 * 100;

#define SLAVE_SDA_PIN 0 // i2c0 Pseudo SDA for BMP280, connect master GP4 here
#define SLAVE_SCL_PIN 1 // i2c0 Pseudo SCL for BMP280, connect master GP5 here
#define MASTER_SDA_PIN 6 // i2c1 Temp, fake master SDA
#define MASTER_SCL_PIN 7 // i2c1 Temp, fake master SCL


static struct
{
    uint8_t mem[MEM_SIZE];
    uint8_t target_mem_address;
    bool address_written;
} GenericMemoryMap;

static void init_memory_map(){
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

static bool verifyCommand(uint8_t input){
    for (int i = 0; i < sizeof(commands); i++){
        if (input == commands[i]){
            return true;
        }
    }
    return false;
}

static bool verifyReadAllowed(uint8_t input){
    for (int i = 0; i < sizeof(writeOnly); i++){
        if (input == writeOnly[i]){
            return false;
        }
    }
    return true;
}

static bool verifyWriteAllowed(uint8_t input){
    for (int i = 0; i < sizeof(readOnly); i++){
        if (input == readOnly[i]){
            return false;
        }
    }
    return true;
}

// BMP280 slave handler
static void BMP280_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    //printf("Hi\n");
    //printf("Test\n");
    switch (event){
        uint8_t data;
        case I2C_SLAVE_RECEIVE:
            //printf("Recevied \n");
            data = i2c_read_byte_raw(i2c);
            if (GenericMemoryMap.address_written){
                if (verifyWriteAllowed(GenericMemoryMap.target_mem_address)){
                    GenericMemoryMap.mem[GenericMemoryMap.target_mem_address] = data;
                    GenericMemoryMap.target_mem_address++;
                } else {
                    printf("%02X is not allowed to be written to\n");
                }
            } else {
                if (verifyCommand(data)){
                    printf("Received command %02X\n", data);
                } else {
                    GenericMemoryMap.target_mem_address = data;
                    GenericMemoryMap.address_written = true;
                }
            }
            break;
        case I2C_SLAVE_REQUEST:
            printf("Slave receive for %02X\n", GenericMemoryMap.target_mem_address);
            if (verifyReadAllowed(GenericMemoryMap.target_mem_address)){
                i2c_write_byte_raw(i2c, GenericMemoryMap.mem[GenericMemoryMap.target_mem_address]);
                GenericMemoryMap.target_mem_address++;
            } else {
                printf("%02X is not allowed to be read from\n");
            }
            break;
        case I2C_SLAVE_FINISH:
            GenericMemoryMap.address_written = false;
            break;
        default:
            break;
    }
}

static void setup_slave(){
    gpio_init(SLAVE_SDA);
    gpio_set_function(SLAVE_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SDA);
    //gpio_pull_down(SLAVE_SDA_PIN);

    gpio_init(SLAVE_SCL);
    gpio_set_function(SLAVE_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL);
    //gpio_pull_down(SLAVE_SCL_PIN);

    i2c_inst_t *interface;
    if (INTERFACE == 0){
        interface = i2c0;
    } else {
        interface = i2c1;
    }
    i2c_init(interface, I2C_BAUDRATE);

    i2c_slave_init(interface, SLAVE_ADDR, &BMP280_i2c_slave_handler);

    init_memory_map();
}

static void runMaster() {
    gpio_init(MASTER_SDA);
    gpio_set_function(MASTER_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA);

    gpio_init(MASTER_SCL);
    gpio_set_function(MASTER_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL);

    i2c_inst_t *interface;
    if (INTERFACE == 0){
        interface = i2c1;
    } else {
        interface = i2c0;
    }

    i2c_init(interface, I2C_BAUDRATE);
    sleep_ms(1000);
    
    while(1) {
        for (int i = 0; i < sizeof(masterRead); i++){
            uint8_t buf[1] = { 0 };
            uint8_t reg = masterRead[i];
            i2c_write_blocking(interface, DEV_ADDR, &reg, 1, true);
            i2c_read_blocking(interface, DEV_ADDR, buf, 1, false);
            printf("Master read at %02X: %02X\n", reg, buf[0]);
            sleep_ms(1000);
        }
        for (int i = 0; i < sizeof(masterWrite_addr); i++){
            uint8_t buf[2] = {masterWrite_addr[i], masterWrite_value[i]};
            i2c_write_blocking(interface, DEV_ADDR, buf, 2, false);
            printf("Master write value %02X at %02X", buf[0], buf[1]);
            sleep_ms(1000);
        }
    }
}


int main() {
    stdio_init_all(); 

    setup_slave();

    if (CREATE_MASTER == 1){
        //runMaster();
    }

    while(1){
        sleep_ms(1000);
    }
    
    // printf("Dev addr: %02X\n", DEV_ADDR);
    // printf("mem size: %d\n", MEM_SIZE);
    // printf("commands 0: %02X", commands[0]);
    // printf("commands 0: %02X", commands[1]);
    // printf("commands 0: %02X", commands[2]);
    // i2c_inst_t *i2c_instance = i2c0;
}