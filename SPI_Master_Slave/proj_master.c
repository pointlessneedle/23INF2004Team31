//=================================================================================//

#include <stdio.h>
#include "hardware/spi.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"

//=================================================================================//

#define BUF_LEN 128

//=================================================================================//

int main() {
  // Enable USB serial so we can print
  stdio_init_all();

  sleep_ms (2 * 1000);
  printf ("SPI Central Example\n");

  // Enable SPI0 at 1 MHz
  spi_init (spi_default, 1 * 1000000);

  // Assign SPI functions to the default SPI pins
  gpio_set_function (PICO_DEFAULT_SPI_RX_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_TX_PIN, GPIO_FUNC_SPI);
  gpio_set_function (PICO_DEFAULT_SPI_CSN_PIN, GPIO_FUNC_SPI);

  // We need two buffers, one for the data to send, and one for the data to receive.
  uint8_t out_buf [BUF_LEN], in_buf [BUF_LEN];

  // Initialize the buffers to 0.
  for (u_int8_t i = 0; i < BUF_LEN; ++i) {
    out_buf [i] = 0;
    in_buf [i] = 0;
  }

  for (uint8_t i = 0; ; ++i) {
    printf ("Sending data %d to SPI Peripheral\n", i);
    out_buf [0] = i;
    // Write the output buffer to COPI, and at the same time read from CIPO to the input buffer.
    spi_write_blocking (spi_default, out_buf, 1);
    // spi_write_read_blocking (spi_default, out_buf, in_buf, 1);

    // Sleep for some seconds so you get a chance to read the output.
    sleep_ms (2 * 1000);
  }
}

//=================================================================================//
