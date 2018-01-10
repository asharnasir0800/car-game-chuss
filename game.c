#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>

#define HURDLE_ARRAY_SIZE 5
#define ROAD_IMAGE "images/road.jpg"
#define CAR_IMAGE "images/car.png"
#define HEART_IMAGE "images/heart2.png"

#define FPS  60.0
#define SCREEN_W  1200
#define SCREEN_H  600
#define CAR_W  64
#define CAR_H  96
#define HURDLE_W  256
#define HURDLE_H  48
#define WALL_EDGE_DISTANCE  200.0
#define WALL_THICKNESS  25.0
#define HEART_W  32
#define HEART_H  32
#define HEART_COUNT 3

float universal_y = 0;
enum MYKEYS {
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ENTER
};

typedef struct {
   int x;
   int y;
   int w;
   int h;
   ALLEGRO_BITMAP *bmp;
} HURDLE;

ALLEGRO_BITMAP *load_bitmap_at_size(const char *filename, int w, int h);
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
HURDLE hurdles[HURDLE_ARRAY_SIZE];
HURDLE create_random_hurdle(int index_of_empty_hurdle, int y_offset);
void draw_walls();

int main()
{
   const int ROAD_WIDTH = SCREEN_W - 2 * (WALL_THICKNESS + WALL_EDGE_DISTANCE) - CAR_W * 3;
   float hurdle_x = WALL_EDGE_DISTANCE + WALL_THICKNESS;
   float hurdle_y = 0;
   ALLEGRO_DISPLAY *display = NULL;
   ALLEGRO_EVENT_QUEUE *event_queue = NULL;
   ALLEGRO_TIMER *timer = NULL;
   ALLEGRO_BITMAP *car = NULL;
   ALLEGRO_BITMAP *hurdle_bmp = NULL;
   ALLEGRO_BITMAP *road_bmp1 = NULL;
   ALLEGRO_BITMAP *road_bmp2 = NULL;
   ALLEGRO_BITMAP *heart_bmp = NULL;

   char score[10];
   char heart_text[2];
   char speed_text[10];
   srand(time(NULL));
   int last_hurdle_pos = 0;
   float universal_dy = 6.0;
   float old_universal_dy = 6.0;
   float universal_d2y = 0.002;
   float universal_dy_limit = 8.0;
   float dcar = 8;
   float car_x = SCREEN_W / 2.0 - CAR_W / 2.0; //Middle of the screen
   float car_y = SCREEN_H  - CAR_H;
   float heart_x = 25.0;
   float heart_y = 50.0;
   int heart_count = HEART_COUNT;


   bool key[4] = { false, false, false, false };
   bool redraw = true;
   bool doexit = false;

   if (!al_init()) {
      fprintf(stderr, "failed to initialize allegro!\n");
      return -1;
   }

   al_init_image_addon();
   if (!al_install_keyboard()) {
      fprintf(stderr, "failed to initialize the keyboard!\n");
      return -1;
   }

   timer = al_create_timer(1.0 / FPS);
   if (!timer) {
      fprintf(stderr, "failed to create timer!\n");
      return -1;
   }

   display = al_create_display(SCREEN_W, SCREEN_H);
   if (!display) {
      fprintf(stderr, "failed to create display!\n");
      al_destroy_timer(timer);
      return -1;
   }
   int road_y = 0;


   car = load_bitmap_at_size(CAR_IMAGE, CAR_W, CAR_H);
   heart_bmp = load_bitmap_at_size(HEART_IMAGE,48, 48);
   road_bmp1 = load_bitmap_at_size(ROAD_IMAGE, SCREEN_W, SCREEN_H);
   road_bmp2 = load_bitmap_at_size(ROAD_IMAGE, SCREEN_W, SCREEN_H);
   if (!(car && road_bmp1 && road_bmp2 && heart_bmp)) {
      fprintf(stderr, "failed to create bitmaps!\n");
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }
   for (int i = 0; i < HURDLE_ARRAY_SIZE; i++) {
      create_random_hurdle(i, 10 * i  * HURDLE_H);
      last_hurdle_pos = 10 * i * HURDLE_H;
   }

   al_set_target_bitmap(al_get_backbuffer(display));


   event_queue = al_create_event_queue();
   if (!event_queue) {
      fprintf(stderr, "failed to create event_queue!\n");
      al_destroy_bitmap(car);
      al_destroy_bitmap(heart_bmp);
      al_destroy_display(display);
      al_destroy_timer(timer);
      return -1;
   }

   al_init_primitives_addon();
   al_init_font_addon();
   al_init_ttf_addon();
   ALLEGRO_FONT *font = al_load_ttf_font("arial.ttf", 20, 0);
   al_register_event_source(event_queue, al_get_display_event_source(display));

   al_register_event_source(event_queue, al_get_timer_event_source(timer));

   al_register_event_source(event_queue, al_get_keyboard_event_source());

   al_clear_to_color(al_map_rgb(0, 0, 0));
   al_flip_display();

   al_start_timer(timer);

   int frames=0;
   int point_at_which_limit_is_reached ;
   bool blinking = false;
   bool draw_car = true;
   while (!doexit)
   { 

   
      ALLEGRO_EVENT ev;
      al_wait_for_event(event_queue, &ev);

      if (ev.type == ALLEGRO_EVENT_TIMER)
      {
         if (key[KEY_UP] && car_y >= 4.0) {
            car_y -= dcar;
         }

         if (key[KEY_DOWN] && car_y <= SCREEN_H - CAR_H - 4.0) {
            car_y += dcar;
         }

         if (key[KEY_LEFT] && car_x - (WALL_EDGE_DISTANCE + WALL_THICKNESS) >= 4.0) {
            car_x -= dcar;
         }
         if (key[KEY_RIGHT] && car_x + (WALL_EDGE_DISTANCE + WALL_THICKNESS) <= SCREEN_W - CAR_W - 4.0) {
            car_x += dcar;
         }

         for (int i = 0; i < HURDLE_ARRAY_SIZE; i++) {

            HURDLE a_hurdle = hurdles[i];

            if (check_collision(car_x, car_y, CAR_W, CAR_H, a_hurdle.x, a_hurdle.y, a_hurdle.w, a_hurdle.h)) {
               if (!blinking)
               {
                  frames = 0;
                  heart_count--;
                  blinking = true;

                  if (heart_count == 0)
                  {
                     doexit = true;
                     printf("dy is %f , d2y is %f \n", universal_dy, universal_d2y);
                  }
               }
               
            }

            if (blinking)
            { 
               if (frames <= 120)
               {
                  if (frames % 5 == 0)
                  {
                     draw_car = !(draw_car);
                  }
               }
               else
               {
                  blinking = false;
                  draw_car = true;
               }
            }
            hurdles[i].y += universal_dy;
            if (hurdles[i].y > SCREEN_H && ((int)(universal_y - last_hurdle_pos)) > 10 * HURDLE_H) {
               create_random_hurdle(i, 0);
               last_hurdle_pos = universal_y;
               al_set_target_bitmap(al_get_backbuffer(display));
            }
         }
         universal_y += universal_dy;
         road_y = ((int)(road_y + universal_dy)) % SCREEN_H;
         if(universal_dy <= universal_dy_limit){
            universal_dy += universal_d2y;
            dcar += universal_d2y;
            point_at_which_limit_is_reached = universal_y;
         }
         else {
            if (universal_y > point_at_which_limit_is_reached + 2000){
               universal_dy_limit += 1;
            }
         }
         redraw = true;
         frames++;
      }
      else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
         break;
      }
      else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
         switch (ev.keyboard.keycode) {
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
      else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
         switch (ev.keyboard.keycode) {
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

      if (redraw && al_is_event_queue_empty(event_queue)) {
         redraw = false;


         al_clear_to_color(al_map_rgb(0, 0, 0));
         al_draw_bitmap(road_bmp1, 0, road_y - SCREEN_H, 0);
         al_draw_bitmap(road_bmp2, 0, road_y, 0);
         if (draw_car)
         {
            al_draw_bitmap(car, car_x, car_y, 0);
         }

         al_draw_bitmap(heart_bmp,heart_x , heart_y, 0);
            
         
         for (int i = 0; i < HURDLE_ARRAY_SIZE; i++) {
            HURDLE a_hurdle = hurdles[i];
            if (!(a_hurdle.y + a_hurdle.h < 0 || a_hurdle.y > SCREEN_H)) {
               al_draw_bitmap(a_hurdle.bmp, a_hurdle.x, a_hurdle.y, 0);
            }
         }
         sprintf(heart_text, "%d", heart_count);
         sprintf(score, "%d", (int) universal_y / CAR_H);
         sprintf(speed_text, "%f",  universal_dy);
         al_draw_text(font, al_map_rgb(255, 255, 255),heart_x + CAR_W ,heart_y , 0, heart_text);
         al_draw_text(font, al_map_rgb(255, 255, 255), SCREEN_W - CAR_W, SCREEN_H - 50, 0, speed_text);
         al_draw_text(font, al_map_rgb(255, 255, 255), 0, SCREEN_H - 50, 0, score);
         al_flip_display();
      }
   }

   al_destroy_bitmap(car);
   al_destroy_timer(timer);
   al_destroy_display(display);
   al_destroy_event_queue(event_queue);

   puts(score);

   return 0;
}

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
bool check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
   bool cond1 = x1 < x2 + w2;
   bool cond2 = x1 + w1 > x2;
   bool cond3 = y1 < y2 + h2;
   bool cond4 = y1 + h1 > y2;

   bool cond = cond1 && cond2 && cond3 && cond4;
   
   return cond;
}

HURDLE create_random_hurdle(int index_of_empty_hurdle, int y_offset) {
   const int ROAD_WIDTH = SCREEN_W - 2 * (WALL_THICKNESS + WALL_EDGE_DISTANCE) - HURDLE_W;
   float hurdle_x = WALL_EDGE_DISTANCE + WALL_THICKNESS;
   // float hurdle_y = 0 - CAR_H ;
   float hurdle_y = 0 - CAR_H - y_offset;


   ALLEGRO_BITMAP *hurdle_bmp = al_create_bitmap(HURDLE_W, CAR_H);
   al_set_target_bitmap(hurdle_bmp);
   al_clear_to_color(al_map_rgb(255, 0, 0));

   int new_offset = rand() % ROAD_WIDTH;
   HURDLE a_hurdle = { hurdle_x + new_offset ,hurdle_y  , HURDLE_W, CAR_H,hurdle_bmp };
   hurdles[index_of_empty_hurdle] = a_hurdle;
   return a_hurdle;
}
