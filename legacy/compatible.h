#ifndef COMPATIBLE_H
#define COMPATIBLE_H
#if GD32F470
// oled spi
// #ifdef DEVELOPMENT_BOARD_TEST
// #define OLED_SPI_BASE SPI2
// #define RCC_OLED_SPI RCC_SPI2
// #define RCC_OLED_DATA RCC_GPIOI
// #define RCC_OLED_COTL RCC_GPIOH

// #define OLED_DC_PORT GPIOH
// #define OLED_DC_PIN GPIO13  // PH13 | Data/Command
// #define OLED_CS_PORT GPIOI
// #define OLED_CS_PIN GPIO0  // PI0 | SPI Select
// #define OLED_RST_PORT GPIOH
// #define OLED_RST_PIN GPIO15  // PH15 | Reset display
// #define OLED_SCK_PIN GPIO1
// #define OLED_MOSI_PIN GPIO3
// #else
#define OLED_SPI_BASE SPI1
#define RCC_OLED_SPI RCC_SPI1
#define RCC_OLED_DATA RCC_GPIOA  // data:Nss Sck Mosi
#define RCC_OLED_COTL RCC_GPIOB  // control: D/C and Reset

#define OLED_DC_PORT GPIOB
#define OLED_DC_PIN GPIO0  // PB0 | Data/Command
#define OLED_CS_PORT GPIOA
#define OLED_CS_PIN GPIO4  // PA4 | SPI Select
#define OLED_RST_PORT GPIOB
#define OLED_RST_PIN GPIO1   // PB1 | Reset display
#define OLED_SCK_PIN GPIO5   // PA5 | SPI CLK
#define OLED_MOSI_PIN GPIO7  // PA4 | SPI MOSI
// #endif

// usb define
#define otgfs_usb_driver_onekey gd32f470_usb_driver
// usart define
#define BLE_UART USART2

// keys define
// for f470 development board
// #define UP_PORT GPIOA
// #define DOWN_PORT GPIOC
// #define CONF_PORT GPIOB
// #ifndef BTN_PIN_UP
// #define BTN_PIN_UP GPIO0
// #endif
// #ifndef BTN_PIN_DOWN
// #define BTN_PIN_DOWN GPIO13
// #endif
// #ifndef BTN_PIN_YES
// #define BTN_PIN_YES GPIO14
// #endif

// keys define
#ifndef BTN_PORT
#define BTN_PORT GPIOC
#endif

#ifndef BTN_PIN_YES
#define BTN_PIN_YES GPIO2
#endif

#ifndef BTN_PORT_NO
#define BTN_PORT_NO BTN_POWER_PORT
#endif

#ifndef BTN_PIN_NO
#define BTN_PIN_NO BTN_POWER_PIN
#endif

#ifndef BTN_PIN_UP
#define BTN_PIN_UP GPIO3
#endif

#ifndef BTN_PIN_DOWN
#define BTN_PIN_DOWN GPIO5
#endif

// power define
// need change later
// #undef BTN_POWER_PORT
// #define BTN_POWER_PORT GPIOC
// #undef BTN_POWER_PIN
// #define BTN_POWER_PIN GPIO4

#ifndef BTN_PORT_NO
#define BTN_PORT_NO UP_PORT
#endif

#ifndef BTN_PIN_NO
#define BTN_PIN_NO BTN_PIN_UP
#endif

// se chip define
#define GPIO_MI2C_SCL GPIO6
#define GPIO_MI2C_SDA GPIO7
#define MI2C_RETRYCNTS (5000)  // timeout 5 s
#define MI2C_TIMEOUT (7000)
#else

// oled spi
#define OLED_SPI_BASE SPI1
#define RCC_OLED_SPI RCC_SPI1
#define RCC_OLED_DATA RCC_GPIOA
#define RCC_OLED_COTL RCC_GPIOB
#ifdef OLD_PCB
#define OLED_DC_PORT GPIOA
#define OLED_DC_PIN GPIO2  // PA2 | Data/Command
#define OLED_CS_PORT GPIOA
#define OLED_CS_PIN GPIO4  // PA4 | SPI Select
#define OLED_RST_PORT GPIOA
#define OLED_RST_PIN GPIO3  // PA3 | Reset display
#define OLED_SCK_PIN GPIO5
#define OLED_MOSI_PIN GPIO7
#else
#define OLED_DC_PORT GPIOB
#define OLED_DC_PIN GPIO0  // PB0 | Data/Command
#define OLED_CS_PORT GPIOA
#define OLED_CS_PIN GPIO4  // PA4 | SPI Select
#define OLED_RST_PORT GPIOB
#define OLED_RST_PIN GPIO1  // PB1 | Reset display
#define OLED_SCK_PIN GPIO5
#define OLED_MOSI_PIN GPIO7
#endif
// usb define
#define otgfs_usb_driver_onekey otgfs_usb_driver
// usart define
#define BLE_UART USART2
// keys define
#ifndef BTN_PORT
#define BTN_PORT GPIOC
#endif

#ifndef BTN_PIN_YES
#define BTN_PIN_YES GPIO2
#endif

#ifndef BTN_PORT_NO
#define BTN_PORT_NO BTN_POWER_PORT
#endif

#ifndef BTN_PIN_NO
#define BTN_PIN_NO BTN_POWER_PIN
#endif

#ifndef BTN_PIN_UP
#define BTN_PIN_UP GPIO3
#endif

#ifndef BTN_PIN_DOWN
#define BTN_PIN_DOWN GPIO5
#endif

// se chip define
#define GPIO_MI2C_SCL GPIO8
#define GPIO_MI2C_SDA GPIO9
#define MI2C_RETRYCNTS (5)
#define MI2C_TIMEOUT (40000)

#endif  // GD32F470
#endif  // COMPATIBLE_H