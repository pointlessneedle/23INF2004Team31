#include <stdio.h>
#include <pico/stdlib.h> 
#include <pico/i2c_slave.h>
#include <string.h>
#include <hardware/i2c.h>
#include "pico/time.h"
#include "Generic.h"
#include "BMP280.h"
#include "ht16k33.h"
#include "lcd1602.h"
#include "flashmicro.h"
#include "spimaster.h"

#define DEBOUNCE 300
const uint BUTTON20 = 20; 
const uint BUTTON21 = 21;
const uint BUTTON22 = 22;
uint32_t lastButtonPressTime = 0; //time of lastest time / last time

struct repeating_timer timer;

// Using interger variables to check for selection and iterate through the menus
int mainMenuSelection = 0;
int I2CMenuSelection = 0;
int SPIMenuSelection = 0;

// Using a character variable check 
char state = 'n';

// Using arrays to store options for the menus
char *mainMenuOptions[] = {"I2C Emulation", "SPI Slave Driver", "SPI Master Driver"};
char *I2CMenuOptions[] = {"24c256 Master", "BMP280 Slave", "ht16k33 Slave", "lcd1602 Slave", "Lis3dh Slave", "Generic Slave"};
char *SPIMenuOptions[] = {"BME280 Master"};

char msg[100] = "";

// Functions to print the menus, the index is iterated through and decides which option is printed/selected
void printMainMenu(){
    printf("Main Menu:\n");
    printf("Current Selection: %s \n", mainMenuOptions[mainMenuSelection]);
}

void printI2CMenu(){
    printf("I2C Emulation Menu:\n");
    printf("Current Selection: %s \n", I2CMenuOptions[I2CMenuSelection]);
}

void printSPIMasterMenu(){
    printf("SPI Master Emulation Menu\n");
    printf("Current Selection: %s \n", SPIMenuOptions[SPIMenuSelection]);
}

// void printSPIMasterMenu(){
//     printf("SPI Master Selected\n");
//     exec_flashmicro();
//     // printf("Current Selection: %s \n", I2CMenuOptions[I2CMenuSelection]);
// }

// Functions to iterate through the menus
void mainMenuNext(){
    if (mainMenuSelection < 2)
    {
        mainMenuSelection++;
    }
    else
    {
        mainMenuSelection = 0;
    }
    printMainMenu();   
}

void I2CMenuNext(){
    if (I2CMenuSelection < 5)
    {
        I2CMenuSelection++;
    }
    else
    {
        I2CMenuSelection = 0;
    }
    printI2CMenu();
}

void SPIMasterMenuNext(){
    if (SPIMenuSelection < 1)
    {
        SPIMenuSelection++;
    }
    else
    {
        SPIMenuSelection = 0;
    }
    printSPIMasterMenu();
}


void gpio_callback(uint gpio, uint32_t events) {

    uint32_t currentTime = to_ms_since_boot(get_absolute_time()); 

    if (currentTime - lastButtonPressTime >= DEBOUNCE) {
        lastButtonPressTime = currentTime; // Update the last button press time

        // If Button20 is pressed, the menu is iterated through. The value of the "state" variable decided which menu is iterated and printed
        if (gpio_get(BUTTON20) == 0) {
            switch (state)
            {
            case 'n':
                {
                    mainMenuNext();
                }
                break;
            case 'i':
                {
                    I2CMenuNext();
                }
                break;
            case 'm':
                {
                    printf("yikes\n");     
                }
                break;
            case 's':
                {
                    SPIMasterMenuNext();
                }
                break;
            }   
        }

        // If Button21 is pressed, the current index value ie. the option printed is selected. The value of "state" variable is changed to reflect the new menu/feature selected
        if (gpio_get(BUTTON21) == 0) {
            switch (state)
            {
            case 'n':
                {
                    switch (mainMenuSelection)
                    {
                    case 0:
                        {
                            state = 'i';
                            printI2CMenu();
                        }
                        break;
                    case 1:
                        {
                            state = 's';
                            printSPIMasterMenu();
                        }
                        break;
                    case 2:
                        {
                            state = 'm';
                            printf("SPI Master Selected\n");
                            // Calls the function imported from flashmicro.h, which in turn calls the function in flashmicro.c to run the feature
                            exec_flashmicro();   
                        }
                        break;
                    }
                }
                break;
                case 'i':
                {
                    switch (I2CMenuSelection)
                    {
                    case 0:
                        {
                            printf("This function has not been coded yet.\n");
                        }
                        break;
                    case 1:
                        {
                            printf("Running BMP280 Slave:\n");
                            setup_slave_BMP280();
                            state = 'z';
                        }
                        break;
                    case 2:
                        {
                            printf("Running ht16k33 Slave\n");
                            setup_slave_ht16k33();
                            state = 'y';
                        }
                        break;
                    case 3:
                        {
                            printf("Running lcd1602 Slave:\n");
                            setup_slave_lcd1602();
                            state = 'x';
                        }
                        break;
                    case 4:
                        {
                            printf("This function has not been coded yet.\n");
                        }
                        break;
                    case 5:
                        {
                            printf("Running Generic Slave:\n");
                            setup_slave_Generic();
                            state = 'g';
                            
                        }
                        break;
                    }
                }
                break;
                case 's':
                {
                    switch (SPIMenuSelection) {
                        case 0:
                        {
                            printf("Running SPI master for BME280:\n");
                            exec_bme280_master();
                            state = 's';
                        }
                        break;
                    }
                }
                break;
            }
        }

        // If Button22 is pressed, the submenu currently printed is exited and the parent menu is printed. This change is reflected through the "state" variable. 
        if (gpio_get(BUTTON22) == 0) {
            switch (state)
            {
            case 'n':
                {
                    
                }
                break;
            case 'i':
                {
                    state = 'n';
                    mainMenuSelection = 0;
                    printMainMenu();
                }
                break;
            case 'm':
                {
                    state = 'n';
                    mainMenuSelection = 0;
                    printMainMenu();
                }
                break;
            case 's':
                {
                    state = 'n';
                    mainMenuSelection = 0;
                    printMainMenu();
                }
                break;
            }   
        }
    }
}

bool repeating_timer_callback(struct repeating_timer *t) {

    // The value of the "state" variable decided which i2c_slave_init function to call since the value will reflect which i2c device was selected in the menu. 
    switch (state)
    {
    case 'g':
        {
            i2c_slave_init_Caller_Generic();
        }
        break;
    case 'z':
        {
            i2c_slave_init_Caller_BMP280();
        }
        break;
    case 'y':
        {
            i2c_slave_init_Caller_ht16k33();
        }
        break;
    case 'x':
        {
            i2c_slave_init_Caller_lcd1602();
        }
        break;
}
return true; 
}

int main()
{
    stdio_init_all();
    msg[0] = '\0';

    sleep_ms(3000); 

    // Prints the main menu on startup
    printMainMenu();

    gpio_set_irq_enabled_with_callback(BUTTON20, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback); 
    gpio_set_irq_enabled_with_callback(BUTTON21, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback); 
    gpio_set_irq_enabled_with_callback(BUTTON22, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback); 

    add_repeating_timer_ms(50, repeating_timer_callback, NULL, &timer); 

    while(1); 

    return 0;
}
