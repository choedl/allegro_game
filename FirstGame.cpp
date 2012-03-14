#include <iostream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include "objects.h"

// globals
const int WIDTH = 800;
const int HEIGHT = 400;
const int NUM_BULLETS = 5;
const int NUM_COMETS = 10;
enum KEYS { UP, DOWN, LEFT, RIGHT, SPACE };
bool keys[5] = { false, false, false, false, false };

// prototypes
void initShip(SpaceShip &ship, ALLEGRO_BITMAP* image);
void resetShipAnimation(SpaceShip &ship, int position);
void drawShip(SpaceShip &ship);
void moveShipUp(SpaceShip &ship);
void moveShipDown(SpaceShip &ship);
void moveShipLeft(SpaceShip &ship);
void moveShipRight(SpaceShip &ship);

void initBullet(Bullet bullet[], int size);
void drawBullet(Bullet bullet[], int size);
void fireBullet(Bullet bullet[], int size, SpaceShip &ship);
void updateBullet(Bullet bullet[], int size);
void collideBullet(Bullet bullet[], int sizeBullets, Comet comets[], int sizeComets, SpaceShip &ship);

void initComet(Comet comets[], int size, ALLEGRO_BITMAP* cometImage);
void drawComet(Comet comets[], int size);
void startComet(Comet comets[], int size);
void updateComet(Comet comets[], int size);
void collideComet(Comet comets[], int sizeComets, SpaceShip &ship);

int main(void) {

  bool done = false;
  bool redraw = true;
  const int FPS = 60;
  bool isGameOver = false;

  // variables
  SpaceShip ship;
  Bullet bullets[NUM_BULLETS];
  Comet comets[NUM_COMETS];

  ALLEGRO_DISPLAY *display = NULL;
  ALLEGRO_EVENT_QUEUE *event_queue = NULL;
  ALLEGRO_TIMER *timer = NULL;
  ALLEGRO_FONT *font18 = NULL;
  ALLEGRO_BITMAP* shipImage = NULL;
  ALLEGRO_BITMAP* cometImage = NULL;

  if (!al_init()) return -1;

  display = al_create_display(WIDTH, HEIGHT);

  if (!display) return -1;

  al_init_primitives_addon();
  al_install_mouse();
  al_install_keyboard();
  al_init_font_addon();
  al_init_ttf_addon();
  al_init_image_addon();

  event_queue = al_create_event_queue();
  timer = al_create_timer(1.0 / FPS);

  shipImage = al_load_bitmap("Spaceship_sprites_by_arboris_modified.png");
  al_convert_mask_to_alpha(shipImage, al_map_rgb(255, 0, 255));

  cometImage = al_load_bitmap("asteroid-1-96.png");

  srand(time(NULL)); // init random number generator
  initShip(ship, shipImage);
  initBullet(bullets, NUM_BULLETS);
  initComet(comets, NUM_COMETS, cometImage);

  font18 = al_load_font("Arial.ttf", 18, 0);

  al_register_event_source(event_queue, al_get_keyboard_event_source());
  al_register_event_source(event_queue, al_get_mouse_event_source());
  al_register_event_source(event_queue, al_get_display_event_source(display));
  al_register_event_source(event_queue, al_get_timer_event_source(timer));

  al_start_timer(timer);

  while (!done) {
	ALLEGRO_EVENT ev;
	al_wait_for_event(event_queue, &ev);

    if (ev.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;
      if (keys[UP])
    	moveShipUp(ship);
      else if (keys[DOWN])
    	moveShipDown(ship);
      else
    	resetShipAnimation(ship, 1);

      if (keys[LEFT])
    	moveShipLeft(ship);
      else if (keys[RIGHT])
    	moveShipRight(ship);
      else
    	resetShipAnimation(ship, 2);

      if (!isGameOver) {
        updateBullet(bullets, NUM_BULLETS);
        startComet(comets, NUM_COMETS);
        updateComet(comets, NUM_COMETS);
        collideBullet(bullets, NUM_BULLETS, comets, NUM_COMETS, ship);
        collideComet(comets, NUM_COMETS, ship);
        if (ship.lives <= 0) isGameOver = true;
      }
    } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
      switch (ev.keyboard.keycode) {
        case ALLEGRO_KEY_ESCAPE:
          done = true;
          break;
        case ALLEGRO_KEY_UP:
          keys[UP] = true;
          break;
        case ALLEGRO_KEY_DOWN:
          keys[DOWN] = true;
          break;
        case ALLEGRO_KEY_LEFT:
          keys[LEFT] = true;
          break;
        case ALLEGRO_KEY_RIGHT:
          keys[RIGHT] = true;
          break;
        case ALLEGRO_KEY_SPACE:
          keys[SPACE] = true;
          fireBullet(bullets, NUM_BULLETS, ship);
          break;

      }
    } else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
      switch (ev.keyboard.keycode) {
        case ALLEGRO_KEY_ESCAPE:
          done = true;
          break;
        case ALLEGRO_KEY_UP:
          keys[UP] = false;
          break;
        case ALLEGRO_KEY_DOWN:
          keys[DOWN] = false;
          break;
        case ALLEGRO_KEY_LEFT:
          keys[LEFT] = false;
          break;
        case ALLEGRO_KEY_RIGHT:
          keys[RIGHT] = false;
          break;
        case ALLEGRO_KEY_SPACE:
          keys[SPACE] = false;
          break;

      }
    } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) done = true;

    if (redraw && al_is_event_queue_empty(event_queue)) {
      redraw = false;
      if (!isGameOver) {
        drawShip(ship);
        drawBullet(bullets, NUM_BULLETS);
        drawComet(comets, NUM_COMETS);
        al_draw_textf(font18, al_map_rgb(255, 0, 255),
                      5, 5, 0, "Player has %i lives left. Player has destroyed %i objects", ship.lives, ship.score);
      } else {
        al_draw_textf(font18, al_map_rgb(0, 255, 255),
                      WIDTH / 2, HEIGHT / 2, ALLEGRO_ALIGN_CENTRE, "Game Over, Final Score: %i", ship.score);
      }

      al_flip_display();
      al_clear_to_color(al_map_rgb(0, 0, 0));
    }
  }

  al_destroy_bitmap(shipImage);
  al_destroy_bitmap(cometImage);
  al_destroy_event_queue(event_queue);
  al_destroy_display(display);
  al_destroy_timer(timer);
  al_destroy_font(font18);

  return 0;

}

void initShip(SpaceShip &ship, ALLEGRO_BITMAP* image) {
  ship.x = 20;
  ship.y = HEIGHT / 2;
  ship.ID = PLAYER;
  ship.lives = 3;
  ship.speed = 6;
  ship.boundx = 10;
  ship.boundy = 12;
  ship.score = 0;

  ship.maxFrame = 3;
  ship.curFrame = 0;
  ship.frameCount = 0;
  ship.frameDelay = 50;
  ship.frameWidth = 46;
  ship.frameHeight = 41;
  ship.animationColumns = 3;
  ship.animationDirection = 1;

  ship.animationRow = 1;

  ship.image = image;
}

void resetShipAnimation(SpaceShip &ship, int position) {
  if (position == 1)
	ship.animationRow = 1;
  else
	ship.curFrame = 0;
}

void drawShip(SpaceShip &ship) {

  int fx = (ship.curFrame % ship.animationColumns) * ship.frameWidth;
  int fy = ship.animationRow * ship.frameHeight;

  al_draw_bitmap_region(ship.image, fx, fy, ship.frameWidth, ship.frameHeight,
      ship.x - ship.frameWidth / 2, ship.y - ship.frameHeight / 2, 0);

  // draw bounding box
  /*al_draw_filled_rectangle(ship.x - ship.boundx, ship.y - ship.boundy, ship.x + ship.boundx,
		  ship.y + ship.boundy, al_map_rgba(255, 0, 255, 100));*/
}

void moveShipUp(SpaceShip &ship) {
  ship.animationRow = 0;
  ship.y -= ship.speed;
  if (ship.y < 0) ship.y = 0;
}

void moveShipDown(SpaceShip &ship) {
  ship.animationRow = 2;
  ship.y += ship.speed;
  if (ship.y > HEIGHT) ship.y = HEIGHT;
}

void moveShipLeft(SpaceShip &ship) {
  ship.curFrame = 2;
  ship.x -= ship.speed;
  if (ship.x < 0) ship.x = 0;
}

void moveShipRight(SpaceShip &ship) {
  ship.curFrame = 1;
  ship.x += ship.speed;
  if (ship.x > 300) ship.x = 300;
}

void initBullet(Bullet bullet[], int size) {
  for (int i = 0; i < size; i++) {
    bullet[i].ID = BULLET;
    bullet[i].speed = 10;
    bullet[i].live = false;
  }
}

void drawBullet(Bullet bullet[], int size) {
  for (int i = 0; i < size; i++) {
    if (bullet[i].live)
      al_draw_filled_circle(bullet[i].x, bullet[i].y, 2, al_map_rgb(255, 255, 255));
  }
}

void fireBullet(Bullet bullet[], int size, SpaceShip &ship) {
  for (int i = 0; i < size; i++) {
    if (!bullet[i].live) {
      bullet[i].x = ship.x + 17;
      bullet[i].y = ship.y;
      bullet[i].live = true;
      break;
    }
  }
}

void updateBullet(Bullet bullet[], int size) {
  for (int i = 0; i < size; i++) {
    if (bullet[i].live) {
      bullet[i].x += bullet[i].speed;
      if (bullet[i].x > WIDTH)
        bullet[i].live = false;
    }
  }
}

void collideBullet(Bullet bullet[], int sizeBullets, Comet comets[], int sizeComets, SpaceShip &ship) {
  for (int i = 0; i < sizeBullets; i++) {
    if (bullet[i].live) {
      for (int j = 0; j < sizeComets; j++) {
        if (comets[j].live) {
          if (bullet[i].x > (comets[j].x - comets[j].boundx) &&
              bullet[i].x < (comets[j].x + comets[j].boundx) &&
              bullet[i].y > (comets[j].y - comets[j].boundy) &&
              bullet[i].y < (comets[j].y + comets[j].boundy)) {
            bullet[i].live = false;
            comets[j].live = false;
            ship.score++;
          }
        }
      }
    }
  }
}

void initComet(Comet comets[], int size, ALLEGRO_BITMAP* image) {
  for (int i = 0; i < size; i++) {
    comets[i].ID = ENEMY;
    comets[i].speed = 5;
    comets[i].live = false;
    comets[i].boundx = 35;
    comets[i].boundy = 35;

    comets[i].maxFrame = 143;
    comets[i].curFrame = 0;
    comets[i].frameCount = 0;
    comets[i].frameDelay = 2;
    comets[i].frameWidth = 96;
    comets[i].frameHeight = 96;
    comets[i].animationColumns = 21;

    if (rand() % 2)
      comets[i].animationDirection = 1;
    else
      comets[i].animationDirection = -1;

    comets[i].image = image;
  }
}

void drawComet(Comet comets[], int size) {
  for (int i = 0; i < size; i++) {
    if (comets[i].live) {
    	int fx = (comets[i].curFrame % comets[i].animationColumns) * comets[i].frameWidth;
    	int fy = (comets[i].curFrame / comets[i].animationColumns) * comets[i].frameHeight;

    	al_draw_bitmap_region(comets[i].image, fx, fy, comets[i].frameWidth, comets[i].frameHeight,
    			comets[i].x - comets[i].frameWidth / 2, comets[i].y - comets[i].frameHeight / 2, 0);
        /*
    	al_draw_filled_rectangle(comets[i].x - comets[i].boundx, comets[i].y - comets[i].boundy,
    			comets[i].x + comets[i].boundx, comets[i].y + comets[i].boundy, al_map_rgba(255, 0, 255, 100));
    	*/
    }
  }
}

void startComet(Comet comets[], int size) {
  for (int i = 0; i < size; i++) {
    if (!comets[i].live) {
      if (rand() % 500 == 0) {
        comets[i].live = true;
        comets[i].x = WIDTH;
        comets[i].y = 30 + rand() % (HEIGHT - 60);
        break;
      }
    }
  }
}

void updateComet(Comet comets[], int size) {
  for (int i = 0; i < size; i++) {
    if (comets[i].live) {
      if (++comets[i].frameCount >= comets[i].frameDelay) {
        comets[i].curFrame += comets[i].animationDirection;
        if (comets[i].curFrame >= comets[i].maxFrame)
          comets[i].curFrame = 0;
        else if (comets[i].curFrame <= 0)
          comets[i].curFrame = comets[i].maxFrame -1;

        comets[i].frameCount = 0;

      }
      comets[i].x -= comets[i].speed;
    }
  }
}

void collideComet(Comet comets[], int sizeComets, SpaceShip &ship) {
  for (int i = 0; i < sizeComets; i++) {
    if (comets[i].live) {
      if (comets[i].x - comets[i].boundx < ship.x + ship.boundx &&
          comets[i].x + comets[i].boundx > ship.x - ship.boundx &&
          comets[i].y - comets[i].boundy < ship.y + ship.boundy &&
          comets[i].y + comets[i].boundy > ship.y - ship.boundy) {
        ship.lives--;
        comets[i].live = false;
      } else if (comets[i].x < 0) {
        comets[i].live = false;
        ship.lives--;
      }
    }
  }
}










