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
	char *grafic;
	int gsize;
	int xpos;
	int ypos;
	struct list_head list;
};

struct obstacle {
	char *grafic;
	int gsize;
	int xpos;
	int ypos;
	int hit;
	struct list_head list;
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

void initObstacle(struct obstacle **obs, int xpos, int ypos) {
	*obs = (struct obstacle*) malloc(sizeof(struct obstacle));
	(*obs)->gsize = 2;
	
	(*obs)->grafic = (char *) malloc(sizeof(char) * (*obs)->gsize);
	strncpy((*obs)->grafic, "#\0", (*obs)->gsize);
	(*obs)->xpos = xpos; 
	(*obs)->ypos = ypos;
	(*obs)->hit = 0;
}

void initBullet(struct bullet **b, int xpos, int ypos) {
	*b = (struct bullet*) malloc(sizeof(struct bullet));
	(*b)->gsize = 2;
	
	(*b)->grafic = (char *) malloc(sizeof(char) * (*b)->gsize);
	strncpy((*b)->grafic, "|\0", (*b)->gsize);
	(*b)->xpos = xpos + 3;
	(*b)->ypos = ypos - 1;
}

void updatebullet(struct list_head *head) {
	if (list_empty(head)) {
		return;
	} 
	struct bullet *b = NULL;
	list_for_each_entry(b, head, list) {
		if (b->ypos > 0) {
			b->ypos -= 1;
		} else if (b->ypos <= 0){
			list_del_entry(&b->list);
			free(b->grafic);
			free(b);
		}
	}
}

void updateobstacle(struct list_head *head) {
	if (list_empty(head)) {
		return;
	} 
	struct obstacle *obs = NULL;
	list_for_each_entry(obs, head, list) {
		if (obs->hit){
			list_del_entry(&obs->list);
			free(obs->grafic);
			free(obs);
		}	
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

void printBullet(struct list_head *bulletHead) {
	if (list_empty(bulletHead))
		return;
	struct bullet *b = NULL;
	list_for_each_entry(b, bulletHead, list) {
		mvprintw(b->ypos, b->xpos, b->grafic);
	}
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

void initObstacleList(struct list_head *head, int offset) {

	struct obstacle *obs= NULL;

	for(int i = offset; i < offset + 15; i++){
		if (list_empty(head)) {
			initObstacle(&obs, offset, LINES - 7);
			INIT_LIST_HEAD(&obs->list);
			list_add(&obs->list, head);
		} else {
			initObstacle(&obs, i, LINES - 7);
			list_add(&obs->list, head);
		}
	}

	for(int i = offset; i < offset + 15; i++){
		initObstacle(&obs, i, LINES - 6);
		list_add(&obs->list, head);
	}

}

void printObstacles(struct list_head *head) {
	struct obstacle *ptr = NULL;

	list_for_each_entry(ptr, head, list) {
		mvprintw(ptr->ypos, ptr->xpos, ptr->grafic);
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

void checkCollision(struct list_head *invaderHead, struct list_head *bulletHead, struct list_head *obstacleHead) {
	struct invader *iptr = NULL;
	struct bullet *bptr = NULL;

	// check collision between bullet and invader
	list_for_each_entry(iptr, invaderHead, list) {
		list_for_each_entry(bptr, bulletHead, list){ 
			if(!iptr->destroyed && bptr->xpos >= iptr->xpos && bptr->xpos <= iptr->xpos+3 && 
					bptr->ypos >= iptr->ypos && bptr->ypos <= iptr->ypos + 3) {
				iptr->hit = 1;
				bptr->ypos = -1;
			}
		}	
	}
	// check collision between bullet and obstacle
	bptr = NULL;
	struct obstacle *optr = NULL;
	list_for_each_entry(optr, obstacleHead, list) {
		list_for_each_entry(bptr, bulletHead, list){ 
			if(optr->xpos == bptr->xpos && optr->ypos == bptr->ypos) {
				optr->hit = 1;
				bptr->ypos = -1;
			}
		}	
	}
	updatebullet(bulletHead);
	updateobstacle(obstacleHead);
}

int main() {
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
	
	/* Invader Doubly Linked List */
	static LIST_HEAD(invaderList);
	initInvaderList(&invaderList);

	/* Bullet Doubly Linked List */
	static LIST_HEAD(bulletList);

	/* Obstacles Doubly Linked List */
	static LIST_HEAD(obstacleList);
	for (int i = 0; i <=6; i++) {
		initObstacleList(&obstacleList, 10 + i * 20);
	}


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
						predator.xpos -=1;
						break;
					case 'k':
						predator.xpos +=1;
						break;
					case ' ':
						initBullet(&shot, predator.xpos, predator.ypos);
						if(list_empty(&bulletList)) {
							INIT_LIST_HEAD(&shot->list);
						}
						list_add(&shot->list, &bulletList);
						shot = NULL;
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

			//updatebullet(&bulletList);
			checkCollision(&invaderList, &bulletList, &obstacleList);
			printInvaders(&invaderList, sprite, step);
			printPlayer(&predator);
			printBullet(&bulletList);
			printObstacles(&obstacleList);
			step = 0;
			ms = 0.0;
			refresh();
		} 
	}
	destroyPlayer(&predator);
	return(0);
}
