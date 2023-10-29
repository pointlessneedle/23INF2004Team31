#include <stdio.h>
#include <pico/stdlib.h> 
#include <hardware/i2c.h>
#include <string.h>

#define ADDR _u(0x50)
#define ADDR_WRITE _u(0x50)
#define ADDR_READ _u(0x58)
static const uint I2C_BAUDRATE = 100 * 1000;

#define SDA_PIN 0
#define SCL_PIN 1

int main() {
    stdio_init_all();

    gpio_init(SDA_PIN);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);

    gpio_init(SCL_PIN);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SCL_PIN);
    
    i2c_init(i2c0, I2C_BAUDRATE);

    uint8_t first_address = 0x00;
    uint8_t second_address = 0x00;
    char msg[100];
    snprintf(msg, sizeof(msg), "Hallelujah a potato has spawned from the sky");
    uint8_t msg_len = strlen(msg);

    uint8_t buf[100];
    buf[0] = first_address;
    buf[1] = second_address;
    memcpy(buf + 2, msg, msg_len);
    int result = i2c_write_blocking(i2c0, ADDR, buf, msg_len + 2, false);
    sleep_ms(1000);
    printf("Sent the whole string, result = %d\n", result);
    sleep_ms(1000);

    // for (int i = 0; i < msg_len; i++){
    //     sleep_ms(500);
    //     uint8_t buffer[3] = {first_address, second_address, msg[i]};
    //     int result = i2c_write_blocking(i2c0, ADDR, buffer, 3, false);
    //     if (result == -2){
    //         printf("Retrying for %d", i);
    //         i--;
    //         continue;
    //     }
    //     printf("Added: %c at address 0x%02X%02X\n", msg[i], first_address, second_address);
    //     second_address++;
    // }

    while(1){
        second_address = 0x00;
        for (int i = 0; i < msg_len; i++){
            sleep_ms(500);
            uint8_t buffer[2] = {first_address, second_address};
            i2c_write_blocking(i2c0, ADDR, buffer, 2, true);  
            uint8_t output[2];          
            int count = i2c_read_blocking(i2c0, ADDR, output, 1, false);
            output[1] = '\0';
            printf("Count: %d, Output: %s from address 0x%02X%02X\n", count, output, first_address, second_address);
            second_address++;
        }
        printf("\n");
    }   

    // for (int i = 65; i < 75; i++){
    //     sleep_ms(1000);
    //     uint8_t buffer[3] = {first_address, second_address, i};
    //     int result = i2c_write_blocking(i2c0, ADDR, buffer, 3, false);
    //     if (result == -2){
    //         printf("Retrying for %d", i);
    //         i--;
    //         continue;
    //     }
    //     printf("Result: %d\n", result);
    //     second_address++;
    // }

    // while(1){
    //     second_address = 0x00;
    //     for (int i = 0; i < 10; i++){
    //         sleep_ms(500);
    //         uint8_t buffer[2] = {first_address, second_address};
    //         i2c_write_blocking(i2c0, ADDR, buffer, 2, true);  
    //         uint8_t output[2];          
    //         int count = i2c_read_blocking(i2c0, ADDR, output, 1, false);
    //         output[1] = '\0';
    //         printf("Count: %d, Output: %s from address 0x%02X%02X\n", count, output, first_address, second_address);
    //         second_address++;
    //     }
    //     printf("\n");
    // }   
    return 0;
}