#include <stdio.h>
#include <conio.h>
#include <cc65.h>
#include <cbm.h>
#include <mouse.h>
#include <string.h>
#include "vickers.h"

#include <peekpoke.h>

#define KEY_F1  0x85
#define KEY_F3  0x86

#define RVSON  0x12
#define RVSOFF 0x92

#define CHAR_RAM 0x0400
#define COLOUR_RAM 0xD800



#define SPRITE0_DATA    0x0340
#define SPRITE0_PTR     0x07F8
//#define DRIVER          "c64-pot.mou"
#define DRIVER          "c64-1351.mou"
//#define DRIVER          "c64-joy.mou"




unsigned short hits=0;
unsigned short step_size=4;
unsigned char baseline=12;
unsigned char tail_length=10;
unsigned char delta_x=30;
unsigned char delta_y=30;
unsigned short deg_offset=30;
unsigned short deg_step=10;
unsigned char x_step=1;
unsigned char x_step_max=10;


struct mouse_info mouse;

/* The mouse sprite (an arrow) */
static const unsigned char CursorSprite[64] = {
0x00,0x28,0x00,0x00,0x96,0x00,0x02,0x69,
0x80,0x09,0xbe,0x60,0x26,0xeb,0x98,0x9b,
0xaa,0xe6,0x9b,0x96,0xe6,0x9b,0x96,0xe6,
0x9b,0x96,0xe6,0x9b,0x96,0xe6,0x9b,0x96,
0xe6,0x9b,0x96,0xe6,0x9b,0x96,0xe6,0x9b,
0x96,0xe6,0x9b,0x96,0xe6,0x9b,0xaa,0xe6,
0x26,0xeb,0x98,0x09,0xbe,0x60,0x02,0x69,
0x80,0x00,0x96,0x00,0x00,0x28,0x00,0x86
};


unsigned char level = 1;
/*
short wave(unsigned char x){

	unsigned char y;

	if(level==1){
		y = cc65_sin(x)+ baseline;
		//gotoxy(1,2);
		//printf("y:%d",y2);
	}


	//24 rows on screen
	//12 per side
	//12*2 = 24
	//There are 24 half blocks per half column
	//

	//round();

	return y;

}
*/

int main(void) { 

	//unsigned char block[8] = {0xA0, 0xBB, 0xAC	}
	unsigned long time_step, time_now;
	unsigned char x,xp1,x2;
	unsigned char y,y1,y2,y3;
	unsigned char colour;
	unsigned short offset;
	unsigned char tail[40][3];
	unsigned char tail_ptr, tail_ptr2;

	unsigned short deg,x_deg;

	POKE( 53272,21);//UPPER CASE/PETSCII MODE
	clrscr();
	bgcolor(0);
  	bordercolor(0);

	//Blank the screen to speed things up
  	POKE(0xd011, PEEK(0xd011) & 0xef);


	memcpy ((void*) SPRITE0_DATA, CursorSprite, sizeof (CursorSprite));


	/* Load and install the mouse driver */
	mouse_load_driver (&mouse_def_callbacks, DRIVER);
	/* Set the VIC sprite pointer */
    *(unsigned char*)SPRITE0_PTR = SPRITE0_DATA / 64;

  	//VIC.bgcolor[0] = 0;


    VIC.spr0_color = COLOR_WHITE;
	VIC.spr_mcolor0 = COLOR_BLUE;
	VIC.spr_mcolor1 = COLOR_PURPLE;


	VIC.spr_mcolor = Spr_EnableBit[0];
	VIC.spr_mcolor = Spr_EnableBit[1];
	VIC.spr_mcolor = Spr_EnableBit[2];
	VIC.spr_mcolor = Spr_EnableBit[3];

	VIC.spr_mcolor = Spr_EnableBit[4];
	VIC.spr_mcolor = Spr_EnableBit[5];
	VIC.spr_mcolor = Spr_EnableBit[6];
	VIC.spr_mcolor = Spr_EnableBit[8];

	VIC.spr_ena = Spr_EnableBit[0];
	VIC.spr_ena = Spr_EnableBit[1];
	VIC.spr_ena = Spr_EnableBit[2];
	VIC.spr_ena = Spr_EnableBit[3];

	VIC.spr_ena = Spr_EnableBit[4];
	VIC.spr_ena = Spr_EnableBit[5];
	VIC.spr_ena = Spr_EnableBit[6];
	VIC.spr_ena = Spr_EnableBit[8];


  	mouse_show();
  	mouse_move (100 , 160);

  	//VIC.spr0_x;
  	//VIC.spr0_y;

	offset = 0;
	colour = 1;
	x=0;
	xp1=0;




	//baseline x axis
	/*textcolor(1);
	for(x=0;x<39;x++){
		gotoxy(x,baseline);
		putchar(0xC0);
	}*/

	//Turn on the screen again
	POKE(0xd011, PEEK(0xd011) | 0x10);

	while(1){
		time_step=(PEEK(160)*65536)+(PEEK(161)*256+PEEK(162))+step_size;

		//y=wave(x);
		//yp1=wave(xp1);
		//textcolor(0);

		//gotoxy(x-1,y);
		//putchar(0xA0);
		//0-360

		//deg=(xp1/40)*360;
/*
		deg=xp1*9 + deg_offset;
		if(deg>360){
			deg=deg-360;
		}

		y = (int)cc65_sin(deg)/36 + baseline;
*/		
		deg=(xp1-1)*9 + deg_offset;
		if(deg>360){deg=deg-360;}
		y1 = (int)cc65_sin(deg)/36 + baseline;
		
		deg=xp1*9 + deg_offset;
		if(deg>360){deg=deg-360;}
		y2 = (int)cc65_sin(deg)/36 + baseline;
		
		deg=(xp1+1)*9 + deg_offset;
		if(deg>360){deg=deg-360;}
		y3 = (int)cc65_sin(deg)/36 + baseline;

		//deg2= (deg + (xp1+1)*9)/2;

		//y1 = cc65_sin(deg1)/36 + baseline;
		//y2 = (int)cc65_sin(deg2)/36 + baseline;

		/*
		textcolor(1);
		gotoxy(2,2);
		printf("                                   ");
		gotoxy(2,2);
		printf("y:%d", deg);
		*/
		tail[tail_ptr][0] = x;
		tail[tail_ptr][1] = y2;
		//tail[tail_ptr][2] = c;

  		//VIC.spr0_x = x;
  		//VIC.spr0_y = y;


		
		textcolor(colour);
		

		gotoxy(x,y2);


		if(y1>y2){
			if(y2>y3){
				POKE(0xC7 , 0x12);//Reverse on
				putchar(0xBF);//right diag
				POKE(0xC7 , 0);//Reverse off

			}
			else{// if(y2<y3){
				putchar(0xE2);//bottom flat
			}
			/*else{//y2==y3
				POKE(0xC7 , 0x12);//Reverse on
				putchar(0xE2);//top flat
				POKE(0xC7 , 0);//Reverse off
			}*/
		}
		else if(y1<y2){
			if(y2<y3){
				putchar(0xBF);//left diag
			}
			else{//if(y2>y3){

				POKE(0xC7 , 0x12);//Reverse on
				putchar(0xE2);//top flat
				POKE(0xC7 , 0);//Reverse off


			}
			/*
			else{//y2=y3
				putchar(0xE2);				
			}*/
		}
		else{//y1==y2
			if(y2>y3){

				POKE(0xC7 , 0x12);//Reverse on
				putchar(0xE2);//top flat
				POKE(0xC7 , 0);//Reverse off


			}
			else if(y2<y3){
				putchar(0xE2);//bottom flat
			}
			else{//y2=y3
				putchar(0xC0);
			}
		}		




/*
		//baseline x axis
		textcolor(1);
		gotoxy(x,baseline);
		putchar(0xC0);
*/


		//for(n=0;n<tail_length;n++){
			textcolor(0);
			tail_ptr2 = tail_ptr+1;
			if(tail_ptr2>tail_length){
				tail_ptr2 = 0;
			}
			//Skip over the baseline
			//if(tail[tail_ptr2][1]!=baseline){
				gotoxy(tail[tail_ptr2][0], tail[tail_ptr2][1]);
				putchar(0xA0);//write over the char
			//}
			//textcolor(1);
			//gotoxy(2,2);
			//printf("         ");
			//printf("x:%d y:%d",tail[tail_ptr2][0],tail[tail_ptr2][1]);
		//}

		++tail_ptr;
		if(tail_ptr>(tail_length)){
			tail_ptr = 0;
		}



		

        mouse_info (&mouse);
        //gotoxy (0, 2);
        //cprintf (" X  = %3d\r\n", mouse.pos.x);
        //cprintf (" Y  = %3d\r\n", mouse.pos.y);

        x2=x*8;
        y=y2*8;
		if((mouse.pos.x < x2+delta_x && mouse.pos.x > x2-delta_x)){
			if((mouse.pos.y < y+delta_y && mouse.pos.y > y-delta_y)){
				++hits;
		        gotoxy (1, 23);
		        textcolor(1);
		        cprintf ("hits = %d\r\n", hits);

				bordercolor(colour);

			}
			else{
				bordercolor(0);
			}
		}
		else{
			bordercolor(0);
		}

		/*y2=y;
		if(y>baseline){

			//POKE(COLOUR_RAM + offset , colour);


			gotoxy(x,y);
			putchar(0xA0);
			//y2= 
			while((y2-baseline)>0){
				gotoxy(1,1);
				printf("y2:%d",y2);
				gotoxy(x,y2);
				putchar(0xA0);
				//POKE(CHAR_RAM + offset , 0xA0);
				--y2;

			}

		}
		else{

		}
		*/
			//wx = y % 2;

			/*
			if(wx==0){
				if(wxp1==1){
					//0xAC
					POKE(CHAR_RAM + offset , 0xAC);
					POKE(COLOUR_RAM + offset , colour);
				}
				else if (wxp1==2){
					POKE(0xC7 , 0x12);//Reverse on
					//REV
					//0xE1
					POKE(CHAR_RAM + offset , 0xE1);
					POKE(COLOUR_RAM + offset , colour);
					POKE(0xC7 , 0);//Reverse off

				}
				//0xA0
				POKE(CHAR_RAM + offset , 0xA0);
				POKE(COLOUR_RAM + offset , colour);
			}
			else if(wx==1){
				if(wxp1==1){
					//0xA2
					POKE(CHAR_RAM + offset , 0xA2);
					POKE(COLOUR_RAM + offset , colour);

				}
				else if (wxp1==2){
					//REV
					//0xBE
					POKE(0xC7 , 0x12);//Reverse on
					POKE(CHAR_RAM + offset , 0xBE);
					POKE(COLOUR_RAM + offset , colour);
					POKE(0xC7 , 0);//Reverse off
				}
				//0xBB
				POKE(CHAR_RAM + offset , 0xBB);
				POKE(COLOUR_RAM + offset , colour);
			}
			else if(wx==2){
				if(wxp1==1){
					//REV
					//0xBC
					POKE(0xC7 , 0x12);//Reverse on
					POKE(CHAR_RAM + offset , 0xBC);
					POKE(COLOUR_RAM + offset , colour);
					POKE(0xC7 , 0);//Reverse off
				}
				else if(wxp1==2){
					//REV
					//0xA0
					POKE(0xC7 , 0x12);//Reverse on
					POKE(CHAR_RAM + offset , 0xA0);
					POKE(COLOUR_RAM + offset , colour);
					POKE(0xC7 , 0);//Reverse off
				}
				//0xA1
				POKE(CHAR_RAM + offset , 0xA1);
				POKE(COLOUR_RAM + offset , colour);	

			}
			*/
		//}
		/*offset = offset + 40;
		if(offset>1000){offset=0;}
		*/

		//colour++;
		//if(colour>15){colour=0;}



		//putc(block[i]);

		time_now = (PEEK(160)*65536)+(PEEK(161)*256+PEEK(162));

		while(time_now<time_step){
			time_now = (PEEK(160)*65536)+(PEEK(161)*256+PEEK(162));
		}

		xp1=xp1+x_step;
		x_deg=xp1*9;
		if((x_deg)>360){
			x_step++;
			if(x_step>x_step_max){
				x_step=1;
			}
			xp1=0;
		}

		x=x+1;
		//xp1=x+1;
		if(x>39){
			x=0;
			colour++;
			if(colour>15){colour=1;}

			deg_offset=deg_offset+deg_step;
			if(deg_offset>360){deg_offset=0;}

		}

	}

    return 0; 
}
