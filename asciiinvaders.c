#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>

struct player {
	int vdim ;
	int hdim ;
	char **grafic;
	int xpos ;
	int ypos ;
};

struct bullet {
	char grafic;
	int xpos;
	int ypos;
};

void quit() {
	endwin();
}

int kbhit(void)
{
	int ch = getch();

	if (ch != ERR) {
		ungetch(ch);
		return 1;
	} else {
		return 0;
	}
}

void initPlayer(struct player *p) {
	p->vdim = 2; // dimension vertical
	p->hdim = 8; // dimension horizontal
	char **g  = (char **) malloc(p->vdim * sizeof(char *));
	g[0] = (char *) malloc(p->hdim * sizeof(char));
	g[1] = (char *) malloc(p->hdim * sizeof(char));
	strncpy(g[0], "  / \\  \0", p->hdim);
	strncpy(g[1], "#######\0", p->hdim);
	p->grafic = g;
	p->xpos = 5;
	p->ypos = LINES - 2;	
}

void initBullet(struct bullet **b, int xpos, int ypos) {
	*b = (struct bullet*) malloc(sizeof(struct bullet));
	(*b)->grafic = '|';
	(*b)->xpos = xpos + 3;
	(*b)->ypos = ypos - 1;
}

struct bullet *updatebullet(struct bullet *b) {
	if (b == NULL) {
		return NULL;
	} else if (b->ypos >=0) {
		b->ypos -= 1;
		return b;
	} else {
		free(b);
		return NULL;
	}
}

void destroyPlayer(struct player *p) {
	free(p->grafic[0]);
	free(p->grafic[1]);
	free(p->grafic);
}

#define divisor 1000000.0

double getTimePast(struct timespec *past, struct timespec *now) {
	double ms;
	if(past->tv_sec == now->tv_sec) 
	{
		ms = ((double)now->tv_nsec / divisor - (double)past->tv_nsec / divisor);
	}
	else
	{
		ms = (1000.0 -(double) past->tv_nsec / divisor + (double)now->tv_nsec / divisor);
	}
	*past = *now;
	return ms;
}

void printPlayer(struct player *p) {
	for(int i = 0; i < 2; i++) {
		for(int j = 0; j < p->hdim - 1; j++){
			mvprintw(p->ypos + i, p->xpos + j, &(p->grafic[i][j]));
		}
	}
}

void printBullet(struct bullet *b) {
	if (b == NULL)
		return;
	mvprintw(b->ypos, b->xpos, &(b->grafic));
}


int main(int argc, char **argv) {
	int x, y;
	int running = 1;
	double fps = 40.0;
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	scrollok(stdscr, TRUE);
	atexit(quit);
	curs_set(0);

	double ms = 0.0;
	struct timespec timepast;
	struct timespec now;

	struct player predator;
	initPlayer(&predator);
	struct bullet *shot = NULL;
	clock_gettime(CLOCK_REALTIME, &timepast);
	while (running){
		clock_gettime(CLOCK_REALTIME, &now);
		ms += getTimePast(&timepast, &now);

		if(ms > fps) { // 30 fps perr second
			if(kbhit()) {
				switch (getch()){ 
					case 'j': 
						predator.xpos -=4;
						break;
					case 'k':
						predator.xpos +=4;
						break;
					case ' ':
						if (shot == NULL)
							initBullet(&shot, predator.xpos, predator.ypos);
						break;
					case 'q':
						running = 0;
						break;
				}			
			}
			erase();
			printPlayer(&predator);
			if (shot != NULL) {
				shot = updatebullet(shot);
				printBullet(shot);
			}
			ms = 0.0;
			refresh();
		} 
	}
	destroyPlayer(&predator);
	return(0);
}
