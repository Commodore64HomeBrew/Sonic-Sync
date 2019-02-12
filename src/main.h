extern char SIDFILE[]; 
extern void SIDINIT(); 
extern void SIDPLAY();


// - ----------------------------------------------------------------------------------------- - // 
// VIC is assigned to a 16k RAM block. 16k RAM broken down as: // 
// - Screen RAM (Character Map) - 16 blocks (1024 bytes each) // 
// - Character Set - 8 blocks (2048 bytes each) // 
// - Sprite Data - 256 blocks (63+1 bytes each) // 

// Character ROM (VIC always sees this) - 0x1000 and 0x1800, 0x9000 and 0x9800 
// - 2k bytes each 
// - 0x1000-0x17FF is Upper Case PETSCII 
// - 0x1800-0x1FFF is Lower Case PETSCII 

// Program starts at 0x800 // 
// Screen RAM typically starts at 0x400 (0x400-0x7FF) // 
// 0x000-0x3E7 - 1000 bytes of characters (40x25 display) 
// 0x3E8-0x3F7 - 16 bytes unused 
// 0x3F8-0x3FF - 8 bytes of block data addresses (0-255, 63+1 bytes each) 
// - ----------------------------------------------------------------------------------------- - // 
void VICSetPage( char VICBase ) { 
    // 0xDD00 // 
    CIA2.pra = (CIA2.pra & 0xFC) | (3-VICBase);     
    // Lower 2 bits only. Pages are in reverse order. // 
} 
// - ----------------------------------------------------------------------------------------- - // 
void VICSetAddr( char ScreenBase, char CharBase ) { 
    // 0xD018 // 
    VIC.addr = (ScreenBase << 4) | (CharBase << 1); 
    // NOTE: When on Pages 0 or 2, Upper Case is Charset 2 and Lower Case is Charset 3 // 
} 
// - ----------------------------------------------------------------------------------------- - // 


// - ----------------------------------------------------------------------------------------- - // 
#define VIC_TOP_BORDER_TOP (8) 
#define VIC_TOP_BORDER_BOTTOM (50) 
#define VIC_BOTTOM_BORDER_TOP (250) 
#define VIC_BOTTOM_BORDER_BOTTOM (299) 
// - ----------------------------------------------------------------------------------------- - // 
unsigned short VICGetLine() { 
    return ((unsigned short)VIC.rasterline) | ((VIC.ctrl1 & 0x80) ? 0x100 : 0); 
} 
// - ----------------------------------------------------------------------------------------- - // 
void VICWaitVBlank() { 
    while ( VICGetLine() < VIC_BOTTOM_BORDER_TOP ); 
} 
// - ----------------------------------------------------------------------------------------- - // 
void VICWaitNotVBlank() { 
    while ( VICGetLine() >= VIC_BOTTOM_BORDER_TOP ); 
} 
// - ----------------------------------------------------------------------------------------- - // 

// ** My VIC Memory Map ** // 
// 0x0000 - VRAM Map (1st half only) // 
// 0x0400 - (unused) 
// 0x0800 - VRAM Tileset 
// 0x1000 - ROM Upper Case 
// 0x1800 - ROM Lower Case 
// 0x2000 - Sprites (128 total, 63+1 bytes each) 
// 0x2800 - Sprites 
// 0x3000 - Sprites 
// 0x3800 - Sprites 

#define MyVICBase        (0x8000) 
#define MyScreenBase    (MyVICBase+(0x0)) 
#define MyCharBase        (MyVICBase+(0x800)) 
#define MySpriteBase    (MyVICBase+(0x2000)) 

// - ----------------------------------------------------------------------------------------- - // 


// - ----------------------------------------------------------------------------------------- - // 
char Spr_Alloc; 
// - ----------------------------------------------------------------------------------------- - // 
const unsigned char Spr_Bit[] = {0x1,0x2,0x4,0x8, 0x10,0x20,0x40,0x80}; 
const unsigned char Spr_EnableBit[] = {0x0, 0x1,0x3,0x7,0xF, 0x1F,0x3F,0x7F,0xFF}; 
unsigned short Spr_InfoAddr[128]; // Addresses of Sprite 128-255 Information // 
// - ----------------------------------------------------------------------------------------- - // 
void SprInit() { 
    unsigned char idx; 
    for ( idx = 0; idx < 128; idx++ ) { 
        Spr_InfoAddr[idx] = MySpriteBase + (idx<<6) + 63; // Last Byte (64-1) // 
    } 
     
    Spr_Alloc = 0; 
} 
// - ----------------------------------------------------------------------------------------- - // 
void SprAlloc( unsigned char Index, unsigned short x, unsigned char y ) { 
    unsigned char Info = *((unsigned char*)Spr_InfoAddr[Index]); 
         
    // Check for Overlay bit //     
    if ( Info & 0x10 ) { 
        // Recursively call this function // 
        SprAlloc(Index+1,x,y); 
    } 

    // Bail on too many sprites // 
    if ( Spr_Alloc >= 8 ) 
        return; 
     
    // Write Position // 
    VIC.spr_pos[Spr_Alloc].x = x;//(x & 0xFF); 
    VIC.spr_pos[Spr_Alloc].y = y; 
     
    VIC.spr_hi_x = (VIC.spr_hi_x & (~Spr_Bit[Spr_Alloc])) | ((x>255) ? Spr_Bit[Spr_Alloc] : 0 ); 

    // Write Color Mode and Color // 
    VIC.spr_mcolor = (VIC.spr_mcolor & (~Spr_Bit[Spr_Alloc])) | ((Info & 0x80) ? Spr_Bit[Spr_Alloc] : 0 ); 
    VIC.spr_color[Spr_Alloc] = Info;// & 0xF; 

    // Write Sprite Block used // 
    *((unsigned char*)(MyScreenBase+0x3F8+Spr_Alloc)) = 128u + Index; 

    // Next Sprite Slot // 
    Spr_Alloc++; 
} 
// - ----------------------------------------------------------------------------------------- - // 
void SprDraw() { 
    VIC.spr_ena = Spr_EnableBit[Spr_Alloc]; 
    Spr_Alloc = 0; 
} 
// - ----------------------------------------------------------------------------------------- - // 


// - ----------------------------------------------------------------------------------------- - // 
char PadGet1() { 
    return ~CIA1.prb;// & 0x1F; 
} 
// - ----------------------------------------------------------------------------------------- - // 
char PadGet2() { 
    return ~CIA1.pra;// & 0x1F; 
} 
// - ----------------------------------------------------------------------------------------- - // 
#define PAD_UP        (0x1) 
#define PAD_DOWN    (0x2) 
#define PAD_LEFT    (0x4) 
#define PAD_RIGHT    (0x8) 
#define PAD_FIRE    (0x10) 
// - ----------------------------------------------------------------------------------------- - // 
