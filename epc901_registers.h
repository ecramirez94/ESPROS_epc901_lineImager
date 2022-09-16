#ifndef EPC901_REGISTERS_H
#define EPC901_REGISTERS_H

/*
 * Register Addresses
 */
#define ACQ_TX_CONF_I2C 0x00
#define BW_VIDEO_CONF_I2C 0x01
#define MISC_CONF 0x02
#define OSC_TRIM_REG 0x90
#define VIDEO_GBW_SEL_REG 0x94
#define TEMP_SENS_L_LSB 0xA0
#define TEMP_SENS_L_MSB 0xA1
#define TEMP_SENS_R_LSB 0xA2
#define TEMP_SENS_R_MSB 0xA3
#define TEMP_SENS_CONF 0xA4
#define I2C_ERROR_IND 0xB0
#define FORCE_ANA_CTRL_SIGS 0xD6
#define OSC_TRIM_RANGE_REG 0xD7
#define CHIP_REV_NO_REG 0xFF



/*
 * Settings bitmasks
 * 
 * All masks should be applied using a 
 * logical AND operation.
 * 
 * Then the bits set with a logical
 * OR operation.
 */

 
// Horizontal bin settings are bits 5 and 4
#define HOR_BIN_MASK 0b00110000
#define HOR_BIN_TWO 0x00  // Available in differential mode only
#define NO_HOR_BIN 0x10
#define HOR_BIN_FOUR 0x20

// Video amplifier voltage gain are bits 3 and 2
#define VIDEO_AMP_GAIN_MASK 0b00001100
#define VIDEO_AMP_GAIN_TWO 0x00
#define VIDEO_AMP_GAIN_ONE 0x04
#define VIDEO_AMP_GAIN_FOUR 0x08

// Region of interest is bit 1
#define ROI_SEL_MASK 0b00000010
#define ROI_SEL_ALL 0x00
#define ROI_SEL_HALF 0x02

// Read direction is bit 0
#define RD_DIR_MASK 0b00000001
#define RD_DIR_LR 0x00  // From 0 to 1023
#define RD_DIR_RL 0x01  // From 1023 to 0


#endif
