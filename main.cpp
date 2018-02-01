#include "mbed.h"
#include "cmsis_os.h"
#include "PinNames.h"
#include "QSPI.h"

// define what flash we got
#define N25Q128A

#if defined(MX25R6435F)
// Command for reading status register
#define QSPI_STD_CMD_RDSR                   0x05
// Command for writing status register
#define QSPI_STD_CMD_WRSR                   0x01
// Command for reading control register (supported only by some memories)
#define QSPI_STD_CMD_RDCR                   0x35
// Command for writing control register (supported only by some memories)
#define QSPI_STD_CMD_WRCR                   0x3E
// Command for setting Reset Enable (supported only by some memories)
#define QSPI_STD_CMD_RSTEN                  0x66
// Command for setting Reset (supported only by some memories)
#define QSPI_STD_CMD_RST                    0x99
// Command for setting WREN (supported only by some memories)
#define QSPI_STD_CMD_WREN                   0x06
// Command for Sector erase (supported only by some memories)
#define QSPI_STD_CMD_SECT_ERASE             0x20

#elif defined(N25Q128A)

// almost identical than MX25R6435F, just that sector erase is above for subsector erase?

// Command for reading status register
#define QSPI_STD_CMD_RDSR                   0x05
// Command for writing status register
#define QSPI_STD_CMD_WRSR                   0x01
// Command for reading control register (supported only by some memories)
#define QSPI_STD_CMD_RDCR                   0x35
// Command for writing control register (supported only by some memories)
#define QSPI_STD_CMD_WRCR                   0x3E
// Command for setting Reset Enable (supported only by some memories)
#define QSPI_STD_CMD_RSTEN                  0x66
// Command for setting Reset (supported only by some memories)
#define QSPI_STD_CMD_RST                    0x99
// Command for setting WREN (supported only by some memories)
#define QSPI_STD_CMD_WREN                   0x06
// Command for Sector erase (supported only by some memories)
#define QSPI_STD_CMD_SECT_ERASE             0x20
#endif

#if defined (TARGET_NORDIC)
// Read/Write commands
#define QSPI_PP_COMMAND_NRF_ENUM            (0x0) //This corresponds to Flash command 0x02
#define QSPI_READ2O_COMMAND_NRF_ENUM        (0x1) //This corresponds to Flash command 0x3B
#define QSPI_READ2IO_COMMAND_NRF_ENUM       (0x2) //This corresponds to Flash command 0xBB
#define QSPI_PP4IO_COMMAND_NRF_ENUM         (0x3) //This corresponds to Flash command 0x38
#define QSPI_READ4IO_COMMAND_NRF_ENUM       (0x4) //This corresponds to Flash command 0xEB

#else

 // use generic values

#define QSPI_PP_COMMAND_NRF_ENUM            (0x02)
#define QSPI_READ2O_COMMAND_NRF_ENUM        (0x3B)
#define QSPI_READ2IO_COMMAND_NRF_ENUM       (0xBB)
#define QSPI_PP4IO_COMMAND_NRF_ENUM         (0x38) // TODO: cant find this in the datasheet
#define QSPI_READ4IO_COMMAND_NRF_ENUM       (0xEB)

#endif


#if defined(TARGET_DISCO_F469NI)
#define QSPI_PIN_IO0 PF_8
#define QSPI_PIN_IO1 PF_9
#define QSPI_PIN_IO2 PF_7
#define QSPI_PIN_IO3 PF_6
#define QSPI_PIN_SCK PF_10
#define QSPI_PIN_CSN PB_6

#elif defined(TARGET_DISCO_L475VG_IOT01A)

#define QSPI_PIN_IO0 PE_12
#define QSPI_PIN_IO1 PE_13
#define QSPI_PIN_IO2 PE_14
#define QSPI_PIN_IO3 PE_15
#define QSPI_PIN_SCK PE_10
#define QSPI_PIN_CSN PE_11

#endif

#define DEBUG_ON 1
#ifdef DEBUG_ON
    #define VERBOSE_PRINT(x) printf x
#else    
    #define VERBOSE_PRINT(x)
#endif

#define DO_TEST( test )                                 \
    {                                                   \
        printf("\nExecuting test: %-40s :", #test );    \
        if( false == test() ) {                         \
            printf(" FAILED" );                         \
        } else {                                        \
            printf(" PASSED" );                         \
        }                                               \
    }                                                   \

QSPI *myQspi = NULL;
QSPI *myQspiOther = NULL;
#define _1_K_ (0x400)
#define _4_K_ (_1_K_ * 4)
    
bool InitializeFlashMem();
bool WaitForMemReady();
bool SectorErase(unsigned int flash_addr);
bool TestWriteReadSimple();
bool TestWriteReadBlockMultiplePattern();
bool TestWriteSingleReadMultiple();
bool TestWriteMultipleReadSingle();
bool TestWriteReadMultipleObjects();
bool TestWriteReadCustomCommands();
    
// main() runs in its own thread in the OS
int main() {
    myQspi = new QSPI((PinName)QSPI_PIN_IO0, (PinName)QSPI_PIN_IO1, (PinName)QSPI_PIN_IO2, (PinName)QSPI_PIN_IO3, (PinName)QSPI_PIN_SCK, (PinName)QSPI_PIN_CSN);        
    if(myQspi) {
        printf("\nCreated QSPI driver object succesfully");
    } else {
        printf("\nERROR: Failed creating QSPI driver object");
        return -1;
    }
    
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_1 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_1");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_8, QSPI_CFG_BUS_SINGLE, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    if(NULL != myQspi)    
        delete myQspi;
    if(NULL != myQspiOther)
        delete myQspiOther;
    
    printf("\nDone...\n");
}

bool TestWriteReadSimple()
{
    // TODO from the table for this flash, seems like 90 for fast read is supported
    myQspi->set_frequency(9000000);
    int result = 0;
    char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
    char rx_buf[16];    
    size_t buf_len = sizeof(tx_buf);
    
    uint32_t flash_addr = 0x100;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write(0x02, 0, flash_addr, tx_buf, &buf_len );
    if( ( result != QSPI_STATUS_OK ) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed. result = %d, bu_len= %lu", result, buf_len);
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    // requires dummy cycle
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_8, QSPI_CFG_BUS_SINGLE, 8, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }

    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read(0x0B, 0, flash_addr, rx_buf, &buf_len );
    if( result != QSPI_STATUS_OK ) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != sizeof(rx_buf) ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( rx_buf, tx_buf, sizeof(rx_buf)))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    }
    
    return true;
}

bool InitializeFlashMem()
{
    bool ret_status = true;
    char status_value[2] = {0};
    
    //Read the Status Register from device
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RDSR, // command to send
                              0,                 // do not transmit
                              NULL,              // do not transmit
                              status_value,                 // just receive two bytes of data
                              1)) {   // store received values in status_value
        VERBOSE_PRINT(("\nReading Status Register Success: value = 0x%02X:0x%02X\n", status_value[0], status_value[1]));
    } else {
        printf("\nERROR: Reading Status Register failed\n");
        ret_status = false;
    }
    
    if(ret_status)
    {
        //Send Reset Enable
        if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RSTEN, // command to send
                                  0,                 // do not transmit
                                  NULL,              // do not transmit
                                  0,                 // just receive two bytes of data
                                  NULL)) {   // store received values in status_value
            VERBOSE_PRINT(("\nSending RSTEN Success\n"));
        } else {
            printf("\nERROR: Sending RSTEN failed\n");
            ret_status = false;
        }
        

        if(ret_status)
        {
            //Send Reset
            if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RST, // command to send
                                      0,                 // do not transmit
                                      NULL,              // do not transmit
                                      status_value,                 // just receive two bytes of data
                                      2)) {   // store received values in status_value
                VERBOSE_PRINT(("\nSending RST Success\n"));
            } else {
                printf("\nERROR: Sending RST failed\n");
                ret_status = false;
            }

            WaitForMemReady();

            // if(ret_status)
            // {
            //     status_value[0] |= 0x80;
            //     //Write the Status Register to set write enable bit
            //     if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_WRSR, // command to send
            //                               &status_value[0],                 
            //                               1,      
            //                               NULL,                 
            //                               0)) {   // store received values in status_value
            //         VERBOSE_PRINT(("\nWriting Status Register Success\n"));
            //     } else {
            //         printf("\nERROR: Writing Status Register failed\n");
            //         ret_status = false;
            //     }
            // }
        }
    }


    status_value[0] = 0;
    status_value[1] = 0;

    if(ret_status)
    {
        //read VCR
        if (QSPI_STATUS_OK == myQspi->command_transfer(0x85, // command to send
                                  0,                 // do not transmit
                                  NULL,              // do not transmit
                                  &status_value[0],                 // just receive two bytes of data
                                  1)) {   // store received values in status_value
            VERBOSE_PRINT(("\nReceiving VCR Success\n"));
            printf("Read VCR: %d \n", status_value[0]);
        } else {
            printf("\nERROR: Receiving VCR failed\n");
            ret_status = false;
        }

        if(ret_status)
        {
            status_value[0] &= 0x0F; // set to 0 first
            status_value[0] |= 8 << 4; // 7:4 in VCR

            if (QSPI_STATUS_OK == myQspi->command_transfer(0x81, // command to send
                                      &status_value[0],                 
                                      1,      
                                      NULL,                 
                                      0)) {   // store received values in status_value
                VERBOSE_PRINT(("\nWriting VCR dummy cycle Success\n"));
            } else {
                printf("\nERROR: Writing VCR dummy cycle failed\n");
                ret_status = false;
            }
        }

        if (ret_status) {
            //read VCR
            if (QSPI_STATUS_OK == myQspi->command_transfer(0x85, // command to send
                                      0,                 // do not transmit
                                      NULL,              // do not transmit
                                      &status_value[0],                 // just receive two bytes of data
                                      1)) {   // store received values in status_value
                if (((uint8_t)status_value[0] >> 4) != 0x8) { 
                    VERBOSE_PRINT(("\n Failed to write VCR\n"));
                    printf("VCR: %d \n", status_value[0]);
                } else {
                    VERBOSE_PRINT(("\nReceiving VCR Success\n"));
                }
            } else {
                printf("\nERROR: Receiving VCR failed\n");
                ret_status = false;
            }
            
        }
    }
    
    return ret_status;
}

bool WaitForMemReady()
{
    char status_value[2];
    int retries = 0;
    
    do
    {
        retries++;
        //Read the Status Register from device
        if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RDSR, // command to send
                                  0,                 // do not transmit
                                  NULL,              // do not transmit
                                  status_value,                 // just receive two bytes of data
                                  2)) {   // store received values in status_value
            VERBOSE_PRINT(("\nReadng Status Register Success: value = 0x%02X:0x%02X\n", status_value[0], status_value[1]));
        } else {
            printf("\nERROR: Reading Status Register failed\n");
        }
    } while( (status_value[0] & 0x1) != 0 && retries <10000 );
    
    if((status_value[0] & 0x1) != 0) return false;
    return true;
}

bool SectorErase(unsigned int flash_addr)
{
    char addrbytes[3] = {0};
    
    addrbytes[2]=flash_addr & 0xFF;
    addrbytes[1]=(flash_addr >> 8) & 0xFF;
    addrbytes[0]=(flash_addr >> 16) & 0xFF;
            
    //Send WREN
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_WREN, // command to send
                              0,                 // do not transmit
                              NULL,              // do not transmit
                              0,                 // just receive two bytes of data
                              NULL)) {   // store received values in status_value
        VERBOSE_PRINT(("\nSending WREN command success\n"));
    } else {
        printf("\nERROR: Sending WREN command failed\n");
        return false;
    }

    WaitForMemReady();

    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_SECT_ERASE, // command to send
                              addrbytes,                 // do not transmit
                              3,              // do not transmit
                              0,                 // just receive two bytes of data
                              NULL)) {   // store received values in status_value
        VERBOSE_PRINT(("\nSending SECT_ERASE command success\n"));
    } else {
        printf("\nERROR: Readng SECT_ERASE command failed\n");
        return false;
    }
    
    WaitForMemReady();

    return true;
}
