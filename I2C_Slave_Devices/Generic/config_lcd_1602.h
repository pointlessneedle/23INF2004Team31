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
static uint8_t commands[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

// All registers in the slave device
static uint8_t registers[] = {};
static uint8_t reg_value[] = {};

// Blacklist of addresses for reading and writing
static uint8_t disallowRead[] = {};
static uint8_t disallowWrite[] = {};

// Data that the master will send
static uint8_t masterRead[] = {};

// Both sizes must be equal
static uint8_t masterWrite_addr[] = {};
static uint8_t masterWrite_value[] = {};
