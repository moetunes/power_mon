#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYS_FILE "/sys/class/power_supply/BAT0/uevent"

static char *text = "";
static char text1[30] = "";


int window_loop(){
	int screen_num, width, height;
	unsigned long background, border;
	Window win;
	XEvent ev;
	Display *dis;
    char *fontname;
	XFontStruct *font;
	int text_width, text_width1, texty;

	/* First connect to the display server, as specified in the DISPLAY environment variable. */
	dis = XOpenDisplay(NULL);
	if (!dis) {fprintf(stderr, "\033[0;31mUnable to connect to display\033[0m");return 1;}

	screen_num = DefaultScreen(dis);
	background = BlackPixel(dis, screen_num);
	border = WhitePixel(dis, screen_num);
	width = (XDisplayWidth(dis, screen_num)/5);
	height = (XDisplayHeight(dis, screen_num)/6);
	fontname = "-*-freesans-*-*-*";
	font = XLoadQueryFont(dis, fontname);
	if (!font) {
		fprintf(stderr, "unable to load preferred font: %s using fixed", fontname);
		font = XLoadQueryFont(dis, "fixed");
	}
	GC pen;
	XGCValues values;

	win = XCreateSimpleWindow(dis, DefaultRootWindow(dis),width*4-20,height*5-20,width,height,2,border,background);

	/* create the pen to draw lines with */
	values.foreground = WhitePixel(dis, screen_num);
	values.line_width = 2;
	values.line_style = LineSolid;
	values.font = font->fid;
	pen = XCreateGC(dis, win, GCForeground|GCLineWidth|GCLineStyle|GCFont,&values);
	text_width = XTextWidth(font, text, strlen(text));
	text_width1 = XTextWidth(font, text1, strlen(text1));

	XSelectInput(dis, win, ButtonPressMask|StructureNotifyMask|ExposureMask );
	XSetTransientForHint(dis, win, DefaultRootWindow(dis));

	XMapWindow(dis, win);

	while(1){
		XNextEvent(dis, &ev);
		switch(ev.type){
		case Expose:
            XDrawRectangle(dis, win, pen, 7, 7, width-14, height-14);
            XDrawRectangle(dis, win, pen, 12, 12, width-24, height-24);
   			texty = (height + font->ascent)/2;
   			XDrawString(dis, ev.xany.window, pen, (width-text_width)/2, texty-font->ascent, text, strlen(text));
   			XDrawString(dis, ev.xany.window, pen, (width-text_width1)/2, texty+font->ascent, text1, strlen(text1));
			break;
		case ConfigureNotify:
			if (width != ev.xconfigure.width || height != ev.xconfigure.height) {
				width = ev.xconfigure.width;
				height = ev.xconfigure.height;
				XClearWindow(dis, ev.xany.window);
			}
			break;
        /* exit if a button is pressed inside the window */
		case ButtonPress:
			XCloseDisplay(dis);
			return 0;
		}
	}
}

int main(void) {
    FILE *Batt;
    char  buffer[80];
    char *battstatus, *chargenow, *lastfull;
    int battdo, dummy;
    long nowcharge, fullcharge;

    Batt = fopen( SYS_FILE, "r" ) ;
    if ( Batt == NULL ) {
        fprintf(stderr, "\t\033[0;31mCouldn't find %s\033[0m \n", SYS_FILE);
        return(0) ;
    } else {
        while(fgets(buffer,sizeof buffer,Batt) != NULL) {
            /* Now look for info */
            if(strstr(buffer,"POWER_SUPPLY_STATUS") != NULL) {
                battstatus = strstr(buffer, "=");
                //printf("%s\n", battstatus);
                if(strcmp(battstatus, "=Charging\n") == 0)
                    battdo = 1;
                else if(strcmp(battstatus, "=Discharging\n") == 0)
                    battdo = 2;
                else if(strcmp(battstatus, "=Charged\n") == 0)
                    battdo = 3;
                else if(strcmp(battstatus, "=Full\n") == 0)
                    battdo = 4;
                else
                    battdo = 5;
            }
            if(strstr(buffer,"POWER_SUPPLY_CHARGE_FULL=") != NULL) {
                lastfull = strstr(buffer, "=");
                fullcharge = atoi(lastfull+1);
                //printf("\t%s", lastfull+=1);
            }
            if(strstr(buffer,"POWER_SUPPLY_CHARGE_NOW=") != NULL) {
                chargenow = strstr(buffer, "=");
                nowcharge = atoi(chargenow+1);
                //printf("\t%s ", chargenow+=1);
            }
        }
        fclose(Batt);

        dummy = ((float)nowcharge/fullcharge)*100;
        if((dummy <= 37 && battdo == 2) || battdo >= 3) {
            if(battdo == 1) text = "Power Supply Charging";
            if(battdo == 2) text = "Power Supply Discharging";
            if(battdo == 3) text = "Power Supply Charged";
            if(battdo == 4) text = "Power Supply Full";
            if(battdo == 5) text = "Power Supply Unknown !!";
            snprintf(text1, 29, "Remaining Charge %d %%", dummy);
            window_loop();
        }

        return 0;
    }
}
