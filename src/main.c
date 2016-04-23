#include <pebble.h>

#ifndef PBL_ROUND
	#define PADING_Y 3
	#define PADING_X 20
#else
	#define PADING_Y 10
	#define PADING_X 40
#endif

#define RESOLUTION_Y 168
#define IMAGE_W 42
#define IMAGE_H 76

Window *window;
GBitmap *B0; GBitmap *B1; GBitmap *B2; GBitmap *B3; GBitmap *B4;
GBitmap *B5; GBitmap *B6; GBitmap *B7; GBitmap *B8; GBitmap *B9;
BitmapLayer *hour1; BitmapLayer *hour2;
BitmapLayer *min1; BitmapLayer *min2;

GBitmap* getBitmapFromChar(char num){
	switch(num){
	case '0':
		return B0;
	case '1':
		return B1;
	case '2':
		return B2;
	case '3':
		return B3;
	case '4':
		return B4;
	case '5':
		return B5;
	case '6':
		return B6;
	case '7':
		return B7;
	case '8':
		return B8;
	case '9':
		return B9;
	default:
		return NULL;
	}
}

void set_time(){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char time_[8];
	strftime(time_, sizeof(time_), clock_is_24h_style() ? "%H%M" : "%I%M", tick_time);
	
	
	bitmap_layer_set_bitmap(hour1, getBitmapFromChar(time_[0]));
	bitmap_layer_set_bitmap(hour2, getBitmapFromChar(time_[1]));
	bitmap_layer_set_bitmap(min1, getBitmapFromChar(time_[2]));
	bitmap_layer_set_bitmap(min2, getBitmapFromChar(time_[3]));
	
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	set_time();
}

void main_window_load(){
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
	B0 = gbitmap_create_with_resource(RESOURCE_ID_RES_0); B1 = gbitmap_create_with_resource(RESOURCE_ID_RES_1);
	B2 = gbitmap_create_with_resource(RESOURCE_ID_RES_2); B3 = gbitmap_create_with_resource(RESOURCE_ID_RES_3);
	B4 = gbitmap_create_with_resource(RESOURCE_ID_RES_4); B5 = gbitmap_create_with_resource(RESOURCE_ID_RES_5);
	B6 = gbitmap_create_with_resource(RESOURCE_ID_RES_6); B7 = gbitmap_create_with_resource(RESOURCE_ID_RES_7);
	B8 = gbitmap_create_with_resource(RESOURCE_ID_RES_8); B9 = gbitmap_create_with_resource(RESOURCE_ID_RES_9);
	
	window_set_background_color(window, GColorBlack);
	
	hour1 = bitmap_layer_create(GRect(PADING_X, PADING_Y, IMAGE_W, IMAGE_H));
	bitmap_layer_set_bitmap(hour1, B0);
	layer_add_child(window_layer, bitmap_layer_get_layer(hour1));
	
	hour2 = bitmap_layer_create(GRect(bounds.size.w - PADING_X - IMAGE_W, PADING_Y, IMAGE_W, IMAGE_H));
	bitmap_layer_set_bitmap(hour2, B1);
	layer_add_child(window_layer, bitmap_layer_get_layer(hour2));
	
	min1 = bitmap_layer_create(GRect(PADING_X, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
	bitmap_layer_set_bitmap(min1, B2);
	layer_add_child(window_layer, bitmap_layer_get_layer(min1));
	
	min2 = bitmap_layer_create(GRect(bounds.size.w - PADING_X - IMAGE_W, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
	bitmap_layer_set_bitmap(min2, B3);
	layer_add_child(window_layer, bitmap_layer_get_layer(min2));
	
	set_time();
}

void main_window_unload(){
	bitmap_layer_destroy(hour1); bitmap_layer_destroy(hour2);
	bitmap_layer_destroy(min1); bitmap_layer_destroy(min2);
	
	gbitmap_destroy(B0); gbitmap_destroy(B1); gbitmap_destroy(B2); gbitmap_destroy(B3); gbitmap_destroy(B4);
	gbitmap_destroy(B5); gbitmap_destroy(B6); gbitmap_destroy(B7); gbitmap_destroy(B8); gbitmap_destroy(B9);
}

void init(){
	window = window_create();
	
	window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	window_stack_push(window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void deinit(){
	window_destroy(window);
}

int main(){
	init();
	app_event_loop();
	deinit();
}