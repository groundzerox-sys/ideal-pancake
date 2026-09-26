// 1. Define your I2C pins
#include <intrins.h>

sbit SDA = P3^0;
sbit SCL = P3^1;

/* 
 * 2. Set the PCF8574 Address
 * If address pins A0,A1,A2 are connected to VCC -> 0x4E
 * If connected to GND (typical in Proteus default) -> 0x40
 * If you are using the 'A' variant (PCF8574A) with VCC -> 0x7E
 */
#define I2C_ADDR 0x7C 

void delay_ms(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 112; j++);
    }
}

// Creates a ~5us delay to let the voltage reach a solid 5V
void i2c_delay(void) {
    _nop_(); _nop_(); _nop_(); _nop_(); _nop_();
}

void i2c_start(void) {
    SDA = 1; 
    SCL = 1;
   	i2c_delay(); 
    SDA = 0; 
	  i2c_delay(); 
    SCL = 0; 
}

void i2c_stop(void) {
	  SCL = 0;
    SDA = 0; 
	  i2c_delay(); 
    SCL = 1; 
	  i2c_delay(); 
    SDA = 1; 
}

void i2c_write(unsigned char dat) {
    unsigned char i;
	  unsigned int timeout = 1000;
    for(i = 0; i < 8; i++) {
        SDA = (dat & 0x80) ? 1 : 0;
        SCL = 1;
        i2c_delay();
        SCL = 0;
        dat <<= 1;
    }
 //acknowledge ment from slave;
    SDA = 1; 
    SCL = 1; 
		i2c_delay();
		while((SDA == 1) && (timeout > 0)) {
        timeout--;
    }
    SCL = 0;
    i2c_delay();
}

// --- LCD OVER I2C LOGIC ---

void lcd_send(unsigned char val, unsigned char rs) {
    unsigned char high_nibble, low_nibble;

    // PCF8574 Pin Map: D7, D6, D5, D4, Backlight(BL), Enable(EN), Read/Write(RW), RegisterSelect(RS)
    // BL is bit 3 (0x08). EN is bit 2 (0x04).
    high_nibble = (val & 0xF0) | 0x08 | rs;
    low_nibble  = ((val << 4) & 0xF0) | 0x08 | rs;

    i2c_start();
    i2c_write(I2C_ADDR);
    
    // Pulse EN high, then low for the high nibble
    i2c_write(high_nibble | 0x04); 
    i2c_write(high_nibble);        

    // Pulse EN high, then low for the low nibble
    i2c_write(low_nibble | 0x04);  
    i2c_write(low_nibble);         
    
    i2c_stop();
}

// Send a command (RS = 0)
void lcd_cmd(unsigned char cmd) {
    i2c_start();
    i2c_write(0x7C); // Slave Address from your diagram
    i2c_write(0x00); // Control Byte: bit 6 (RS) is 0
    i2c_write(cmd);  // The actual command byte
    i2c_stop();
    delay_ms(2);
}

// Send a character (RS = 1)
// Note: Rename this to lcd_char if that is what your main loop uses
void lcd_char(unsigned char dat) {
    i2c_start();
    i2c_write(0x7C); // Slave Address 
    i2c_write(0x40); // Control Byte: bit 6 (RS) is 1 (0x40 = 01000000 in binary)
    i2c_write(dat);  // The actual character byte
    i2c_stop();
    delay_ms(2);
}

void lcd_init(void) {
    delay_ms(50);
    lcd_cmd(0x38); // 8-bit mode, 2 lines, 5x8 font
    lcd_cmd(0x0C); // Display ON, Cursor OFF
    lcd_cmd(0x01); // Clear display
    delay_ms(5);
}

void lcd_print(char *str) {
    while(*str) {
        lcd_char(*str++);
    }
}