
#define HAL_TEST 0

#if HAL_TEST == 1

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

    // write status register
    {
        qspi_command_t command;
        command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
        command.instruction.value = 0x01;
        // command.address.bus_width = QSPI_CFG_BUS_SINGLE;
        command.address.size = QSPI_CFG_ADDR_SIZE_NONE;
        // command.address.value = block_address;
        command.alt.size = QSPI_CFG_ALT_SIZE_NONE;
        command.dummy_count = 0;

        qspi_write_command(&obj, &command);
    }

    // read status register 
    {
            qspi_command_t command;
            command.instruction.bus_width = QSPI_CFG_BUS_SINGLE;
            command.instruction.value = 0x05;
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

#else

#include "mbed.h"
#include "cmsis_os.h"
#include "PinNames.h"
#include "QSPI.h"

#define QSPI_PIN_IO0 PF_8
#define QSPI_PIN_IO1 PF_9
#define QSPI_PIN_IO2 PF_7
#define QSPI_PIN_IO3 PF_6
#define QSPI_PIN_SCK PF_10
#define QSPI_PIN_CSN PB_6

// The below values are command codes defined in Datasheet for MX25R6435F Macronix Flash Memory
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
// Read/Write commands
#define QSPI_PP_COMMAND_NRF_ENUM            (0x0) //This corresponds to Flash command 0x02
#define QSPI_READ2O_COMMAND_NRF_ENUM        (0x1) //This corresponds to Flash command 0x3B
#define QSPI_READ2IO_COMMAND_NRF_ENUM       (0x2) //This corresponds to Flash command 0xBB
#define QSPI_PP4IO_COMMAND_NRF_ENUM         (0x3) //This corresponds to Flash command 0x38
#define QSPI_READ4IO_COMMAND_NRF_ENUM       (0x4) //This corresponds to Flash command 0xEB

//#define DEBUG_ON 1
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
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_SINGLE, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
        
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
            
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_4 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
            
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_4_4 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_4_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_QUAD, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
  
////////////////////////////////////////////////////////////////////////////////////////////////////
// The Macronix Flash part on NRF52840_DK does not support Dual Mode writes. The only testing we can
// is to do a single line write and Dual-line reads which is covered by TestWriteReadCustomCommands.
// So, for now keep this tests disabled using the below DUAL_MODE_READ_ENABLED flag.    
////////////////////////////////////////////////////////////////////////////////////////////////////
//#define DUAL_MODE_READ_ENABLED    
#ifdef DUAL_MODE_READ_ENABLED    
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_1_2 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_1_2");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_DUAL, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
        
    ///////////////////////////////////////////
    // Run tests in QUADSPI 1_2_2 mode
    ///////////////////////////////////////////
    printf("\n\nQSPI Config = 1_2_2");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_DUAL, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_DUAL, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    if( false == InitializeFlashMem()) {
        printf("\nUnable to initialize flash memory, tests failed\n");
        return -1;
    }
    
    DO_TEST( TestWriteReadSimple );
    DO_TEST( TestWriteReadBlockMultiplePattern );
    DO_TEST( TestWriteMultipleReadSingle );
    DO_TEST( TestWriteSingleReadMultiple );
#endif //DUAL_MODE_READ_ENABLED    
    
    DO_TEST( TestWriteReadCustomCommands );
    
    if( true == TestWriteReadMultipleObjects()) {
        printf("\nExecuting test: %-40s : PASSED", "TestWriteReadMultipleObjects" );
    } else {
        printf("\nExecuting test: %-40s : FAILED", "TestWriteReadMultipleObjects" );        
    }
    
    if(NULL != myQspi)    
        delete myQspi;
    if(NULL != myQspiOther)
        delete myQspiOther;
    
    printf("\nDone...\n");
}

bool TestWriteReadSimple()
{
    int result = 0;
    char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
    char rx_buf[16];    
    size_t buf_len = sizeof(tx_buf);
    
    uint32_t flash_addr = 0x1000;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( flash_addr, tx_buf, &buf_len );
    if( ( result != QSPI_STATUS_OK ) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( flash_addr, rx_buf, &buf_len );
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

bool TestWriteReadBlockMultiplePattern()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 2 ); //Alloc 2k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ ); //Alloc 2k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    
    flash_addr = 0x2000;
    for(int i=0; i < 16; i++) {
        if( false == SectorErase(flash_addr)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
            return false;
        }
        
        if( false == WaitForMemReady()) {
            printf("\nDevice not ready, tests failed\n");
            return false;
        }
        
        memset( test_tx_buf_aligned, pattern_buf[i], _1_K_ );
        buf_len = _1_K_; //1k 
        result = myQspi->write( flash_addr, test_tx_buf_aligned, &buf_len );
        if( ( result != QSPI_STATUS_OK ) || buf_len != _1_K_ ) {
            printf("\nERROR: Write failed");
        }
        
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( test_rx_buf, 0, _1_K_ );
        buf_len = _1_K_; //1k
        result = myQspi->read( flash_addr, test_rx_buf, &buf_len );
        if( result != QSPI_STATUS_OK ) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != _1_K_ ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        if(0 != (memcmp( test_rx_buf, test_tx_buf_aligned, _1_K_))) {
            printf("\nERROR: Buffer contents are invalid"); 
            return false;
        }
        
        flash_addr += 0x1000;
    }
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteMultipleReadSingle()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    char *test_rx_buf_aligned = NULL;
    char *tmp = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };  
    unsigned int start_addr = 0x2000;
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    test_rx_buf_aligned = (char *)((((uint32_t)test_rx_buf) + _1_K_) & 0xFFFFFC00);
    
    flash_addr = start_addr;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nDevice not ready, tests failed\n");
        return false;
    }
    
    tmp = test_tx_buf_aligned;
    for( int i=0; i < 4; i++) {
        memset( tmp, pattern_buf[i], _1_K_ );
        buf_len = _1_K_; //1k 
        result = myQspi->write( flash_addr, tmp, &buf_len );
        if( ( result != QSPI_STATUS_OK ) || buf_len != _1_K_ ) {
            printf("\nERROR: Write failed");
        }
        
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        flash_addr += _1_K_;
        tmp += _1_K_;
    }
    
    memset( test_rx_buf_aligned, 0, _4_K_ );
    buf_len = _4_K_; //1k
    flash_addr = start_addr;
    result = myQspi->read( flash_addr, test_rx_buf_aligned, &buf_len );
    if( result != QSPI_STATUS_OK ) {
        printf("\nERROR: Read failed");
        return false;
    }
    if( buf_len != _4_K_ ) {
        printf( "\nERROR: Unable to read the entire buffer" );
        return false;
    }
    if(0 != (memcmp( test_rx_buf_aligned, test_tx_buf_aligned, _4_K_))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    } 
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteSingleReadMultiple()
{
    char *test_tx_buf = NULL;
    char *test_rx_buf = NULL;
    char *test_tx_buf_aligned = NULL;
    char *test_rx_buf_aligned = NULL;
    char *tmp = NULL;
    uint32_t flash_addr = 0;
    int result = 0;
    size_t buf_len = 0;
    char pattern_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };  
    unsigned int start_addr = 0x2000;
            
    test_tx_buf = NULL;
    test_tx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_tx_buf == NULL) {
        printf("\nERROR: tx buf alloc failed");
        return -1;
    }
    test_tx_buf_aligned = (char *)((((uint32_t)test_tx_buf) + _1_K_) & 0xFFFFFC00);
    
    test_rx_buf = NULL;
    test_rx_buf = (char *)malloc( _1_K_ * 5 ); //Alloc 5k to get a 1K boundary
    if(test_rx_buf == NULL) {
        printf("\nERROR: rx buf alloc failed");
        return false;
    }
    test_rx_buf_aligned = (char *)((((uint32_t)test_rx_buf) + _1_K_) & 0xFFFFFC00);
    
    flash_addr = start_addr;
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nDevice not ready, tests failed\n");
        return false;
    }
    
    tmp = test_tx_buf_aligned;
    for( int i=0; i < 4; i++) {
        memset( tmp, pattern_buf[i], _1_K_ );
        tmp += _1_K_;
    }
    
    buf_len = _4_K_; //4k 
    result = myQspi->write( flash_addr, test_tx_buf_aligned, &buf_len );
    if( ( result != QSPI_STATUS_OK ) || buf_len != _4_K_ ) {
        printf("\nERROR: Write failed");
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( test_rx_buf_aligned, 0, _4_K_ );
    
    buf_len = _1_K_; //1k
    flash_addr = start_addr;
    tmp = test_rx_buf_aligned;
    for( int i=0; i < 4; i++) {
        result = myQspi->read( flash_addr, tmp, &buf_len );
        if( result != QSPI_STATUS_OK ) {
            printf("\nERROR: Read failed");
            return false;
        }
        if( buf_len != _1_K_ ) {
            printf( "\nERROR: Unable to read the entire buffer" );
            return false;
        }
        tmp += _1_K_;
        flash_addr += _1_K_;
    }
    if(0 != (memcmp( test_rx_buf_aligned, test_tx_buf_aligned, _4_K_))) {
        printf("\nERROR: Buffer contents are invalid"); 
        return false;
    } 
    
    free(test_rx_buf);
    free(test_tx_buf);
    
    return true;
}

bool TestWriteReadMultipleObjects()
{
    unsigned int flash_addr1 = 0x2000;
    unsigned int flash_addr2 = 0x4000;
    
    myQspiOther = new QSPI((PinName)QSPI_PIN_IO0, (PinName)QSPI_PIN_IO1, (PinName)QSPI_PIN_IO2, (PinName)QSPI_PIN_IO3, (PinName)QSPI_PIN_SCK, (PinName)QSPI_PIN_CSN);        
    if(myQspiOther) {
        printf("\nCreated 2nd QSPI driver object succesfully");
    } else {
        printf("\nERROR: Failed creating 2nd QSPI driver object");
        return -1;
    }
    
    ////////////////////////////////////////////////////
    // Configure myQspiOther object to do 1_1_1 mode
    ////////////////////////////////////////////////////
    if( QSPI_STATUS_OK == myQspiOther->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_SINGLE, 0, 0 )) {
        printf("\nConfigured 2nd QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring 2nd QSPI object");
        return -1;
    }
    
    // if( QSPI_STATUS_OK == myQspiOther->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    //////////////////////////////////////////////
    // Configure myQspi object to do 1_4_4 mode
    //////////////////////////////////////////////
    printf("\n\nQSPI Config = 1_4_4");
    if(QSPI_STATUS_OK == myQspi->configure_format( QSPI_CFG_BUS_SINGLE, QSPI_CFG_BUS_QUAD, QSPI_CFG_ADDR_SIZE_24, QSPI_CFG_BUS_SINGLE, QSPI_CFG_ALT_SIZE_NONE, QSPI_CFG_BUS_QUAD, 0, 0 )) {
        printf("\nConfigured QSPI driver configured succesfully");
    } else {
        printf("\nERROR: Failed configuring QSPI driver");
        return -1;
    }
    
    // if(QSPI_STATUS_OK == myQspi->initialize()) {
    //     printf("\nInitializing QSPI driver success");
    // } else {
    //     printf("\nERROR: Failed init-ing QSPI driver");
    //     return -1;
    // }
    
    for(int i=0; i < 10; i++) {
        int result = 0;
        char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
        char rx_buf[16];    
        size_t buf_len = sizeof(tx_buf);
        
        if( false == SectorErase(flash_addr1)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr1);
            return false;
        }
        
        // Wait for FlashMem to be ready
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        result = myQspi->write( flash_addr1, tx_buf, &buf_len );
        if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
            printf("\nERROR: Write failed");
        }
            
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( rx_buf, 0, sizeof(rx_buf) );
        result = myQspi->read( flash_addr1, rx_buf, &buf_len );
        if(result != QSPI_STATUS_OK) {
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
        
        //Now use other object to deal with other part of memory
        if( false == SectorErase(flash_addr2)) {
            printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr2);
            return false;
        }
        
        // Wait for FlashMem to be ready
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        result = myQspiOther->write( flash_addr2, tx_buf, &buf_len );
        if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
            printf("\nERROR: Write failed");
        }
            
        if( false == WaitForMemReady()) {
            printf("\nERROR: Device not ready, tests failed\n");
            return false;
        }
        
        memset( rx_buf, 0, sizeof(rx_buf) );
        result = myQspiOther->read( flash_addr2, rx_buf, &buf_len );
        if(result != QSPI_STATUS_OK) {
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
        osDelay(100);
    }    
    
    return true;
}

bool TestWriteReadCustomCommands()
{
    int result = 0;
    char tx_buf[] = { 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x10, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x2F };    
    char rx_buf[16];    
    size_t buf_len = sizeof(tx_buf);
    
    uint32_t flash_addr = 0x1000;
    
    //Try 1-1-2 mode using custom commands
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( QSPI_PP_COMMAND_NRF_ENUM, flash_addr, 0, tx_buf, &buf_len );
    if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( QSPI_READ2O_COMMAND_NRF_ENUM, flash_addr, 0, rx_buf, &buf_len );
    if(result != QSPI_STATUS_OK) {
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
    
    //Try 1-2-2 mode using custom commands
    if( false == SectorErase(flash_addr)) {
        printf("\nERROR: SectorErase failed(addr = 0x%08X)\n", flash_addr);
        return false;
    }
    
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    result = myQspi->write( QSPI_PP_COMMAND_NRF_ENUM, flash_addr, 0, tx_buf, &buf_len );
    if( (result != QSPI_STATUS_OK) || buf_len != sizeof(tx_buf) ) {
        printf("\nERROR: Write failed");
    }
        
    if( false == WaitForMemReady()) {
        printf("\nERROR: Device not ready, tests failed\n");
        return false;
    }
    
    memset( rx_buf, 0, sizeof(rx_buf) );
    result = myQspi->read( QSPI_READ2IO_COMMAND_NRF_ENUM, flash_addr, 0, rx_buf, &buf_len );
    if(result != QSPI_STATUS_OK) {
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
    char status_value[2];
    
    //Read the Status Register from device
    if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_RDSR, // command to send
                              0,                 // do not transmit
                              NULL,              // do not transmit
                              status_value,                 // just receive two bytes of data
                              2)) {   // store received values in status_value
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
            
            if(ret_status)
            {
                status_value[0] |= 0x40;
                //Write the Status Register to set QE enable bit
                if (QSPI_STATUS_OK == myQspi->command_transfer(QSPI_STD_CMD_WRSR, // command to send
                                          status_value,                 
                                          1,      
                                          NULL,                 
                                          0)) {   // store received values in status_value
                    VERBOSE_PRINT(("\nWriting Status Register Success\n"));
                } else {
                    printf("\nERROR: Writing Status Register failed\n");
                    ret_status = false;
                }
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
    
    return true;
}



#endif
