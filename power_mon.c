/* A light battery warning system
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 2 of the License, or
*  (at your option) any later version.
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYS_FILE "/sys/class/power_supply/BAT0/uevent"
#define MIN_PERCENT 25
#define SEARCHTERM1 "POWER_SUPPLY_STATUS"
#define SEARCHTERM2 "POWER_SUPPLY_CHARGE_FULL="
#define SEARCHTERM3 "POWER_SUPPLY_CHARGE_NOW="

static char *text = "FILE";
static char text1[30] = "ERROR";

void window_loop(){
	unsigned int screen_num, width, height;
	unsigned long background, border;
	Window win;
	XEvent ev;
	Display *dis;
    char *fontname = "-*-terminus-*-*-*-*-*-*-*-*-*-*-*-*";
	XFontStruct *font;
	unsigned int text_width, text_width1, texty;

	dis = XOpenDisplay(NULL);
	if (!dis) {
	    fputs("\033[0;31mPOWERMON :: Unable to connect to display\033[0m", stderr);
	    return;
	}

	screen_num = DefaultScreen(dis);
	background = BlackPixel(dis, screen_num);
	border = WhitePixel(dis, screen_num);
	width = (XDisplayWidth(dis, screen_num)/5);
	height = (XDisplayHeight(dis, screen_num)/6);
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
		    XFreeGC(dis, pen);
			XCloseDisplay(dis);
			return;
		}
	}
}

int main(void) {
    FILE *Batt;
    char  buffer[80];
    char *battstatus, *chargenow, *lastfull;
    unsigned int battdo = 0, dummy;
    long nowcharge, fullcharge;

    Batt = fopen( SYS_FILE, "rb" ) ;
    if ( Batt == NULL ) {
        fprintf(stderr, "\t\033[0;31mCouldn't find %s\033[0m \n", SYS_FILE);
        window_loop();
        return(0) ;
    } else {
        while(fgets(buffer,sizeof buffer,Batt) != NULL) {
            /* Now look for info
            * first search term to match */
            if(strstr(buffer,SEARCHTERM1) != NULL) {
                battstatus = strstr(buffer, "=");
                //printf("%s\n", battstatus);
                if(strcmp(battstatus, "=Discharging\n") == 0)
                    battdo = 2;
                else if(strcmp(battstatus, "=Charging\n") == 0)
                    battdo = 1;
                else if(strcmp(battstatus, "=Charged\n") == 0)
                    battdo = 3;
                else if(strcmp(battstatus, "=Full\n") == 0)
                    battdo = 4;
                else
                    battdo = 5;
            }
            /* Second search term */
            if(strstr(buffer,SEARCHTERM2) != NULL) {
                lastfull = strstr(buffer, "=");
                fullcharge = atoi(lastfull+1);
                //printf("\t%s", lastfull+=1);
            }
            /* Third search term */
            if(strstr(buffer,SEARCHTERM3) != NULL) {
                chargenow = strstr(buffer, "=");
                nowcharge = atoi(chargenow+1);
                //printf("\t%s ", chargenow+=1);
            }
        }
        fclose(Batt);
        if(battdo < 1)
            window_loop();

        dummy = ((float)nowcharge/fullcharge)*100;
        /* if the battery is above MIN_PERCENT don't show the window
           unless it's charged */
        if((dummy <= MIN_PERCENT && battdo == 2) || battdo > 2) {
            if(battdo == 2) text = "Power Supply Discharging";
            else if(battdo == 3) text = "Power Supply Charged";
            else if(battdo == 4) text = "Power Supply Full";
            else if(battdo == 5) text = "Power Supply Unknown !!";
            snprintf(text1, 29, "Remaining Charge %d %%", dummy);
            window_loop();
        }

        return 0;
    }
}
