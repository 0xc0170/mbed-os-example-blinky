
// DISCO F469NI QSPI pinout

// #define QSPI_CS_PIN                GPIO_PIN_6
// #define QSPI_CS_GPIO_PORT          GPIOB
// #define QSPI_CLK_PIN               GPIO_PIN_10
// #define QSPI_CLK_GPIO_PORT         GPIOF
// #define QSPI_D0_PIN                GPIO_PIN_8
// #define QSPI_D1_PIN                GPIO_PIN_9
// #define QSPI_D2_PIN                GPIO_PIN_7
// #define QSPI_D3_PIN                GPIO_PIN_6
// #define QSPI_DX_GPIO_PORT          GPIOF


#include "mbed.h"
#include "qspi_api.h"

#define MACRON_READ_VOLATILE_STATUS_REGISTER = 0x85;
#define MACRON_READ_ENH_VOLATILE_STATUS_REGISTER = 0x65;
#define MACRON_WRITE_VOLATILE_STATUS_REGISTER = 0x81;
#define MACRON_WRITE_ENH_VOLATILE_STATUS_REGISTER = 0x61;
#define MACRON_ENTER_QUAD_MODE = 0x38;

#define MACRO_FLASH_SIZE 0x1000000
#define MACRON_SECTOR_SIZE 0x10000
#define MACRON_PAGE_SIZE 0x100

// write to first page

const uint32_t write_address = 0x10;
const size_t buffer_size = 16;


int main()
{
    qspi_t obj;
    qspi_status_t status = qspi_init(&obj, PF_8, PF_9, PF_7, PF_6, PF_10, PB_6 ,1000000, 0);

    // simple test to one page, we do not need magic to calculate offset etc...

    // write enable
    {
        qspi_command_t command;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.value = 0x06;
        // command.address.bus_width = QSPI_CFG_BUS_SINGLE;
        command.address.size = QSPI_CFG_ADDR_SIZE_NONE;
        // command.address.value = block_address;
        command.alt.size = QSPI_CFG_ALT_SIZE_NONE;
        command.dummy_count = 0;

        qspi_write_command(&obj, &command);
    }

    // erase a block
    {
        qspi_command_t command;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.value = 0x20;
        command.address.bus_width = QSPI_CFG_BUS_SINGLE;
        command.address.size = QSPI_CFG_ADDR_SIZE_24;
        command.address.value = 0;
        command.alt.size = QSPI_CFG_ALT_SIZE_NONE;
        command.dummy_count = 0;

        qspi_write_command(&obj, &command);
    }

    uint8_t WriteBuffer[buffer_size] = "Hello World !!!";
    uint8_t ReadBuffer[buffer_size];

    {
        qspi_command_t command;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.value = 0x12;
        command.address.bus_width = QSPI_CFG_BUS_QUAD;
        command.address.size = QSPI_CFG_ADDR_SIZE_24;
        command.address.value = write_address;
        command.alt.size = QSPI_CFG_ALT_SIZE_NONE;
        command.dummy_count = 0;
        command.data.bus_width = QSPI_CFG_BUS_QUAD;

        size_t size = buffer_size;
        if (qspi_write(&obj, &command, &WriteBuffer, &size) != QSPI_STATUS_OK) {
            error("QSPI write failed");
        }
    }

    {
        qspi_command_t command;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.value = 0x12;
        command.address.bus_width = QSPI_CFG_BUS_QUAD;
        command.address.size = QSPI_CFG_ADDR_SIZE_24;
        command.address.value = write_address;
        command.alt.size = QSPI_CFG_ALT_SIZE_NONE;
        command.dummy_count = 0;
        command.data.bus_width = QSPI_CFG_BUS_QUAD;

        size_t size = buffer_size;
        if (qspi_read(&obj, &command, &ReadBuffer, &size) != QSPI_STATUS_OK) {
            error("QSPI read failed");
        }
    }



    // compare
    if (strcmp((const char *)&WriteBuffer, (const char *)&ReadBuffer) != 0) {
        error("String not matched");
    }
}
