#include <msp430.h>
#include "onewire.h"
#include "delay.h"
#include <string.h>

// Resources
// https://blog.danman.eu/cloning-ibutton-using-rw1990-and-avr/
// http://sun-student.ru/hard/rw1990/finale.html

int main()
{
  onewire_t ow;
  int i;
  uint8_t rom_buffer[8];
  memset(rom_buffer, 0xFF, 8);
  // http://www.datastat.com/sysadminjournal/maximcrc.cgi can be used to check/calculate the CRC
  static uint8_t serial_number_to_write[8] = {0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x75};

  WDTCTL = WDTPW + WDTHOLD; //Stop watchdog timer
  BCSCTL1 = CALBC1_8MHZ;
  DCOCTL = CALDCO_8MHZ;

  ow.port_out = &P1OUT;
  ow.port_in = &P1IN;
  ow.port_ren = &P1REN;
  ow.port_dir = &P1DIR;
  ow.pin = BIT7; //P1.7

  // Keep retrying until the proper serial number is written
  // Probably wouldn't need this if proper hardware timers were used
  while(memcmp(serial_number_to_write, rom_buffer, 8) != 0)
  {
      DELAY_MS(16);
      onewire_reset(&ow);
      onewire_write_byte(&ow, 0x33); // read rom
      for(i = 0; i < 8; i++)
      {
          rom_buffer[i] = onewire_read_byte(&ow);
      }
      DELAY_MS(16);
      onewire_reset(&ow);
      onewire_write_byte(&ow, 0xD5); // write rom (RW1990 only cmd)
      for(i = 0; i < 8; i++)
      {
          onewire_write_serial_number_byte(&ow, serial_number_to_write[i]);
      }
  }

  memset(rom_buffer, 0xFF, 8); // Reset buffer so we ensure new values are read.

  onewire_reset(&ow);
  onewire_write_byte(&ow, 0x33); // read rom
  for(i = 0; i < 8; i++)
  {
      rom_buffer[i] = onewire_read_byte(&ow);
  }
  _BIS_SR(LPM0_bits + GIE);
  return 0;
}
