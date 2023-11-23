// Demonstrate DISP/DRAW env, font setup, and display a text.
// Schnappy 2020
// Based on Lameguy64 tutorial : http://lameguy64.net/svn/pstutorials/chapter1/1-display.html
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#define VMODE 1                 // Video Mode : 0 : NTSC, 1: PAL
#define SCREENXRES 320          // Screen width
#define SCREENYRES 240          // Screen height
#define CENTERX SCREENXRES/2    // Center of screen on x 
#define CENTERY SCREENYRES/2    // Center of screen on y
#define FONTSIZE 8 * 7           // Text Field Height
//DISPENV disp[2];                 // Double buffered DISPENV and DRAWENV
//DRAWENV draw[2];
extern DISPENV disp[2];
extern DRAWENV draw[2];
short db = 0;                      // index of which buffer is used, values 0, 1

//extern void HelloMain();

// void SetDefDispEnv1(DISPENV *disp_, int x,int y,int yy)
// {
	// SetDefDispEnv(disp_, x,y, SCREENXRES,yy);  
// }

// void Print12345(char c1,char c2,char c3,char c4,char c5)
// {
	// char str[6];
	// str[0] = c1;
	// str[1] = c2;
	// str[2] = c3;
	// str[3] = c4;
	// str[4] = c5;
	// str[5] = 0;
	// FntPrint(str);
// }

void c_init(void)
{
    //ResetGraph(0);                 // Initialize drawing engine with a complete reset (0)
    //SetDefDispEnv1(&disp[0]);
	//SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);     // Set display area for both &disp[0] and &disp[1]
    //SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // &disp[0] is on top  of &disp[1]
    //SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);     // Set draw for both &draw[0] and &draw[1]
    //SetDefDrawEnv(&draw[1], 0, 0         , SCREENXRES, SCREENYRES);     // &draw[0] is below &draw[1]
    //if (VMODE)                  // PAL
    //{
    //    SetVideoMode(MODE_PAL);
    //    disp[0].screen.y += 8;  // add offset : 240 + 8 + 8 = 256
    //    disp[1].screen.y += 8;
    //}
    //SetDispMask(1);                 // Display on screen    
    setRGB0(&draw[0], 32, 32, 32); // set color for first draw area
    setRGB0(&draw[1], 32, 32, 32); // set color for second draw area
    draw[0].isbg = 1;               // set mask for draw areas. 1 means repainting the area with the RGB color each frame 
    draw[1].isbg = 1;
    PutDispEnv(&disp[db]);          // set the disp and draw environnments
    PutDrawEnv(&draw[db]);
    FntLoad(960, 0);                // Load font to vram at 960,0(+128)
    FntOpen(32, 32, 320-32, FONTSIZE, 0, 280 ); // FntOpen(x, y, width, height,  black_bg, max. nbr. chars
}

void c_display(void)
{
    //DrawSync(0);                    // Wait for all drawing to terminate
    //VSync(0);                       // Wait for the next vertical blank
    PutDispEnv(&disp[db]);          // set alternate disp and draw environnments
    PutDrawEnv(&draw[db]);  
    db = !db;                       // flip db value (0 or 1)
}

//void c_print(void)
//{
        //FntPrint("Testin'");  // Send string to print stream
        //FntFlush(-1);               // Draw printe stream	
//}

//int c_main(void)
//{
//    c_init();                         // execute init()
//    while (1)                       // infinite loop
//    {   
		//c_print();
		//c_display();
		//HelloMain();
//    }
//    return 0;
//}
