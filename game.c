#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
 
#define HURDLE_ARRAY_SIZE 10

const float FPS = 60;
const int SCREEN_W = 800;
const int SCREEN_H = 600;
const int BOUNCER_W = 32;
const int BOUNCER_H = 48;
const float WALL_EDGE_DISTANCE = 100;
const float WALL_THICKNESS = 25;

enum MYKEYS {
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

typedef struct {
   int x;
   int y;
   int w;
   int h;
   ALLEGRO_BITMAP *bmp;
} HURDLE;

bool check_for_hurdle_collisions(HURDLE *hurdle_ptr,int bouncer_x,int bouncer_y){
   HURDLE a_hurdle = *hurdle_ptr;
   printf("%d,%d\n", bouncer_x,bouncer_y);
   if (bouncer_x < a_hurdle.x + BOUNCER_W * 3 && bouncer_x + BOUNCER_W > a_hurdle.w && bouncer_y + BOUNCER_H > a_hurdle.h && bouncer_y < a_hurdle.y + BOUNCER_H){
      return true;
   }
   return false;
}

HURDLE create_random_hurdle(){
   const int ROAD_WIDTH = SCREEN_W - 2 * (WALL_THICKNESS + WALL_EDGE_DISTANCE) - BOUNCER_W * 3;
   float hurdle_x = WALL_EDGE_DISTANCE + WALL_THICKNESS ;
   float hurdle_y = 0;

   int static i = 0;
   ALLEGRO_BITMAP *hurdle_bmp = al_create_bitmap(BOUNCER_W * 3, BOUNCER_H);
   al_set_target_bitmap(hurdle_bmp);
   al_clear_to_color(al_map_rgb(255, 0, 0));
   int new_offset = rand() % ROAD_WIDTH ;
   printf("%d\n", new_offset);
   HURDLE a_hurdle = { hurdle_x + new_offset ,hurdle_y + 5 * i * BOUNCER_H, BOUNCER_W * 3, BOUNCER_H,hurdle_bmp } ;

   i = (i + 1) % HURDLE_ARRAY_SIZE;

   return a_hurdle;
}

void draw_walls(){
   al_draw_filled_rectangle(WALL_EDGE_DISTANCE , 0.0 , WALL_EDGE_DISTANCE+WALL_THICKNESS, SCREEN_H, al_map_rgb(255,255,255));
   al_draw_filled_rectangle(SCREEN_W - WALL_EDGE_DISTANCE , 0.0 , SCREEN_W - (WALL_EDGE_DISTANCE+WALL_THICKNESS), SCREEN_H, al_map_rgb(255,255,255));
}

int main(int argc, char **argv)
{
   const int ROAD_WIDTH = SCREEN_W - 2 * (WALL_THICKNESS + WALL_EDGE_DISTANCE) - BOUNCER_W * 3;
   float hurdle_x = WALL_EDGE_DISTANCE + WALL_THICKNESS ;
   float hurdle_y = 0;
   ALLEGRO_DISPLAY *display = NULL;
   ALLEGRO_EVENT_QUEUE *event_queue = NULL;
   ALLEGRO_TIMER *timer = NULL;
   ALLEGRO_BITMAP *bouncer = NULL;
   ALLEGRO_BITMAP *hurdle_bmp = NULL;

   HURDLE hurdles[HURDLE_ARRAY_SIZE];

   srand(time(NULL));
   float universal_dy = 4.0;
   float bouncer_x = SCREEN_W / 2.0 - BOUNCER_W / 2.0;
   float bouncer_y = SCREEN_H / 2.0 - BOUNCER_H / 2.0;

   bool key[4] = { false, false, false, false };
   bool redraw = true;
   bool doexit = false;

   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }
 
   if(!al_install_keyboard()) {
      fprintf(stderr, "failed to initialize the keyboard!\n");
      return -1;
   }

   timer = al_create_timer(1.0 / FPS);
   if(!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }
 
   display = al_create_display(SCREEN_W, SCREEN_H);
   if(!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }
 
   bouncer = al_create_bitmap(BOUNCER_W, BOUNCER_H);
   if(!bouncer) {
      fprintf(stderr, "failed to create bouncer bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   if(!bouncer) {
      fprintf(stderr, "failed to create bouncer bitmap!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

 
   al_set_target_bitmap(bouncer);
   al_clear_to_color(al_map_rgb(255, 0, 255));
 
   for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){
      hurdle_bmp = al_create_bitmap(BOUNCER_W * 3, BOUNCER_H);
      al_set_target_bitmap(hurdle_bmp);
      al_clear_to_color(al_map_rgb(255, 0, 0));
      int new_offset = rand() % ROAD_WIDTH ;
      printf("%d\n", new_offset);
      HURDLE a_hurdle = { hurdle_x + new_offset ,hurdle_y + 5 * i * BOUNCER_H, BOUNCER_W * 3, BOUNCER_H,hurdle_bmp } ;
      hurdles[i] = a_hurdle;
   }
 
   al_set_target_bitmap(al_get_backbuffer(display));

 
   event_queue = al_create_event_queue();
   if(!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_bitmap(bouncer);
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }
   
   al_init_primitives_addon();
 
   al_register_event_source(event_queue, al_get_display_event_source(display));
 
   al_register_event_source(event_queue, al_get_timer_event_source(timer));

   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0,0,0));
   al_flip_display();
 
   al_start_timer(timer);
 
   while(!doexit)
   {
      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);
 
      if(ev.type == ALLEGRO_EVENT_TIMER) {
         if(key[KEY_UP] && bouncer_y >= 4.0) {
            bouncer_y -= 4.0;
         }

         if(key[KEY_DOWN] && bouncer_y <= SCREEN_H - BOUNCER_H - 4.0) {
            bouncer_y += 4.0;
         }

         if(key[KEY_LEFT] && bouncer_x - ( WALL_EDGE_DISTANCE + WALL_THICKNESS ) >= 4.0) {
            bouncer_x -= 4.0;
         }

         if(key[KEY_RIGHT] && bouncer_x + ( WALL_EDGE_DISTANCE + WALL_THICKNESS ) <= SCREEN_W - BOUNCER_W - 4.0) {
            bouncer_x += 4.0;
         }

         for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){

            HURDLE a_hurdle = hurdles[i];
            // printf("%d,%d\n", bouncer_x,bouncer_y);
            bool cond1 = bouncer_x < a_hurdle.x + a_hurdle.w;
            bool cond2 = bouncer_x + BOUNCER_W > a_hurdle.x;
            bool cond3 = bouncer_y < a_hurdle.y + a_hurdle.h;
            bool cond4 = bouncer_y + BOUNCER_H > a_hurdle.y;

            if (cond1 && cond2 && cond3 && cond4 ) {
               // printf("%d-%d-%d-%d\n", cond1,cond2,cond3,cond4);
               // printf("%d,%d\n", a_hurdle.x,a_hurdle.y);
               doexit = true;
            }

            // printf("%d\n",doexit );
            hurdles[i].y += universal_dy;
            if (hurdles[i].y > SCREEN_H){

            }
         }
         redraw = true;
      }
      else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         break;
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
         switch(ev.keyboard.keycode) {
            case ALLEGRO_KEY_UP:
               key[KEY_UP] = true;
               break;

            case ALLEGRO_KEY_DOWN:
               key[KEY_DOWN] = true;
               break;

            case ALLEGRO_KEY_LEFT: 
               key[KEY_LEFT] = true;
               break;

            case ALLEGRO_KEY_RIGHT:
               key[KEY_RIGHT] = true;
               break;
         }
      }
      else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
         switch(ev.keyboard.keycode) {
            case ALLEGRO_KEY_UP:
               key[KEY_UP] = false;
               break;

            case ALLEGRO_KEY_DOWN:
               key[KEY_DOWN] = false;
               break;

            case ALLEGRO_KEY_LEFT: 
               key[KEY_LEFT] = false;
               break;

            case ALLEGRO_KEY_RIGHT:
               key[KEY_RIGHT] = false;
               break;

            case ALLEGRO_KEY_ESCAPE:
               doexit = true;
               break;
         }
      }
 
      if(redraw && al_is_event_queue_empty(event_queue)) {
         redraw = false;
 

         al_clear_to_color(al_map_rgb(0,0,0));
         draw_walls();
         al_draw_bitmap(bouncer, bouncer_x, bouncer_y, 0);
         for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){
            HURDLE a_hurdle = hurdles[i];
            al_draw_bitmap(a_hurdle.bmp, a_hurdle.x, a_hurdle.y, 0);
         }
         al_flip_display();
 
      }
   }

   al_destroy_bitmap(bouncer);
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);
 
   return 0;
}
