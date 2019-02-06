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

#define KOALA_DRIVER          "c64-pot.mou"
#define MOUSE_DRIVER          "c64-1351.mou"
#define JOY_DRIVER          "c64-joy.mou"




unsigned short hits=0;
unsigned short misses=0;
unsigned short misses_max=4;
unsigned short level=1;
unsigned short level_max=15;
unsigned short step_size=4;
unsigned short step_max=4;
unsigned char baseline=12;
unsigned char tail_length=10;
unsigned char delta_x=25;
unsigned char delta_y=25;
unsigned short deg_offset=30;
unsigned short deg_step=10;
unsigned char x_step=1;
unsigned char x_step_max=10;
short amplitude=36;

unsigned char tail[40][3];
unsigned char tail2[40][3];

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

void print_level(){
	gotoxy (30, 24);
	textcolor(5);
	cprintf ("level: %d \r\n", level);
}

void print_hits(){
	gotoxy (1, 24);
	textcolor(7);
	cprintf ("hits: %d  \r\n", hits);
}



void print_misses(){
	gotoxy (15, 24);
	textcolor(8);
	cprintf ("misses: %d \r\n", misses);
}

/*
void print_speed(){
	gotoxy (22, 24);
	textcolor(8);
	cprintf ("speed: %d  \r\n", x_step);
}

void print_deg_steps(){
	gotoxy (22, 24);
	textcolor(8);
	cprintf ("speed: %d  \r\n", step_max-level);

}
	print_amplitude();
*/


void level_one(){
	level=1;
	hits=0;
	misses=0;
	step_size=step_max;
	x_step=1;
	amplitude=36;
	print_level();
	print_hits();
	print_misses();
	//print_speed();
	//print_deg_steps();
	//print_amplitude();
}

int main(void) { 

	//unsigned char block[8] = {0xA0, 0xBB, 0xAC	}
	unsigned long time_step, time_now;
	unsigned char x,xp1,x2;
	unsigned char y,y1,y2,y3;
	unsigned char colour,c;
	unsigned short offset;
	unsigned char tail_ptr, tail_ptr2;
	unsigned short n;
	unsigned short deg,x_deg;

	unsigned char loop;

	POKE( 53272,23);//UPPER CASE/PETSCII MODE
	clrscr();
	bgcolor(1);
  	bordercolor(1);

  	textcolor(0);
	gotoxy (8, 10);
	cprintf ("Select your input device:",);
	
	gotoxy (13, 12);
	textcolor(2);
	cprintf ("1 -> Koala Pad");
	gotoxy (13, 13);
	textcolor(4);
	cprintf ("2 -> 1351 Mouse");
	gotoxy (13, 14);
	textcolor(6);
	cprintf ("3 -> Joystick");

	colour=2;
	loop=1;
	while( loop==1 )
	{

	  	textcolor(colour);
		gotoxy (15, 3);
		cprintf ("SONIC SYNC",);

		if( kbhit() )
		{
			c = cgetc();

			switch( c )
			{
				case 0x31:
					/* Load and install the koala pad driver */
					mouse_load_driver (&mouse_def_callbacks, KOALA_DRIVER);
					loop=0;
					break;

				case 0x32:
					/* Load and install the mouse driver */
					mouse_load_driver (&mouse_def_callbacks, MOUSE_DRIVER);
					loop=0;
					break;

				case 0x33:
					/* Load and install the joystick driver */
					mouse_load_driver (&mouse_def_callbacks, JOY_DRIVER);
					loop=0;
					break;
			}

		}
		colour++;
		if(colour>15){colour=2;}
	}

	bgcolor(0);
  	bordercolor(0);


	//Blank the screen to speed things up
  	POKE(0xd011, PEEK(0xd011) & 0xef);

  	clrscr();
	POKE( 53272,21);//UPPER CASE/PETSCII MODE


	memcpy ((void*) SPRITE0_DATA, CursorSprite, sizeof (CursorSprite));



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
	tail_ptr=0;


	//baseline x axis
	/*textcolor(1);
	for(x=0;x<39;x++){
		gotoxy(x,baseline);
		putchar(0xC0);
	}*/

	//Turn on the screen again
	POKE(0xd011, PEEK(0xd011) | 0x10);

	//Little effect
	/*
	while(1){
		for(c=1;c<15;c++){
			bordercolor(c);

			VICWaitNotVBlank(); 
            VICWaitVBlank(); 

		}

	}
	*/
	print_level();
	print_hits();
	print_misses();
	
	time_step=(PEEK(160)*65536)+(PEEK(161)*256+PEEK(162))+step_size;

	while(1){

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






		x2=xp1;

		for(n=0;n<tail_length;n++){

			deg=x2*9 + deg_offset;
			if(deg>360){deg=deg-360;}

			tail[n][0] = x2;
			tail[n][1] = (int)cc65_sin(deg)/amplitude + baseline;

			if(x2==0){
				x2=40;
			}

			x2--;



		}

		for(n=1;n<tail_length;n++){


			y1 = tail[n+1][1];
			y2 = tail[n][1];
			y3 = tail[n-1][1];


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
			//tail[tail_ptr][0] = x;
			//tail[tail_ptr][1] = y2;
			//tail[tail_ptr][2] = c;

	  		//VIC.spr0_x = x;
	  		//VIC.spr0_y = y;

			textcolor(0);
			gotoxy(tail2[n][0], tail2[n][1]);
			putchar(0xA0);//write over the char
			
			//textcolor(colour);
			textcolor(n);

			gotoxy(tail[n][0],y2);

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
				//dy1=y2-y1;

				if(y2<y3){
					//dy3=y3-y2;
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
					//dy=y2-y3;

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






		}

		for(n=0;n<tail_length;n++){
			tail2[n][0]=tail[n][0];
			tail2[n][1]=tail[n][1];

		}

        mouse_info (&mouse);
        //gotoxy (0, 2);
        //cprintf (" X  = %3d\r\n", mouse.pos.x);
        //cprintf (" Y  = %3d\r\n", mouse.pos.y);

		x = tail[0][0];

        x2=x*8;
        y=y2*8;
		if((mouse.pos.x < x2+delta_x && mouse.pos.x > x2-delta_x)){
			if((mouse.pos.y < y+delta_y && mouse.pos.y > y-delta_y)){
				++hits;
				print_hits();

				if(misses>=1){
					misses--;
					print_misses();
				}
				
				//bordercolor(colour);
				for(c=1;c<15;c++){bordercolor(c);}

				if(hits>99){
					for(n=0;n<25;n++){
						for(c=1;c<15;c++){
							bordercolor(c);
							VICWaitNotVBlank(); 
				            VICWaitVBlank(); 
						}
					}
					hits=0;
					level++;
					if(level==level_max){
						level_one();
					}

					if(step_size>1){
						step_size=step_max-level;
					}
					//print_speed();
					print_hits();
					print_level();

				}
				bordercolor(0);

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
	


		xp1=xp1+x_step;
		x_deg=xp1*9;
		if((x_deg)>360){
			xp1=0;

			if(level>3){
				x_step++;
				if(x_step>x_step_max || x_step>level){
					x_step=1;
				}
			}
		}

		x=x+1;
		//xp1=x+1;
		if(x>39){
			x=0;
			colour++;
			if(colour>15){colour=1;}

			if(level>6){
				amplitude++;
				if(amplitude>70){amplitude=25;}
			}

			deg_offset=deg_offset+deg_step;
			if(deg_offset>360){deg_offset=0;}

			//if(hits>1){
				//hits--;
				misses++;
				print_misses();
				if(misses==misses_max){level_one();}
			//}

		}


		time_now = (PEEK(160)*65536)+(PEEK(161)*256+PEEK(162));

		while(time_now<time_step){
			time_now = (PEEK(160)*65536)+(PEEK(161)*256+PEEK(162));
		}
		time_step=(PEEK(160)*65536)+(PEEK(161)*256+PEEK(162))+step_size;


	}

    return 0; 
}
