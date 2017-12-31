#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
 
#define HURDLE_ARRAY_SIZE 5
#define ROAD_IMAGE "images/road.png"
#define CAR_IMAGE "images/car.png"

const float FPS = 60;
const int SCREEN_W = 1200;
const int SCREEN_H = 600;
const int BOUNCER_W = 64;
const int BOUNCER_H = 96;
const int HURDLE_W = 128;
const int HURDLE_H = 48;
const float WALL_EDGE_DISTANCE = 200;
const float WALL_THICKNESS = 25;

float universal_y = 0;
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

ALLEGRO_BITMAP *load_bitmap_at_size(const char *filename, int w, int h)
{
  ALLEGRO_BITMAP *resized_bmp, *loaded_bmp, *prev_target;

  // 1. create a temporary bitmap of size we want
  resized_bmp = al_create_bitmap(w, h);
  if (!resized_bmp) return NULL;

  // 2. load the bitmap at the original size
  loaded_bmp = al_load_bitmap(filename);
  if (!loaded_bmp)
  {
     al_destroy_bitmap(resized_bmp);
     return NULL;
  }

  // 3. set the target bitmap to the resized bmp
  prev_target = al_get_target_bitmap();
  al_set_target_bitmap(resized_bmp);

  // 4. copy the loaded bitmap to the resized bmp
  al_draw_scaled_bitmap(loaded_bmp,
     0, 0,                                // source origin
     al_get_bitmap_width(loaded_bmp),     // source width
     al_get_bitmap_height(loaded_bmp),    // source height
     0, 0,                                // target origin
     w, h,                                // target dimensions
     0                                    // flags
  );

  // 5. restore the previous target and clean up
  al_set_target_bitmap(prev_target);
  // al_destroy_loaded_bmp(loaded_bmp);

  return resized_bmp;      
}

bool check_collision(int x1,int y1, int w1, int h1, int x2, int y2, int w2 , int h2){
   bool cond1 = x1 < x2 + w2;
   bool cond2 = x1 + w1 > x2;
   bool cond3 = y1 < y2 + h2;
   bool cond4 = y1 + h1 > y2;

   bool cond = cond1 && cond2 && cond3 && cond4;
   return cond;
}

bool check_collision_hurdles(HURDLE *hurdle1 , HURDLE *hurdle2){
   HURDLE a = *hurdle1;
   HURDLE b = *hurdle2;
   bool cond = check_collision(a.x - a.w*3 ,a.y - a.h *3 , a.w*3, a.h*3 , b.x - b.w * 3, b.y - b.h*3 , b.w*3 , b.h*3);
   if(cond){
      printf("collided\n" );
   }
   return cond;
}

HURDLE hurdles[HURDLE_ARRAY_SIZE];

bool check_collision_with_all_of_hurdles(HURDLE ahurdle){
   bool collision = true;
   for(int i = 0 ; i<HURDLE_ARRAY_SIZE;i++){
      collision = collision && check_collision_hurdles(&ahurdle,&hurdles[i]);
   }
   return collision;
}

HURDLE create_random_hurdle(int index_of_empty_hurdle, int y_offset){
   const int ROAD_WIDTH = SCREEN_W - 2 * (WALL_THICKNESS + WALL_EDGE_DISTANCE) - HURDLE_W;
   float hurdle_x = WALL_EDGE_DISTANCE + WALL_THICKNESS ;
   // float hurdle_y = 0 - BOUNCER_H ;
   float hurdle_y = 0 - BOUNCER_H - y_offset;

   int static last_hurdle = -1;
   
   // if(last_hurdle != -1){
   //    hurdle_y -= (hurdles[last_hurdle].y + HURDLE_H * 4);
   // }

   ALLEGRO_BITMAP *hurdle_bmp = al_create_bitmap(HURDLE_W, BOUNCER_H);
   al_set_target_bitmap(hurdle_bmp);
   al_clear_to_color(al_map_rgb(255, 0, 0)); 

   int new_offset = rand() % ROAD_WIDTH ;
   // printf("%d\n", 0 - (BOUNCER_H + distance_since_last_hurdle)  );
   HURDLE a_hurdle = { hurdle_x + new_offset ,hurdle_y  , HURDLE_W, BOUNCER_H,hurdle_bmp } ;
   // last_y_pos = hurdle_y;
   // hurdles[i] = a_hurdle;
   // i = (i + 1) % HURDLE_ARRAY_SIZE;
   hurdles[index_of_empty_hurdle] = a_hurdle;
   last_hurdle = index_of_empty_hurdle;
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
   ALLEGRO_BITMAP *road_bmp1 = NULL;
   ALLEGRO_BITMAP *road_bmp2 = NULL;

   char score[6];

   srand(time(NULL));
   float universal_dy = 6.0;
   float universal_d2y = 0.01;
   float dcar = 8;
   float bouncer_x = SCREEN_W / 2.0 - BOUNCER_W / 2.0;
   float bouncer_y = SCREEN_H / 2.0 - BOUNCER_H / 2.0;

   bool key[4] = { false, false, false, false };
   bool redraw = true;
   bool doexit = false;

   if(!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }
 
   al_init_image_addon();
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
   int road_y = 0;
 
   bouncer = load_bitmap_at_size(CAR_IMAGE, BOUNCER_W, BOUNCER_H);
   road_bmp1 = load_bitmap_at_size(ROAD_IMAGE, SCREEN_W, SCREEN_H);
   road_bmp2 = load_bitmap_at_size(ROAD_IMAGE, SCREEN_W, SCREEN_H);
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

 
   // al_set_target_bitmap(bouncer);
   // al_clear_to_color(al_map_rgb(255, 255, 0));
 
   for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){
      create_random_hurdle( i, 10 * i  * HURDLE_H);
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
   al_init_font_addon(); 
   al_init_ttf_addon();
   ALLEGRO_FONT *font = al_load_ttf_font("arial.ttf",14,0);
 
   al_register_event_source(event_queue, al_get_display_event_source(display));
 
   al_register_event_source(event_queue, al_get_timer_event_source(timer));

   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0,0,0));
   al_flip_display();
 
   al_start_timer(timer);
   
   int last_hurdle_pos = 0; 
   while(!doexit)
   {
      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);
 
      if(ev.type == ALLEGRO_EVENT_TIMER) {
         if(key[KEY_UP] && bouncer_y >= 4.0) {
            bouncer_y -= dcar;
         }

         if(key[KEY_DOWN] && bouncer_y <= SCREEN_H - BOUNCER_H - 4.0) {
            bouncer_y += dcar;
         }

         if(key[KEY_LEFT] && bouncer_x - ( WALL_EDGE_DISTANCE + WALL_THICKNESS ) >= 4.0) {
            bouncer_x -= dcar;
         }

         if(key[KEY_RIGHT] && bouncer_x + ( WALL_EDGE_DISTANCE + WALL_THICKNESS ) <= SCREEN_W - BOUNCER_W - 4.0) {
            bouncer_x += dcar;
         }

         for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){

            HURDLE a_hurdle = hurdles[i];

            if (check_collision(bouncer_x,bouncer_y,BOUNCER_W,BOUNCER_H,a_hurdle.x,a_hurdle.y,a_hurdle.w,a_hurdle.h) ) {
               // printf("%d-%d-%d-%d\n", cond1,cond2,cond3,cond4);
               // printf("%d,%d\n", a_hurdle.x,a_hurdle.y);
               doexit = true;
            }

            // printf("%d\n",doexit );
            hurdles[i].y += universal_dy;
            if (hurdles[i].y > SCREEN_H && ((int) (universal_y - last_hurdle_pos)) > 10 * HURDLE_H ){
            // if (hurdles[i].y > SCREEN_H ){
               create_random_hurdle(i ,(universal_y - last_hurdle_pos));
               last_hurdle_pos = universal_y;
               al_set_target_bitmap(al_get_backbuffer(display));
            }
         }
         universal_y += universal_dy;
         road_y = ((int)(road_y + universal_dy)) % SCREEN_H;
         universal_dy += universal_d2y;
         dcar += universal_d2y/5;
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
         al_draw_bitmap(road_bmp1, 0, road_y-SCREEN_H,0);
         al_draw_bitmap(road_bmp2, 0, road_y,0);
         draw_walls();
         al_draw_bitmap(bouncer, bouncer_x, bouncer_y, 0);
         for(int i = 0 ; i < HURDLE_ARRAY_SIZE; i++){
            HURDLE a_hurdle = hurdles[i];
            if(!(a_hurdle.y + a_hurdle.w > 0  && a_hurdle.y > SCREEN_H) || true){
               al_draw_bitmap(a_hurdle.bmp, a_hurdle.x, a_hurdle.y, 0);
            }
         }
         sprintf(score,"%d",(int) (universal_y / BOUNCER_H ) );
         al_draw_text(font,al_map_rgb(255,255,255), 0, 300, 0, score);
         al_flip_display(); 
      }
   }

   al_destroy_bitmap(bouncer);
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);
 
   return 0;
}
