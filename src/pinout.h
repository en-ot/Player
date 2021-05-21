#ifndef _PINOUT_H_
#define _PINOUT_H_


// Digital I/O used
//VSPI
#define TFT_MISO -1     //19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   5  // Chip select control pin
#define TFT_DC   16  // Data Command control pin
#define TFT_RST  17  // Reset pin (could connect to RST pin)

// #ifndef SS
//     #define SS      13
// #endif
#define SD_CS       13
#define SPI_SCK     14
#define SPI_MISO    2
#define SPI_MOSI    15

#define I2S_DOUT    19
#define I2S_BCLK    26
#define I2S_LRC     25

#define BTN_1       38
#define BTN_2       37
#define BTN_3       39

#define AENC1       34
#define AENC2       36

#endif // _PINOUT_H_
