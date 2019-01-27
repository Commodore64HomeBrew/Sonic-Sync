#include <stdio.h>
#include <conio.h>
#include <cc65.h>
#include <cbm.h>
#include <mouse.h>
#include <string.h>

#include <peekpoke.h>

#define KEY_F1  0x85
#define KEY_F3  0x86

#define RVSON  0x12
#define RVSOFF 0x92

#define CHAR_RAM 0x0400
#define COLOUR_RAM 0xD800

#define step_size 2

#define baseline 12
#define tail_length 10

#define SPRITE0_DATA    0x0340
#define SPRITE0_PTR     0x07F8
#define DRIVER          "c64-pot.mou"

/* The mouse sprite (an arrow) */
static const unsigned char CursorSprite[64] = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x0F, 0xE0, 0x00,
    0x0F, 0xC0, 0x00,
    0x0F, 0x80, 0x00,
    0x0F, 0xC0, 0x00,
    0x0D, 0xE0, 0x00,
    0x08, 0xF0, 0x00,
    0x00, 0x78, 0x00,
    0x00, 0x3C, 0x00,
    0x00, 0x1E, 0x00,
    0x00, 0x0F, 0x00,
    0x00, 0x07, 0x80,
    0x00, 0x03, 0x80,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00
};


unsigned char level = 1;

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


int main(void) { 

	//unsigned char block[8] = {0xA0, 0xBB, 0xAC	}
	unsigned long time_step, time_now;
	unsigned char x,xp1,xt;
	unsigned char y, yp1,y2;
	unsigned char wx,wxp1;
	unsigned char colour;
	unsigned short offset;
	unsigned char tail[tail_length][2];
	unsigned char tail_ptr, tail_ptr2;

	unsigned short deg;
	

	memcpy ((void*) SPRITE0_DATA, CursorSprite, sizeof (CursorSprite));


	/* Load and install the mouse driver */
	mouse_load_driver (&mouse_def_callbacks, DRIVER);
	/* Set the VIC sprite pointer */
    *(unsigned char*)SPRITE0_PTR = SPRITE0_DATA / 64;

    VIC.spr0_color = COLOR_WHITE;

  	mouse_show();
  	mouse_move (30 , 30);



	offset = 0;
	colour = 1;
	x=0;
	xp1=0;

	POKE(0xC7 , 0x12);//Reverse on
	clrscr();
	bgcolor(0);
  	bordercolor(0);

	while(1){
		time_step=(PEEK(160)*65536)+(PEEK(161)*256+PEEK(162))+step_size;

		//y=wave(x);
		//yp1=wave(xp1);
		//textcolor(0);

		//gotoxy(x-1,y);
		//putchar(0xA0);
		//0-360

		//deg=(xp1/40)*360;

		deg=xp1*9;
		if(deg>360){

		}
		y = cc65_sin(deg)/36 + baseline;
		
		tail[tail_ptr][0] = x;
		tail[tail_ptr][1] = y;


		//gotoxy(20,20);
		//printf("         ");
		//printf("y:%d",y);
		
		textcolor(colour);
		

		gotoxy(x,y);
		putchar(0xA0);


		//for(n=0;n<tail_length;n++){
			textcolor(0);
			tail_ptr2 = tail_length - tail_ptr;

			gotoxy(tail[tail_ptr2][0], tail[tail_ptr2][1]);
			putchar(0xA0);

			//textcolor(1);
			//gotoxy(2,2);
			//printf("         ");
			//printf("x:%d y:%d",tail[tail_ptr2][0],tail[tail_ptr2][1]);
		//}

		++tail_ptr;
		if(tail_ptr>tail_length){
			tail_ptr = 0;
		}

		
		POKE(0xC7 , 0);//Reverse off

		//baseline x axis
		gotoxy(x,baseline);
		putchar(0xC0);
		POKE(0xC7 , 0x12);//Reverse on





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

		xp1=xp1+1;
		if(xp1>39){xp1=0;}

		x=x+1;
		//xp1=x+1;
		if(x>39){
			x=0;
			colour++;
			if(colour>15){colour=0;}
		}
	}

    return 0; 
}