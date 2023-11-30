#include <stdio.h>
#include "pico/stdlib.h"
#include "sd_card.h"
#include "ff.h"

void read_file(const char *path) {
    FIL fil;
    FRESULT fr;
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

void list_all_files(const char *path) {
    FRESULT fr;
    DIR dir;
    static FILINFO filinfo;
    
    // Open the given under path
    //
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
            printf("File: %s\r\n", filinfo.fname);
        }
    }
}

void list_and_read_all_files(const char *path) {
    FRESULT fr;
    DIR dir;
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
    FRESULT fr; 
    FIL fil; 
    
    // Open file for writing while creating new file
    //
    fr = f_open(&fil, file_name, FA_WRITE | FA_CREATE_ALWAYS); 
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
 
void write_to_existing_file(char *file_name, char *data) { 
     
    FRESULT fr; 
    FIL fil; 
 
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


int main() {
    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char file_name[] = "test1.txt";

    stdio_init_all();
    sleep_ms(6000);

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

    // Open file for writing
    //
    fr = f_open(&fil, file_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Write something to file
    //
    ret = f_printf(&fil, "This is another test\r\n");
    if (ret < 0) {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true);
    }
    ret = f_printf(&fil, "of writing to an SD card.\r\n");
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

    // Open the newly created file for reading
    //
    fr = f_open(&fil, file_name, FA_READ);
    if (fr != FR_OK) {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true);
    }

    // Print every line/content in the file over serial
    //
    printf("Reading from file '%s':\r\n", file_name);
    printf("---\r\n");
    while (f_gets(buf, sizeof(buf), &fil)) {
        printf(buf);
    }
    printf("\r\n---\r\n");

    // Close file
    //
    fr = f_close(&fil);
    if (fr != FR_OK) {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true);
    }

    // List and read all files in given directory
    //
    printf("READING!!\n");
    list_and_read_all_files("0:");
    
    sleep_ms(10);
    
    // Write to a new file with the given content in 2nd value
    //
    printf("WRITINGGG\n");
    write_to_new_file("test1000.txt","testWRRIIITEE");
    
    sleep_ms(10);
    
    // Unmount drive
    //
    f_unmount("0:");
    // Loop forever doing nothing
    //
    while (true) {
        sleep_ms(1000);        
    }
}