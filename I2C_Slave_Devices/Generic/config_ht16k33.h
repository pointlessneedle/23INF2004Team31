#define MEM_SIZE 256
#define DEV_ADDR 0x76
#define SLAVE_SDA 0
#define SLAVE_SCL 1
#define INTERFACE 0

// CREATE_MASTER: 1 to run a master, 0 to have no master
#define CREATE_MASTER 1
#define MASTER_SDA 6
#define MASTER_SCL 7

// Addresses that the slave device will regard as a command
static uint8_t commands[] = {0x20, 0x21, 0xA0, 0xE0, 0x80, 0x0, 0x1, 0x2, 0x4, 0x6, 0xA0, 0xE0, 0xD9};

// All registers in the slave device
static uint8_t registers[] = {};
static uint8_t reg_value[] = {};

// Blacklist of addresses for reading and writing
static uint8_t readOnly[] = {0x40, 0x60};
static uint8_t writeOnly[] = {0x20, 0x80, 0xA0, 0xE0, 0xD9};

// Data that the master will send
static uint8_t masterRead[] = {};

// Both sizes must be equal
static uint8_t masterWrite_addr[] = {};
static uint8_t masterWrite_value[] = {};
