#include<at89c5131.h>
#include<stdio.h>
#include<lcd.h>
#include<ir_data.h>
#include<keypad.h>

unsigned char head = NULL_IDX;
unsigned char curr_rem = NULL_IDX;
unsigned char curr_cmd = NULL_IDX;

unsigned char chunk[4] = {0x00,0x00,0x00,0x00};
unsigned char chunk_idx = 0;
unsigned char bt_idx = 0;
unsigned char tx_state = 0; // 0=Leader OFF, 1=Data ON, 2=Data OFF, 3=Stop
unsigned char tx_bit_count = 0;
unsigned char tx_byte_idx = 0;
sbit ir_tx = P3^4;
	unsigned char i = 0;
bit carrier = 0;
bit rx_tx = 0; //receiver mode default
bit r_start = 0;
bit rx_bt; // received bit
bit tx_bt;
bit is_edge = 1; //is this 
sbit ir_rx = P3^2; //active low receiver
bit tx_start = 1;
sbit IE0 = TCON^1;

void update_display(void) {
    lcd_cmd(0x01); // Clear LCD
    delay_ms(2);
    
    // Line 1: Remote Name
    lcd_cmd(0x80);
    lcd_print(get_rem_name(curr_rem));
    
    // Line 2: Command Name
    lcd_cmd(0xC0);
    lcd_print(get_cmd_name(curr_cmd));
}

void trigger_tx(void) {
    unsigned char j;
    if (curr_cmd == NULL_IDX) return; // Nothing to transmit
    
    // Load the selected command's 4 bytes into chunk[] for transmission
    for (j = 0; j < 4; j++) {
        chunk[j] = get_cmd_byte(curr_cmd, j);
    }
    
    EX0 = 0;      // Disable receiver interrupt during TX
    TR0 = 0;
    TR1 = 0;
    
    TMOD = 0x21;  // Timer 1: 8-bit Auto-Reload (38kHz), Timer 0: 16-bit
    
    rx_tx = 1;
    tx_state = 0;
    tx_bit_count = 0;
    tx_byte_idx = 0;
    
    // 9ms Leader ON
    TH0 = 0xB9;
    TL0 = 0xB0;
    
    // 38kHz Carrier (24MHz crystal)
    TH1 = 0xE6;
    TL1 = 0xE6;
    
    tx_bt = 1;
    TF0 = 0;
    TF1 = 0;
    TR1 = 1;
    TR0 = 1;
}

void ir_detect() interrupt 0{
	if(r_start == 0){
		TH0 = 0xBB; // programmed for 8750us 24mhz to check if start bit is valid
		TL0 = 0xA4; 
		TR0 = 1;
	}
		else{
		TR0 = 0;
		TR1 = 0;
		if(chunk_idx > 3) {
            r_start = 0;       // Reset state machine
            is_edge = 1;       // Reset edge flag
            chunk_idx = 0;     // Reset index for next button press
            bt_idx = 0;
            // Here you would normally set a flag to tell main() to process 'chunk'
            return; 
        }
		TH1 = 0xEE; // 2275us
		TL1 = 0x3A; // check if transmistion failed or ended
		TH0 = 0xF2; // 1750us 
		TL0 = 0x54; //check if bit 1 or 0
		
		if(is_edge == 1){
 //don't consider if start
			is_edge = 0;
		}
		else{
		if (rx_bt == 1) {
                chunk[chunk_idx] |= (1 << bt_idx);
            } else {
                chunk[chunk_idx] &= ~(1 << bt_idx); // ensure bit is 0
            }
		bt_idx++;
		}
		if(bt_idx == 8){
			bt_idx = 0;
			chunk_idx++;
		}	
					rx_bt = 0;
		TR0 = 1;
		TR1 = 1;
	}
}
void pulse_check() interrupt 1{
	if(rx_tx == 0){
	if(r_start == 0 && ir_rx == 0){
		r_start = 1;  // if start bit valid
		TR0 = 0; 
	}
	else{
		TR0 = 0;
		rx_bt = 1;
	}
}
	else {
        // TRANSMIT STATE MACHINE
        TR0 = 0; // Stop timer to reload values safely

        if (tx_state == 0) { 
            // Finished 9ms ON, switch to 4.5ms OFF
            tx_bt = 0;          // Turn off 38kHz carrier
            TH0 = 0xDD;         // 4.5ms
            TL0 = 0x98;
            tx_state = 1;
        } 
        else if (tx_state == 1 || tx_state == 2) { 
            // Finished OFF gap, start 562us ON pulse for the next bit
            if (tx_byte_idx > 3) {
                // Payload complete, transmit final Stop Bit
                tx_bt = 1;
                TH0 = 0xFB;     // 562us
                TL0 = 0xBB;
                tx_state = 3;
            } else {
                tx_bt = 1;      // Turn on 38kHz carrier
                TH0 = 0xFB;     // 562us
                TL0 = 0xBB;
                tx_state = 4;   // Go evaluate data bit
            }
        } 
        else if (tx_state == 4) { 
            // Finished 562us ON, calculate OFF gap based on array data
            tx_bt = 0;          // Turn off carrier
            
            // Check bit value (LSB first)
            if ((chunk[tx_byte_idx] >> tx_bit_count) & 1) {
                // Bit is '1' -> 1687us gap
                TH0 = 0xF2;
                TL0 = 0xD1;
            } else {
                // Bit is '0' -> 562us gap
                TH0 = 0xFB;
                TL0 = 0xBB;
            }
            
            tx_bit_count++;
            if (tx_bit_count == 8) {
                tx_bit_count = 0;
                tx_byte_idx++;
            }
            tx_state = 2; // Loop back for next bit
        } 
        else if (tx_state == 3) { 
            // Stop bit finished, shut everything down and restore RX mode
            TR1 = 0;
            ir_tx = 0;
            TMOD = 0x11; // Restore Timer 1 to 16-bit mode for RX timeout
            IE0 = 0;     // Clear any false interrupt flag picked up during TX
            EX0 = 1;     // Re-enable IR receiver interrupt
            rx_tx = 0;   // Return to receive mode
            return;      // Do not restart Timer 0
        }
        
        TR0 = 1; // Restart timer for next phase
    }
}

void rx_end() interrupt 3{
	if(rx_tx == 0){
 r_start = 0;	//rx failed or ended
	is_edge = 1;
		chunk_idx = 0;
		bt_idx = 0;
		TR0 = 0;
	TR1 = 0;
	}
	else{
		carrier = ~carrier;
		ir_tx = carrier & tx_bt;
	}
}

void main(void){
	char name_buf[10] = {'D','E','F','A','U','L','T','\0'};
  unsigned char j;
	
	TMOD = 0x11;
	IT0 = 1;
	EX0 = 1;
	ET0 = 1;
	ET1 = 1;
	EA = 1;
	ir_tx = 0;
	lcd_init();
	while(1){
switch (key()) {
case 'A': // Add new Remote
                sprintf(name_buf, "REM %d", (int)(ir_pool_idx + 1));
                head = insert_name(head, name_buf);
                curr_rem = head;
                curr_cmd = get_rem_cmd_head(curr_rem);
                update_display();
                break;
                
            case 'B': // Save last received IR signal into current Remote
                if (curr_rem != NULL_IDX) {
                    sprintf(name_buf, "CMD %d", (int)(cmd_idx + 1));
                    curr_cmd = insert_cmd_name(curr_rem, name_buf);
                    for (j = 0; j < 4; j++) {
                        insert_cmd(curr_rem, chunk[j], j);
											chunk[j] = 0;
                    }
                    update_display();
                }
                break;
                
            case 'C': // Transmit currently selected Command
                trigger_tx();
                break;
                
            case 'D': // Previous Remote
                curr_rem = get_prev_rem(curr_rem);
                curr_cmd = get_rem_cmd_head(curr_rem);
                update_display();
                break;
                
            case 'E': // Next Remote
                curr_rem = get_next_rem(curr_rem);
                curr_cmd = get_rem_cmd_head(curr_rem);
                update_display();
                break;
                
            case 'F': // Previous Command
                curr_cmd = get_prev_cmd(curr_cmd);
                update_display();
                break;
                
            case 'G': // Next Command
                curr_cmd = get_next_cmd(curr_cmd);
                update_display();
                break;
                
            default:
                break;
		//test
}
	}
}