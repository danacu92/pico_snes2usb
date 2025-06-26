
//#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

// Pins definition
#define PIN_LATCH 2 // GP2
#define PIN_CLOCK 3 // GP3
#define PIN_DATA 4 // GP4

// 2 bytes report, 1 byte for D-pad (hat) and 1 byte for Buttons.
typedef struct {
    uint8_t hat;
    uint8_t buttons; 
} __attribute__((packed)) GamepadReport;

typedef union {
    struct {
        uint16_t B : 1;      // Bit 0
        uint16_t Y : 1;      // Bit 1
        uint16_t Select : 1; // Bit 2
        uint16_t Start : 1;  // Bit 3
        uint16_t UP : 1;     // Bit 4
        uint16_t DOWN : 1;   // Bit 5
        uint16_t LEFT : 1;   // Bit 6
        uint16_t RIGHT : 1;  // Bit 7
        uint16_t A : 1;      // Bit 8
        uint16_t X : 1;      // Bit 9
        uint16_t L : 1;      // Bit 10
        uint16_t R : 1;      // Bit 11
        uint16_t unused : 4; // Bits 12-15
    } buttons;
    uint16_t raw;
} Inputs;

/*------------- MAIN -------------*/
void send_gamepad_report() {
  if (!tud_hid_ready()) 
  return;

  Inputs in;
  in.raw=0;
  GamepadReport rpt;

  // Start to read SNES controller status.
  gpio_put(PIN_LATCH, 1);
  sleep_us(12);
  gpio_put(PIN_LATCH, 0);

  for (int i = 0 ; i<16; i++ ) {
  
    in.raw |= !gpio_get(PIN_DATA) << i; 
    sleep_us(6); // To avoid garbage reads.
    gpio_put(PIN_CLOCK, 1); // Clock
    sleep_us(6);
    gpio_put(PIN_CLOCK, 0);
    sleep_us(6);
    
}
  
  // Getting the hat final value.
  if (in.buttons.UP) {
        if (in.buttons.RIGHT) {
            rpt.hat = 2 ; //  UP RIGHT
        } else if (in.buttons.LEFT) {
            rpt.hat = 8;  //  UP LEFT
        } else {
            rpt.hat = 1;       // UP
        }
    } 
  else if (in.buttons.DOWN) {
        if (in.buttons.RIGHT ) {
            rpt.hat = 4; // DOWN RIGHT
        } else if (in.buttons.LEFT) {
            rpt.hat = 6 ;  // HAT_DOWN_LEFT
        } else {
            rpt.hat = 5 ;       // HAT_DOWN
        }
    } 
  else if (in.buttons.LEFT) {
        rpt.hat = 7;           // HAT_LEFT
    } 
  else if (in.buttons.RIGHT) {
        rpt.hat = 3;          // HAT_RIGHT
    } 
  else {
        rpt.hat = 0 ;         // HAT_CENTER
    }

  rpt.buttons = ((in.raw & 0x0F00) >> 4) | (in.raw & in.raw & 0x000F);
  tud_hid_report(0, &rpt, sizeof(rpt));
}

// Callbacks needed by TinyUSB
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer, uint16_t bufsize)
{
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;

}
uint16_t tud_hid_get_report_cb(uint8_t instance,
                                uint8_t report_id,
                                hid_report_type_t report_type,
                                uint8_t* buffer,
                                uint16_t reqlen)
{
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;
  return 0; 
}

// MAIN
int main(void)
{
  board_init();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  gpio_init(PIN_LATCH);
  gpio_pull_down(PIN_LATCH);
  gpio_set_dir(PIN_LATCH, GPIO_OUT);

  gpio_init(PIN_CLOCK);
  gpio_pull_down(PIN_CLOCK);
  gpio_set_dir(PIN_CLOCK, GPIO_OUT);

  gpio_init(PIN_DATA);
  gpio_pull_up(PIN_DATA);
  gpio_set_dir(PIN_DATA, GPIO_IN);

  while (1)
  {
    tud_task(); // tinyusb device task
    send_gamepad_report();
    sleep_ms(5);
  }
}
