#include <stdio.h>
#include <conio.h>
#include <cc65.h>
#include <cbm.h>
#include <mouse.h>
#include <string.h>
#include <peekpoke.h>
#include "main.h"

#define KEY_F1  0x85
#define KEY_F3  0x86

#define RVSON  0x12
#define RVSOFF 0x92

#define CHAR_RAM 0x0400
#define COLOUR_RAM 0xD800

#define SCREEN_W 39
#define SCREEN_H 24

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
unsigned short step_size=6;
unsigned short step_max=6;
unsigned char baseline=12;
unsigned char tail_length=8;
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

unsigned int SIDSIZE = (1024)*8; 			//8kb should be more than enough
unsigned int SIDLOAD = 0x4000;				//Make sure you offset your sid at $4000 using sidereloc -p 40


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
	unsigned short offset,offset2;
	unsigned short n,t;
	unsigned short deg,x_deg;

	unsigned char loop;

	//unsigned char tail_colour[8] = { 0x9a, 0x9F, 0x05, 0x9e, 0x81, 0x1C, 0x95, 0x97 };
	unsigned char tail_colour[8] = { 14, 3, 1, 7, 8, 2, 9, 11 };






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



	//baseline x axis
	/*textcolor(1);
	for(x=0;x<SCREEN_W;x++){
		gotoxy(x,baseline);
		putchar(0xC0);
	}*/


	memcpy((void*)(SIDLOAD),(void*)&SIDFILE,SIDSIZE);	//Load music into memory 
	SIDINIT(); 						//Init SID
	SIDPLAY();						//Play 



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
	level_one();
	print_level();
	print_hits();
	print_misses();
	
	time_step=(PEEK(160)*65536)+(PEEK(161)*256+PEEK(162))+step_size;
	x2=tail_length;
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
		deg=xp1*9 + deg_offset;
		//x2=25;
		t=tail_length+1;
		//for(n=0;n<tail_length;n++){
		n=0;
		do{

			//deg=x2*9 + deg_offset;
			deg=deg+10;
			if(deg>360){deg=deg-360;}

			tail[n][0] = x2;
			tail[n][1] = (int)cc65_sin(deg)/amplitude + baseline;

			if(x2==0){
				x2=40;
			}

			x2--;
			n++;
		}while(n<t);

		//for(n=1;n<tail_length;n++){
		n=1;
		do{

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


	  		//VIC.spr0_x = x;
	  		//VIC.spr0_y = y;



			//write over the char

			//POKE(CHAR_RAM + offset , 0x20);
			//POKE(COLOUR_RAM + offset , 0);

			//textcolor(0);
			//gotoxy(tail2[n][0], tail2[n][1]);
			//putchar(0xA0);//write over the char
			

			offset2 = (tail2[n][1])*40 + tail2[n][0];
			offset = y2*40 + tail[n][0];

			//textcolor(colour);
			//textcolor(n);

			//gotoxy(tail[n][0],y2);
			colour=tail_colour[n-1];
			if(y1>y2){
				if(y2>y3){
					POKE(CHAR_RAM + offset2 , 0x20);
					//right diag
					POKE(CHAR_RAM + offset , 0xFF);
					POKE(COLOUR_RAM + offset , colour);


				}
				else{// if(y2<y3){
					POKE(CHAR_RAM + offset2 , 0x20);
					//bottom flat
					POKE(CHAR_RAM + offset , 0x62);
					POKE(COLOUR_RAM + offset , colour);
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
					POKE(CHAR_RAM + offset2 , 0x20);
					//left diag
					POKE(CHAR_RAM + offset , 0x7F);
					POKE(COLOUR_RAM + offset , colour);
				}
				else{//if(y2>y3){
					POKE(CHAR_RAM + offset2 , 0x20);
					//top flat
					POKE(CHAR_RAM + offset , 0xE2);
					POKE(COLOUR_RAM + offset , colour);
				}
				/*
				else{//y2=y3
					putchar(0xE2);				
				}*/
			}
			else{//y1==y2
				if(y2>y3){
					//dy=y2-y3;
					POKE(CHAR_RAM + offset2 , 0x20);
					//top flat
					POKE(CHAR_RAM + offset , 0xE2);
					POKE(COLOUR_RAM + offset , colour);

				}
				else if(y2<y3){
					POKE(CHAR_RAM + offset2 , 0x20);
					//bottom flat
					POKE(CHAR_RAM + offset , 0x62);
					POKE(COLOUR_RAM + offset , colour);
				}
				else{//y2=y3
					POKE(CHAR_RAM + offset2 , 0x20);
					//middle flat
					POKE(CHAR_RAM + offset , 0x40);
					POKE(COLOUR_RAM + offset , colour);
				}
			}		




	/*
			//baseline x axis
			textcolor(1);
			gotoxy(x,baseline);
			putchar(0xC0);
	*/





			n++;
		}while(n<tail_length);

		//for(n=0;n<tail_length;n++){
		n=0;		
		do{			
			tail2[n][0]=tail[n][0];
			tail2[n][1]=tail[n][1];
			n++;
		}while(n<tail_length);

        mouse_info (&mouse);
        gotoxy (0, 2);
        //cprintf (" x  = %3d\r\n", mouse.pos.x);
        //cprintf (" y  = %3d\r\n", mouse.pos.y);
        cprintf (" x  = %3d\r\n", PEEK(0xd419));
        cprintf (" y  = %3d\r\n", PEEK(0xd41A));
  		mouse_move ( PEEK(0xd419), PEEK(0xd41A));

        x2=tail[1][0]*8;
        y=tail[1][1]*8;
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
		if(x>SCREEN_W){
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
		time_step=time_now+step_size;
		

	}

    return 0; 
}
