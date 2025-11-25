#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "board_config.h"
#include "spi.h"

// SD Card Commands
#define CMD0    0   // GO_IDLE_STATE
#define CMD8    8   // SEND_IF_COND
#define CMD55   55  // APP_CMD
#define CMD58   58  // READ_OCR
#define ACMD41  41  // SD_SEND_OP_COND

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg) {
    uint8_t tx[6];
    tx[0] = 0x40 | cmd;
    tx[1] = (arg >> 24) & 0xFF;
    tx[2] = (arg >> 16) & 0xFF;
    tx[3] = (arg >> 8) & 0xFF;
    tx[4] = arg & 0xFF;
    tx[5] = 0x95;  // CRC for CMD0
    if (cmd == CMD8) tx[5] = 0x87;
    
    spi_cs_select(SD_CS_PIN);
    
    // Send command
    spi_write_blocking(SD_SPI, tx, 6);
    
    // Wait for response
    uint8_t response = 0xFF;
    for (int i = 0; i < 10; i++) {
        spi_read_blocking(SD_SPI, 0xFF, &response, 1);
        if (response != 0xFF) break;
    }
    
    spi_cs_deselect(SD_CS_PIN);
    
    return response;
}

void sd_card_test(void) {
    printf("Testing SD Card (SPI mode)...\n");
    
    // Send 80 clock cycles with CS high
    spi_cs_deselect(SD_CS_PIN);
    uint8_t dummy[10];
    memset(dummy, 0xFF, 10);
    spi_write_blocking(SD_SPI, dummy, 10);
    
    // CMD0 - Go to idle state
    printf("Sending CMD0 (GO_IDLE)...\n");
    uint8_t r1 = sd_send_cmd(CMD0, 0);
    printf("  Response: 0x%02X\n", r1);
    
    if (r1 == 0x01) {
        printf("  SD Card detected and in idle state.\n");
        
        // CMD8 - Check voltage
        printf("Sending CMD8 (SEND_IF_COND)...\n");
        r1 = sd_send_cmd(CMD8, 0x1AA);
        printf("  Response: 0x%02X\n", r1);
        
        if (r1 == 0x01) {
            printf("  SD Card is SDv2 (SDHC/SDXC capable).\n");
        } else {
            printf("  SD Card is SDv1 or MMC.\n");
        }
        
        printf("SD Card basic detection complete.\n");
    } else if (r1 == 0xFF) {
        printf("  No SD Card detected (no response).\n");
    } else {
        printf("  Unexpected response (card may be in unknown state).\n");
    }
    
    printf("SD Card test complete.\n");
}
