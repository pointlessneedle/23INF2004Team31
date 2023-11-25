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
static uint8_t commands[] = {};

// All registers in the slave device
static uint8_t registers[] = {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x38, 0x39, 0x3A};
static uint8_t reg_value[] = {0x00, 0xED, 0x7E, 0xC0, 0x5A, 0x65, 0x00, 0x58, 0x00, 0x00, 0x58, 0x70, 0x6B, 0x43, 0x67, 0x18, 0xFC, 0x7D, 0x8E, 0x43, 0xD6, 0xD0, 0x0B, 0x27, 0x0B, 0x8C, 0x00, 0xF9, 0xFF, 0x8C, 0x3C, 0xF8, 0xC6};

// Blacklist of addresses for reading and writing
static uint8_t disallowRead[] = {};
static uint8_t disallowWrite[] = {};

// Data that the master will send
static uint8_t masterRead[] = {};

// Both sizes must be equal
static uint8_t masterWrite_addr[] = {};
static uint8_t masterWrite_value[] = {};
