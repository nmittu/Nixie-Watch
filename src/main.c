#include <pebble.h>

#ifndef PBL_ROUND
	#define PADING_Y 3
	#define PADING_X 10
	#define PADING_CENTER_X 20
#else
	#define PADING_Y 10
	#define PADING_X 40
	#define PADING_CENTER_X 40
#endif

#define RESOLUTION_Y 168
#define IMAGE_W 42
#define IMAGE_H 76
#define CONNECTED_CIR_PADING 3
#define CONNECTED_CIR_RADIUS 5
#define BATT_PADING 3
#define HEALTH_PADING 3

#define VIBE_TIME 0
#define CENTER 1

#define KEY_MINUTES 0

Window *window;
GBitmap *B0; GBitmap *B1; GBitmap *B2; GBitmap *B3; GBitmap *B4;
GBitmap *B5; GBitmap *B6; GBitmap *B7; GBitmap *B8; GBitmap *B9;
BitmapLayer *hour1; BitmapLayer *hour2;
BitmapLayer *min1; BitmapLayer *min2;
int mins;
bool center;

void init();
void deinit();

static void app_connection_handler(bool connected) {
	layer_mark_dirty(window_get_root_layer(window));
	if (!connected){
		vibes_long_pulse();
	}
  APP_LOG(APP_LOG_LEVEL_INFO, "Pebble app %sconnected", connected ? "" : "dis");
}

static void kit_connection_handler(bool connected) {
  APP_LOG(APP_LOG_LEVEL_INFO, "PebbleKit %sconnected", connected ? "" : "dis"); 
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context){
	Tuple *minutes = dict_find(iterator, VIBE_TIME);
	if(minutes){
		mins = minutes->value->uint8;
		persist_write_int(KEY_MINUTES, mins);
	}
	
	Tuple *center_t = dict_find(iterator, CENTER);
	if(center_t){
		center = center_t->value->int8;
		persist_write_bool(CENTER, center);
		
		window_stack_pop(false);
		deinit();
		init();
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context){
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context){
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send success!!");
}

static void update_proc(Layer *layer, GContext *ctx) {
  // Custom drawing happens here!
	GRect bounds = layer_get_bounds(layer);
	
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, bounds, 0, 0);
	
	if(center){
		return;
	}
	
	BatteryChargeState state = battery_state_service_peek();
	
	char batteryT[5];
	snprintf(batteryT, sizeof(batteryT), "%d%%", state.charge_percent);
	
	GSize size1 = graphics_text_layout_get_content_size(batteryT, fonts_get_system_font(FONT_KEY_GOTHIC_24), bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
	
	GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	
	#ifdef PBL_COLOR
		graphics_context_set_text_color(ctx, state.charge_percent>20 ? GColorGreen : GColorRed);
	#else
		graphics_context_set_fill_color(ctx, GColorWhite);
	#endif
	
	graphics_draw_text(ctx, batteryT, font, GRect(bounds.size.w - BATT_PADING - size1.w, bounds.size.h/2-size1.h/2, size1.w, size1.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
	
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, connection_service_peek_pebble_app_connection() ? GColorGreen : GColorRed);
	#else
		graphics_context_set_fill_color(ctx, connection_service_peek_pebble_app_connection() ? GColorWhite : GColorBlack);
	#endif
	
	graphics_fill_circle(ctx,
											 GPoint(bounds.size.w - CONNECTED_CIR_PADING - CONNECTED_CIR_RADIUS * PBL_IF_ROUND_ELSE(3, 1), PBL_IF_RECT_ELSE(CONNECTED_CIR_PADING + CONNECTED_CIR_RADIUS, bounds.size.h/2-size1.h/2-CONNECTED_CIR_RADIUS-CONNECTED_CIR_PADING)),
											 CONNECTED_CIR_RADIUS);
	
	
	HealthValue steps = health_service_sum_today(HealthMetricStepCount);
	
	steps = 99999;
	
	char steps_string[10];
	if(steps >= 1000){
		snprintf(steps_string, sizeof(steps_string), "%d.%dK", (int)steps/1000, (int)((float)(steps-(steps/1000)*1000)/100));
	}else{
		snprintf(steps_string, sizeof(steps_string), "%d", (int)steps);
	}
	
	
	GSize size2 = graphics_text_layout_get_content_size(steps_string, fonts_get_system_font(FONT_KEY_GOTHIC_24), bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight);
	
	graphics_context_set_text_color(ctx, GColorGreen);
	graphics_draw_text(ctx, steps_string, font, GRect(bounds.size.w - HEALTH_PADING * PBL_IF_ROUND_ELSE(3, 1) - size2.w, PBL_IF_RECT_ELSE(bounds.size.h-size2.h-HEALTH_PADING, bounds.size.h/2+HEALTH_PADING), size2.w, size2.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
	
	
}

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
	
	if(mins != 0 && (tick_time->tm_min +(tick_time->tm_hour*60))%mins == 0){
		vibes_double_pulse();
		APP_LOG(APP_LOG_LEVEL_DEBUG, "fmt");
	}
}

void main_window_load(){
	Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
	
	layer_set_update_proc(window_layer, update_proc);
	
	B0 = gbitmap_create_with_resource(RESOURCE_ID_RES_0); B1 = gbitmap_create_with_resource(RESOURCE_ID_RES_1);
	B2 = gbitmap_create_with_resource(RESOURCE_ID_RES_2); B3 = gbitmap_create_with_resource(RESOURCE_ID_RES_3);
	B4 = gbitmap_create_with_resource(RESOURCE_ID_RES_4); B5 = gbitmap_create_with_resource(RESOURCE_ID_RES_5);
	B6 = gbitmap_create_with_resource(RESOURCE_ID_RES_6); B7 = gbitmap_create_with_resource(RESOURCE_ID_RES_7);
	B8 = gbitmap_create_with_resource(RESOURCE_ID_RES_8); B9 = gbitmap_create_with_resource(RESOURCE_ID_RES_9);
	
	#ifdef PBL_ROUND
		bool round = true;
	#else
		bool round = false;
	#endif
	
	if(!(center || round)){
		hour1 = bitmap_layer_create(GRect(PADING_X, PADING_Y, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(hour1, B0);
		layer_add_child(window_layer, bitmap_layer_get_layer(hour1));
		
		hour2 = bitmap_layer_create(GRect(PADING_X*2 + IMAGE_W, PADING_Y, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(hour2, B1);
		layer_add_child(window_layer, bitmap_layer_get_layer(hour2));
		
		min1 = bitmap_layer_create(GRect(PADING_X, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(min1, B2);
		layer_add_child(window_layer, bitmap_layer_get_layer(min1));	
		
		min2 = bitmap_layer_create(GRect(PADING_X*2 + IMAGE_W, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(min2, B3);
		layer_add_child(window_layer, bitmap_layer_get_layer(min2));
	}else{
		hour1 = bitmap_layer_create(GRect(PADING_CENTER_X, PADING_Y, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(hour1, B0);
		layer_add_child(window_layer, bitmap_layer_get_layer(hour1));
	
		hour2 = bitmap_layer_create(GRect(bounds.size.w - PADING_CENTER_X - IMAGE_W, PADING_Y, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(hour2, B1);
		layer_add_child(window_layer, bitmap_layer_get_layer(hour2));
	
		min1 = bitmap_layer_create(GRect(PADING_CENTER_X, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(min1, B2);
		layer_add_child(window_layer, bitmap_layer_get_layer(min1));
	
		min2 = bitmap_layer_create(GRect(bounds.size.w - PADING_CENTER_X - IMAGE_W, bounds.size.h - PADING_Y - IMAGE_H, IMAGE_W, IMAGE_H));
		bitmap_layer_set_bitmap(min2, B3);
		layer_add_child(window_layer, bitmap_layer_get_layer(min2));
	}
	
	set_time();
}

void main_window_unload(){
	bitmap_layer_destroy(hour1); bitmap_layer_destroy(hour2);
	bitmap_layer_destroy(min1); bitmap_layer_destroy(min2);
	
	gbitmap_destroy(B0); gbitmap_destroy(B1); gbitmap_destroy(B2); gbitmap_destroy(B3); gbitmap_destroy(B4);
	gbitmap_destroy(B5); gbitmap_destroy(B6); gbitmap_destroy(B7); gbitmap_destroy(B8); gbitmap_destroy(B9);
}

void init(){
	mins = 0;
	if(persist_exists(KEY_MINUTES)){
		mins = persist_read_int(KEY_MINUTES);
	}
	
	center = false;
	if(persist_exists(CENTER)){
		center = persist_read_bool(CENTER);
	}
	
	window = window_create();
	
	window_set_window_handlers(window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	window_stack_push(window, true);
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
	
	connection_service_subscribe((ConnectionHandlers) {
  	.pebble_app_connection_handler = app_connection_handler,
  	.pebblekit_connection_handler = kit_connection_handler
	});
}

void deinit(){
	window_destroy(window);
}

int main(){
	init();
	app_event_loop();
	deinit();
}