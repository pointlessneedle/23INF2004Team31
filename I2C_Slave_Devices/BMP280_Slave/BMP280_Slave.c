#include <stdio.h>
#include <pico/stdlib.h> 
#include <pico/i2c_slave.h>
#include <string.h>
#include <hardware/i2c.h>
#include "pico/time.h"

// This slave device only mimics I2C 

// I2C slave address
//static const uint SLAVE_ADDR = 0x76;
#define SLAVE_ADDR _u(0x76)
static const uint I2C_BAUDRATE = 100 * 1000;

#define SLAVE_SDA_PIN 0 // i2c0 Pseudo SDA for BMP280, connect master GP4 here
#define SLAVE_SCL_PIN 1 // i2c0 Pseudo SCL for BMP280, connect master GP5 here
#define MASTER_SDA_PIN 6 // i2c1 Temp, fake master SDA
#define MASTER_SCL_PIN 7 // i2c1 Temp, fake master SCL

// BMP280 registers
#define REG_TEMP_XLSB _u(0xFC)
#define REG_TEMP_LSB _u(0xFB)
#define REG_TEMP_MSB _u(0xFA)

uint8_t temp_xlsb = 0x00;
uint8_t temp_lsb = 0xED;
uint8_t temp_msb = 0x7E;

#define REG_PRESSURE_XLSB _u(0xF9)
#define REG_PRESSURE_LSB _u(0xF8)
#define REG_PRESSURE_MSB _u(0xF7)

uint8_t pressure_xlsb = 0xC0;
uint8_t pressure_lsb = 0x5A;
uint8_t pressure_msb = 0x65;

#define REG_CONFIG _u(0xF5)
#define REG_CTRL_MEAS _u(0xF4)
#define REG_STATUS _u(0xF3)
#define REG_RESET _u(0xE0)
#define REG_ID _u(0xD0)

uint8_t reset = 0x00; // reset triggered on 0xB6
uint8_t id = 0x58;

// calibration registers
#define REG_DIG_T1_LSB _u(0x88)
#define REG_DIG_T1_MSB _u(0x89)
#define REG_DIG_T2_LSB _u(0x8A)
#define REG_DIG_T2_MSB _u(0x8B)
#define REG_DIG_T3_LSB _u(0x8C)
#define REG_DIG_T3_MSB _u(0x8D)
#define REG_DIG_P1_LSB _u(0x8E)
#define REG_DIG_P1_MSB _u(0x8F)
#define REG_DIG_P2_LSB _u(0x90)
#define REG_DIG_P2_MSB _u(0x91)
#define REG_DIG_P3_LSB _u(0x92)
#define REG_DIG_P3_MSB _u(0x93)
#define REG_DIG_P4_LSB _u(0x94)
#define REG_DIG_P4_MSB _u(0x95)
#define REG_DIG_P5_LSB _u(0x96)
#define REG_DIG_P5_MSB _u(0x97)
#define REG_DIG_P6_LSB _u(0x98)
#define REG_DIG_P6_MSB _u(0x99)
#define REG_DIG_P7_LSB _u(0x9A)
#define REG_DIG_P7_MSB _u(0x9B)
#define REG_DIG_P8_LSB _u(0x9C)
#define REG_DIG_P8_MSB _u(0x9D)
#define REG_DIG_P9_LSB _u(0x9E)
#define REG_DIG_P9_MSB _u(0x9F)

// temperature params
uint8_t dig_t1_lsb = 0x70;
uint8_t dig_t1_msb = 0x6B;
int8_t dig_t2_lsb = 0x43;
int8_t dig_t2_msb = 0x67;
int8_t dig_t3_lsb = 0x18;
int8_t dig_t3_msb = 0xFC;

// pressure params
uint8_t dig_p1_lsb = 0x7D;
uint8_t dig_p1_msb = 0x8E;
int8_t dig_p2_lsb = 0x43;
int8_t dig_p2_msb = 0xD6;
int8_t dig_p3_lsb = 0xD0;
int8_t dig_p3_msb = 0x0B;
int8_t dig_p4_lsb = 0x27;
int8_t dig_p4_msb = 0x0B;
int8_t dig_p5_lsb = 0x8C;
int8_t dig_p5_msb = 0x00;
int8_t dig_p6_lsb = 0xF9;
int8_t dig_p6_msb = 0xFF;
int8_t dig_p7_lsb = 0x8C;
int8_t dig_p7_msb = 0x3C;
int8_t dig_p8_lsb = 0xF8;
int8_t dig_p8_msb = 0xC6;
int8_t dig_p9_lsb = 0x70;
int8_t dig_p9_msb = 0x17;

// (Master) number of calibration registers to be read
#define NUM_CALIB_PARAMS 24

// (Master) For master to store calibration params
struct bmp280_calib_param {
    // temperature params
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;

    // pressure params
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
};

static struct
{
    uint8_t mem[256];
    uint8_t target_mem_address;
    bool address_written;
} BMPMemoryMap;

static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;

// Initialise the memory map of BMP280
static void init_memory_map(){
    // Initialising Temperature and Pressure
    BMPMemoryMap.mem[REG_TEMP_XLSB] = temp_xlsb;
    BMPMemoryMap.mem[REG_TEMP_LSB] = temp_lsb;
    BMPMemoryMap.mem[REG_TEMP_MSB] = temp_msb;

    BMPMemoryMap.mem[REG_PRESSURE_XLSB] = pressure_xlsb;
    BMPMemoryMap.mem[REG_PRESSURE_LSB] = pressure_lsb;
    BMPMemoryMap.mem[REG_PRESSURE_MSB] = pressure_msb;

    // Setting id and reset
    BMPMemoryMap.mem[REG_RESET] = reset;
    BMPMemoryMap.mem[REG_ID] = id;

    // Initialising Calibration registers
    BMPMemoryMap.mem[REG_DIG_T1_LSB] = dig_t1_lsb;
    BMPMemoryMap.mem[REG_DIG_T1_MSB] = dig_t1_msb;
    BMPMemoryMap.mem[REG_DIG_T2_LSB] = dig_t2_lsb;
    BMPMemoryMap.mem[REG_DIG_T2_MSB] = dig_t2_msb;
    BMPMemoryMap.mem[REG_DIG_T3_LSB] = dig_t3_lsb;
    BMPMemoryMap.mem[REG_DIG_T3_MSB] = dig_t3_msb;
    BMPMemoryMap.mem[REG_DIG_P1_LSB] = dig_p1_lsb;
    BMPMemoryMap.mem[REG_DIG_P1_MSB] = dig_p1_msb;
    BMPMemoryMap.mem[REG_DIG_P2_LSB] = dig_p2_lsb;
    BMPMemoryMap.mem[REG_DIG_P2_MSB] = dig_p2_msb;
    BMPMemoryMap.mem[REG_DIG_P3_LSB] = dig_p3_lsb;
    BMPMemoryMap.mem[REG_DIG_P3_MSB] = dig_p3_msb;
    BMPMemoryMap.mem[REG_DIG_P4_LSB] = dig_p4_lsb;
    BMPMemoryMap.mem[REG_DIG_P4_MSB] = dig_p4_msb;
    BMPMemoryMap.mem[REG_DIG_P5_LSB] = dig_p5_lsb;
    BMPMemoryMap.mem[REG_DIG_P5_MSB] = dig_p5_msb;
    BMPMemoryMap.mem[REG_DIG_P6_LSB] = dig_p6_lsb;
    BMPMemoryMap.mem[REG_DIG_P6_MSB] = dig_p6_msb;
    BMPMemoryMap.mem[REG_DIG_P7_LSB] = dig_p7_lsb;
    BMPMemoryMap.mem[REG_DIG_P7_MSB] = dig_p7_msb;
    BMPMemoryMap.mem[REG_DIG_P8_LSB] = dig_p8_lsb;
    BMPMemoryMap.mem[REG_DIG_P8_MSB] = dig_p8_msb;
    BMPMemoryMap.mem[REG_DIG_P9_LSB] = dig_p9_lsb;
    BMPMemoryMap.mem[REG_DIG_P9_MSB] = dig_p9_msb;
}

// BMP280 slave handler
static void BMP280_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    //printf("Hi\n");
    switch (event){
        case I2C_SLAVE_RECEIVE:
            printf("Recevied \n");
            if (BMPMemoryMap.address_written){
                if (BMPMemoryMap.target_mem_address == REG_CONFIG || BMPMemoryMap.target_mem_address == REG_CTRL_MEAS || BMPMemoryMap.target_mem_address == REG_RESET){
                    BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] = i2c_read_byte_raw(i2c);
                }
            } else {
                BMPMemoryMap.target_mem_address = i2c_read_byte_raw(i2c);
                printf("%02x", BMPMemoryMap.target_mem_address);
                BMPMemoryMap.address_written = true;
            } 
            break;
        case I2C_SLAVE_REQUEST:
            if (BMPMemoryMap.target_mem_address != REG_RESET){
                i2c_write_byte_raw(i2c, BMPMemoryMap.mem[BMPMemoryMap.target_mem_address]);
                printf("Slave sending: %02x, from address: %02x\n", BMPMemoryMap.mem[BMPMemoryMap.target_mem_address], BMPMemoryMap.target_mem_address);

                // Increases or decreases the LSB of pressure and temperature with a range of 1-5;
                if (BMPMemoryMap.target_mem_address == REG_PRESSURE_LSB || BMPMemoryMap.target_mem_address == REG_TEMP_LSB){
                    int decision, amount;
                    decision = rand() % 2;
                    amount = rand() % 5 + 1;
                    if (decision){
                        BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] += amount;
                    }else{
                        BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] -= amount;
                    }
                }
                BMPMemoryMap.target_mem_address++;
            }
            break;
        case I2C_SLAVE_FINISH:
            BMPMemoryMap.address_written = false;
            break;
        default:
            break;
    }
}

// Old, can remove when necessary
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    // while(1){
    //     sleep_ms(1000);
    //     printf("handler\n");
    // }
    switch (event) {
        // uint8_t slave_data;
        case I2C_SLAVE_RECEIVE: // master has written some data
            // slave_data = i2c_read_byte(i2c);
            // printf("Slave received: %s\n", slave_data);
            if (!context.mem_address_written) {
                // writes always start with the memory address
                context.mem_address = i2c_read_byte_raw(i2c);
                context.mem_address_written = true;
            } else {
                // save into memory
                context.mem[context.mem_address] = i2c_read_byte_raw(i2c);
                // printf("Slave received: %02x\n", context.mem[context.mem_address]);
                context.mem_address++;
            }            
            break;
        case I2C_SLAVE_REQUEST: // master is requesting data
            // load from memory
            //printf("Slave sending %s", dataToSend);
            //i2c_write_byte(i2c, dataToSend);
            i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
            // printf("Slave sending %02x\n", context.mem[context.mem_address]);
            context.mem_address++;
            break;
        case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
            context.mem_address_written = false;
            // printf("Slave done with printing\n");
            break;
        default:
            break;
    }
}

static void setup_slave(){
    gpio_init(SLAVE_SDA_PIN);
    gpio_set_function(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SDA_PIN);
    //gpio_pull_down(SLAVE_SDA_PIN);

    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);
    //gpio_pull_down(SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);

    i2c_slave_init(i2c0, SLAVE_ADDR, &BMP280_i2c_slave_handler);

    init_memory_map();
}

// From bmp280_i2c
void bmp280_init() {
    // use the "handheld device dynamic" optimal setting (see datasheet)
    uint8_t buf[2];

    // 500ms sampling time, x16 filter
    const uint8_t reg_config_val = ((0x04 << 5) | (0x05 << 2)) & 0xFC;

    // send register number followed by its corresponding value
    buf[0] = REG_CONFIG;
    buf[1] = reg_config_val;
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);

    // osrs_t x1, osrs_p x4, normal mode operation
    const uint8_t reg_ctrl_meas_val = (0x01 << 5) | (0x03 << 2) | (0x03);
    buf[0] = REG_CTRL_MEAS;
    buf[1] = reg_ctrl_meas_val;
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);
}

// From bmp280_i2c
void bmp280_read_raw(int32_t* temp, int32_t* pressure) {
    // BMP280 data registers are auto-incrementing and we have 3 temperature and
    // pressure registers each, so we start at 0xF7 and read 6 bytes to 0xFC
    // note: normal mode does not require further ctrl_meas and config register writes

    uint8_t buf[6];
    uint8_t reg = REG_PRESSURE_MSB;
    i2c_write_blocking(i2c1, SLAVE_ADDR, &reg, 1, true);  // true to keep master control of bus
    i2c_read_blocking(i2c1, SLAVE_ADDR, buf, 6, false);  // false - finished with bus

    // store the 20 bit read in a 32 bit signed integer for conversion
    *pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
}

// From bmp280_i2c
void bmp280_reset() {
    // reset the device with the power-on-reset procedure
    uint8_t buf[2] = { REG_RESET, 0xB6 };
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);
}

// (From bmp280_i2c)intermediate function that calculates the fine resolution temperature
// used for both pressure and temperature conversions
int32_t bmp280_convert(int32_t temp, struct bmp280_calib_param* params) {
    // use the 32-bit fixed point compensation implementation given in the
    // datasheet
    int32_t var1, var2;
    var1 = ((((temp >> 3) - ((int32_t)params->dig_t1 << 1))) * ((int32_t)params->dig_t2)) >> 11;
    var2 = (((((temp >> 4) - ((int32_t)params->dig_t1)) * ((temp >> 4) - ((int32_t)params->dig_t1))) >> 12) * ((int32_t)params->dig_t3)) >> 14;
    return var1 + var2;
}

// From bmp280_i2c
int32_t bmp280_convert_temp(int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the temperature value read from its registers
    int32_t t_fine = bmp280_convert(temp, params);
    return (t_fine * 5 + 128) >> 8;
}

// From bmp280_i2c
int32_t bmp280_convert_pressure(int32_t pressure, int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the pressure value read from its registers

    int32_t t_fine = bmp280_convert(temp, params);

    int32_t var1, var2;
    uint32_t converted = 0.0;
    var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)params->dig_p6);
    var2 += ((var1 * ((int32_t)params->dig_p5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)params->dig_p4) << 16);
    var1 = (((params->dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)params->dig_p2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)params->dig_p1)) >> 15);
    if (var1 == 0) {
        return 0;  // avoid exception caused by division by zero
    }
    converted = (((uint32_t)(((int32_t)1048576) - pressure) - (var2 >> 12))) * 3125;
    if (converted < 0x80000000) {
        converted = (converted << 1) / ((uint32_t)var1);
    } else {
        converted = (converted / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)params->dig_p9) * ((int32_t)(((converted >> 3) * (converted >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(converted >> 2)) * ((int32_t)params->dig_p8)) >> 13;
    converted = (uint32_t)((int32_t)converted + ((var1 + var2 + params->dig_p7) >> 4));
    return converted;
}

// From bmp280_i2c
void bmp280_get_calib_params(struct bmp280_calib_param* params) {
    // raw temp and pressure values need to be calibrated according to
    // parameters generated during the manufacturing of the sensor
    // there are 3 temperature params, and 9 pressure params, each with a LSB
    // and MSB register, so we read from 24 registers

    uint8_t buf[NUM_CALIB_PARAMS] = { 0 };
    uint8_t reg = REG_DIG_T1_LSB;
    i2c_write_blocking(i2c1, SLAVE_ADDR, &reg, 1, true);  // true to keep master control of bus
    // read in one go as register addresses auto-increment
    i2c_read_blocking(i2c1, SLAVE_ADDR, buf, NUM_CALIB_PARAMS, false);  // false, we're done reading

    // store these in a struct for later use
    params->dig_t1 = (uint16_t)(buf[1] << 8) | buf[0];
    params->dig_t2 = (int16_t)(buf[3] << 8) | buf[2];
    params->dig_t3 = (int16_t)(buf[5] << 8) | buf[4];

    params->dig_p1 = (uint16_t)(buf[7] << 8) | buf[6];
    params->dig_p2 = (int16_t)(buf[9] << 8) | buf[8];
    params->dig_p3 = (int16_t)(buf[11] << 8) | buf[10];
    params->dig_p4 = (int16_t)(buf[13] << 8) | buf[12];
    params->dig_p5 = (int16_t)(buf[15] << 8) | buf[14];
    params->dig_p6 = (int16_t)(buf[17] << 8) | buf[16];
    params->dig_p7 = (int16_t)(buf[19] << 8) | buf[18];
    params->dig_p8 = (int16_t)(buf[21] << 8) | buf[20];
    params->dig_p9 = (int16_t)(buf[23] << 8) | buf[22];
}

// BMP 280 master (Taken from bmp280_i2c)
static void run_BMP280_master(){
    gpio_init(MASTER_SDA_PIN);
    gpio_set_function(MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA_PIN);

    gpio_init(MASTER_SCL_PIN);
    gpio_set_function(MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL_PIN);
    
    i2c_init(i2c1, I2C_BAUDRATE);

    // configure BMP280
    bmp280_init();

    // retrieve fixed compensation params
    struct bmp280_calib_param params;
    bmp280_get_calib_params(&params);

    int32_t raw_temperature;
    int32_t raw_pressure;

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        bmp280_read_raw(&raw_temperature, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temperature, &params);
        printf("Pressure = %.3f kPa\n", pressure / 1000.f);
        printf("Temp. = %.2f C\n", temperature / 100.f);
        // poll every 500ms
        sleep_ms(500);
    }
}

// (From slave_mem) To fake a master
static void run_master(){
    gpio_init(MASTER_SDA_PIN);
    gpio_set_function(MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA_PIN);

    gpio_init(MASTER_SCL_PIN);
    gpio_set_function(MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL_PIN);
    
    i2c_init(i2c0, I2C_BAUDRATE);
    
    for (uint8_t mem_address = 0;; mem_address = (mem_address + 32) % 256) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Hello, I2C slave! - 0x%02X", mem_address);
        uint8_t msg_len = strlen(msg);

        uint8_t buf[32];
        buf[0] = mem_address;
        memcpy(buf + 1, msg, msg_len);
        // write message at mem_address
        printf("Master write at 0x%02X: '%s'\n", mem_address, msg);
        int count = i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 1 + msg_len, false);
        printf("Master count: %d\n", count);
        if (count < 0) {
            puts("Couldn't write to slave, please check your wiring!");
            return;
        }
        hard_assert(count == 1 + msg_len);
        
        // seek to mem_address
        count = i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 1, true);
        hard_assert(count == 1);
        // partial read
        uint8_t split = 5;
        count = i2c_read_blocking(i2c1, SLAVE_ADDR, buf, split, true);
        hard_assert(count == split);
        buf[count] = '\0';
        printf("Master partial read  at 0x%02X: '%s'\n", mem_address, buf);
        hard_assert(memcmp(buf, msg, split) == 0);
        // read the remaining bytes, continuing from last address
        count = i2c_read_blocking(i2c1, SLAVE_ADDR, buf, msg_len - split, false);
        hard_assert(count == msg_len - split);
        buf[count] = '\0';
        printf("Master remaining read  at 0x%02X: '%s'\n", mem_address + split, buf);
        hard_assert(memcmp(buf, msg + split, msg_len - split) == 0);

        puts("");
        sleep_ms(2000);
    }
}

int main() {
    stdio_init_all(); 
    
    setup_slave();
    //run_master();
    run_BMP280_master();
    
    while(1){
        // sleep_ms(1000);
        // printf("%02x\n", SLAVE_ADDR);
    }

    return 0;
}