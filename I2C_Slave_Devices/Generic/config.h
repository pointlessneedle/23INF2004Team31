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
static uint8_t commands[] = {0x20, 0x21, 0xA0, 0xE0, 0x80, 0x0, 0x1, 0x2, 0x4, 0x6};

// All registers in the slave device
static uint8_t registers[] = {0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7, 0xF5, 0xF4, 0xF3, 0xE0, 0xD0, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F};
static uint8_t reg_value[] = {0x00, 0xED, 0x7E, 0xC0, 0x5A, 0x65, 0x00, 0x58, 0x00, 0x00, 0x58, 0x70, 0x6B, 0x43, 0x67, 0x18, 0xFC, 0x7D, 0x8E, 0x43, 0xD6, 0xD0, 0x0B, 0x27, 0x0B, 0x8C, 0x00, 0xF9, 0xFF, 0x8C, 0x3C, 0xF8, 0xC6, 0x70, 0x17};

// Blacklist of addresses for reading and writing
static uint8_t readOnly[] = {};
static uint8_t writeOnly[] = {};

// Data that the master will send
static uint8_t masterRead[] = {0xFC, 0xFB, 0xFA};

// Both sizes must be equal
static uint8_t masterWrite_addr[] = {0x00, 0x01, 0x02};
static uint8_t masterWrite_value[] = {0xA0, 0xA1, 0xA2};
