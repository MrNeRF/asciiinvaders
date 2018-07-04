#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <time.h>
#include <unistd.h>
#include "linkedList.h"

struct player {
	int vdim;
	int hdim;
	char **grafic;
	int xpos;
	int ypos;
};

struct bullet {
	char grafic;
	int xpos;
	int ypos;
};

struct invader {
	char **grafic;
	int vdim;
	int hdim;
	int xpos;
	int ypos;
	int direction;
	int ystepsz;
	int xstepsz;
	int hit;
	int destroyed;
	struct list_head list;
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

void initInvader(struct invader *inv) {
	inv->vdim = 9; // dimension vertical
	inv->hdim = 4; // dimension horizontal
	char **g  = (char **) malloc(inv->vdim * sizeof(char *));
	g[0] = (char *) malloc(inv->hdim * sizeof(char));
	g[1] = (char *) malloc(inv->hdim * sizeof(char));
	g[2] = (char *) malloc(inv->hdim * sizeof(char));
	g[3] = (char *) malloc(inv->hdim * sizeof(char));
	g[4] = (char *) malloc(inv->hdim * sizeof(char));
	g[5] = (char *) malloc(inv->hdim * sizeof(char));
	g[6] = (char *) malloc(inv->hdim * sizeof(char));
	g[7] = (char *) malloc(inv->hdim * sizeof(char));
	g[8] = (char *) malloc(inv->hdim * sizeof(char));
	strncpy(g[0], "@M@\0",  inv->hdim);
	strncpy(g[1], "===\0",  inv->hdim);
	strncpy(g[2], "/ \\\0", inv->hdim);
	strncpy(g[3],  "@M@\0", inv->hdim);
	strncpy(g[4],  "===\0", inv->hdim);
	strncpy(g[5], "\\ /\0", inv->hdim);
	strncpy(g[6],  "\\|/\0", inv->hdim);
	strncpy(g[7],  "-  -\0", inv->hdim);
	strncpy(g[8], "/|\\\0", inv->hdim);
	inv->grafic = g;
	inv->xpos = 0;
	inv->ypos = 0;	
	inv->direction = 1;
	inv->ystepsz = 4;
	inv->xstepsz = 8;
	inv->hit = 0;
	inv->destroyed = 0;
}

void initBullet(struct bullet **b, int xpos, int ypos) {
	*b = (struct bullet*) malloc(sizeof(struct bullet));
	(*b)->grafic = '|';
	(*b)->xpos = xpos + 3;
	(*b)->ypos = ypos - 1;
}

void updatebullet(struct bullet **b) {
	if (b == NULL || (*b) == NULL) {
		return;
	} else if ((*b)->ypos >=0) {
		(*b)->ypos -= 1;
	} else if ((*b)->ypos < 0){
		free(*b);
		*b = NULL;
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

void initInvaderList(struct list_head *head) {
	struct invader *myinvader= NULL;
	for(int i=0; i < 2; i++) {
		for(int j=0; j < 10; j++) {
			myinvader = (struct invader *) malloc(sizeof(struct invader));
			initInvader(myinvader);
			myinvader->xpos = 8 + j * 6;
			myinvader->ypos = 2 + i * 3;

			if( i == 0 && j == 0)
				INIT_LIST_HEAD(&myinvader->list);
			list_add(&myinvader->list, head);
		}
	}
}

void printInvaders(struct list_head *head, int sprite, int step) {
	int changeDirection = 0;
	struct invader *ptr = NULL;
	int maxxpos = 0;
	int minxpos = COLS;

	list_for_each_entry(ptr, head, list) {
		if (step){
			ptr->xpos += ptr->xstepsz * ptr->direction;
			if (ptr->hit)
				ptr->destroyed = 1;
		}
		for(int i = 0; i < 3; i++) {
			if (ptr->destroyed) {
				ptr->hit = 0;
				continue;
			}
			for(int j = 0; j < 4; j++){
				if(ptr->hit) {
					mvprintw(ptr->ypos + i, ptr->xpos + j, &(ptr->grafic[i + 6][j]));
				} else {
					mvprintw(ptr->ypos + i, ptr->xpos + j, &(ptr->grafic[i + 3 * sprite][j]));
				}
			}
		}

		if (ptr->xpos < minxpos)
			minxpos = ptr->xpos;
		if (ptr->xpos + ptr->hdim > maxxpos)
			maxxpos = ptr->xpos;
	}

	if (maxxpos + 4 > COLS - 8 || minxpos < 8) {
		changeDirection = 1;
	}

	if (changeDirection) {
		clear();
		ptr = NULL;
		list_for_each_entry(ptr, head, list) {
			ptr->direction *= -1;
			ptr->ypos += ptr->ystepsz;
			ptr->xpos += ptr->direction * ptr->xstepsz;

			for(int i = 0; i < 3; i++) {
				if (ptr->destroyed)
					continue;
				for(int j = 0; j < 4; j++){
					if(ptr->hit) {
						mvprintw(ptr->ypos + i, ptr->xpos + j, &(ptr->grafic[i + 6][j]));
					} else {
						mvprintw(ptr->ypos + i, ptr->xpos + j, &(ptr->grafic[i + 3 * sprite][j]));
					}
				}
			}
		}
	}

}

void checkCollision(struct list_head *invaderHead, struct bullet **shot) {
	if ( *shot == NULL)
		return;
	struct invader *ptr = NULL;
	list_for_each_entry(ptr, invaderHead, list) {
		if(!ptr->destroyed && (*shot)->xpos >= ptr->xpos && (*shot)->xpos <= ptr->xpos+3 && 
				(*shot)->ypos >= ptr->ypos && (*shot)->ypos <= ptr->ypos + 3) {
			ptr->hit = 1;
			(*shot)->ypos = -1;
			updatebullet(shot);
			return;
		}
	}
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

	static LIST_HEAD(invaderList);
	initInvaderList(&invaderList);

	int sprite = 0;
	int step = 0;
	double spritecounter = 0.0;
	while (running){
		clock_gettime(CLOCK_REALTIME, &now);
		ms += getTimePast(&timepast, &now);

		if(ms > fps) { 
			spritecounter += ms;

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
			clear();
			if (spritecounter > 1000.0f) {
				sprite = (sprite + 1) % 2;
				step = 1;
				spritecounter = 0.0;
			}
			if (shot != NULL) {
				updatebullet(&shot);
			}
			checkCollision(&invaderList, &shot);
			printInvaders(&invaderList, sprite, step);
			printPlayer(&predator);
			printBullet(shot);
			step = 0;
			ms = 0.0;
			refresh();
		} 
	}
	destroyPlayer(&predator);
	return(0);
}
