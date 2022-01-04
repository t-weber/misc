/**
 * simple lcd module
 * @author Tobias Weber
 * @date jan-2022
 * @license see 'LICENSE.GPL' file
 *
 * @see https://www.arduino.cc/documents/datasheets/LCDscreen.PDF
 */

#include <stdint.h>
#include <stdbool.h>


typedef struct _LCDInfo
{
	/* enable and register select pins */
	int pin_en;
	int pin_rs;

	/* data pins for 4-bit mode */
	int pin_d4;
	int pin_d5;
	int pin_d6;
	int pin_d7;

	/* (microcontroller's) delay function */
	void (*delay)(uint32_t millisecs);

	/* (microcontroller's) outout function */
	void (*set_pin)(uint8_t pin, uint8_t state);

	/* constants for set or unset pins */
	int pin_set;
	int pin_unset;
} LCDInfo;


/**
 * send 4 bits to the display
 */
void lcd_send_nibble(const LCDInfo* lcd, bool rs, uint8_t data)
{
	lcd->set_pin(lcd->pin_en, lcd->pin_unset);
	lcd->set_pin(lcd->pin_rs, rs ? lcd->pin_set : lcd->pin_unset);
	lcd->set_pin(lcd->pin_d4, (data & 1) ? lcd->pin_set : lcd->pin_unset);
	lcd->set_pin(lcd->pin_d5, ((data>>1) & 1) ? lcd->pin_set : lcd->pin_unset);
	lcd->set_pin(lcd->pin_d6, ((data>>2) & 1) ? lcd->pin_set : lcd->pin_unset);
	lcd->set_pin(lcd->pin_d7, ((data>>3) & 1) ? lcd->pin_set : lcd->pin_unset);
	lcd->set_pin(lcd->pin_en, lcd->pin_set);
	lcd->delay(1);
	lcd->set_pin(lcd->pin_en, lcd->pin_unset);
}


/**
 * send 8 bits to the display
 */
void lcd_send_byte(const LCDInfo* lcd, bool rs, uint8_t data)
{
	lcd_send_nibble(lcd, rs, (data&0xf0)>>4);
	lcd_send_nibble(lcd, rs, data&0x0f);
}


/**
 * initialise the display
 * @see p. 12 of https://www.arduino.cc/documents/datasheets/LCDscreen.PDF
 */
void lcd_init(const LCDInfo* lcd)
{
	lcd->delay(20);
	lcd_send_nibble(lcd, 0, 0b0011);
	lcd->delay(5);
	lcd_send_nibble(lcd, 0, 0b0011);
	lcd->delay(1);
	lcd_send_nibble(lcd, 0, 0b0011);
	lcd_send_nibble(lcd, 0, 0b0010);
}


/**
 * clear the screen
 */
void lcd_clear(const LCDInfo* lcd)
{
	lcd_send_byte(lcd, 0, 0b00000001);
	lcd->delay(2);
}


/**
 * set the direction of the caret
 */
void lcd_set_caret_direction(const LCDInfo* lcd)
{
	lcd_send_byte(lcd, 0, 0b00000110);
}


/**
 * caret return
 */
void lcd_return(const LCDInfo* lcd)
{
	lcd_send_byte(lcd, 0, 0b00000010);
	lcd->delay(2);
}


/**
 * shift display
 */
void lcd_shift(const LCDInfo* lcd, bool all, bool right)
{
	uint8_t byte = 0b00010000;
	if(all) /* shift all or just caret? */
		byte |= 1<<3;
	if(right)
		byte |= 1<<2;
	lcd_send_byte(lcd, 0, byte);
}


/**
 * set display functions
 */
void lcd_set_function(const LCDInfo* lcd, bool displ, bool lines, bool font)
{
	uint8_t byte = 0b00100000;
	if(displ)
		byte |= 1<<4;
	if(lines)
		byte |= 1<<3;
	if(font)
		byte |= 1<<2;
	lcd_send_byte(lcd, 0, byte);
}


/**
 * turn display and caret on or off
 */
void lcd_set_display(const LCDInfo* lcd, bool on, bool caret_line, bool caret_box)
{
	uint8_t byte = 0b00001000;
	if(on)
		byte |= 1<<2;
	if(caret_line)
		byte |= 1<<1;
	if(caret_box)
		byte |= 1;
	lcd_send_byte(lcd, 0, byte);
}


/**
 * write a string to the display
 */
void lcd_puts(const LCDInfo* lcd, const char* str)
{
	const char* iter = str;
	while(*iter)
	{
		lcd_send_byte(lcd, 1, *iter);
		++iter;
	}
}
