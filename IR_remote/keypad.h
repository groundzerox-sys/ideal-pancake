sbit row_1 = P1^0;
sbit row_2 = P1^1;
sbit row_3 = P1^2;
sbit row_4 = P1^3;
sbit col_1 = P1^4;
sbit col_2 = P1^5;
sbit col_3 = P1^6;
sbit col_4 = P1^7;

void delay(unsigned int t){
	int i,j;
	for(i = 0; i < t; i++){
		for(j = 0; j < 382; j++){
		}
	}
}

unsigned char key(void){
		P1 = 0x0F;
		if(P1 < 0x0F){
			
			delay(100);
			
			if(P1 < 0x0F){
				
				P1 = 0xE0 | 0x0F;
				
				if(row_1 == 0){
				  return 'A';
				}
				
				else if(row_2 == 0){
					return 'E';
				}
				
				else if(row_3 == 0){
					return 'I';
				}
				
				else if(row_4 == 0){
					return 'M';
				}
				
				P1 = 0xD0 | 0x0F;
				
				if(row_1 == 0){
					return 'B';
				}
				
				else if(row_2 == 0){
					return 'F';
				}
				
				else if(row_3 == 0){
					return 'J';
				}
				
				else if(row_4 == 0){
					return 'N';
				}
				
				P1 = 0xB0 | 0x0F;
				if(row_1 == 0){
					return 'C';
				}
				
				else if(row_2 == 0){
					return 'G';
				}
				
				else if(row_3 == 0){
					return 'K';
				}
				
				else if(row_4 == 0){
					return 'O';
				}
				
				P1 = 0x70 | 0x0F;
				
				if(row_1 == 0){
					return 'D';
				}
				
				else if(row_2 == 0){
					return 'H';
				}
				
				else if(row_3 == 0){
					return 'L';
				}
				
				else if(row_4 == 0){
					return 'P';
				}
				
			}
		}
		return 0;
}