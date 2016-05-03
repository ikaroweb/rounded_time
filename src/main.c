#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

GColor color_loser;
GColor color_winner;

static Window *s_main_window; // declare the main window
static TextLayer *s_time_layer; //text layer
static TextLayer *s_date_layer; //text layer
static TextLayer *s_day_name_layer;
static GFont s_time_font,s_date_font; // use custom font file
static TextLayer *s_weather_layer; //layer for weather

static GFont s_weather_font,s_step_font,s_weather_font,s_day_font; //weather font

static int s_battery_level;

//steps
static TextLayer *s_step_layer; //layer for steps
static int s_step_count = 0,s_step_average = 0;//count steps
static char s_current_steps_buffer[16];
	
//static Layer *s_battery_layer_2;

//Weather
static GDrawCommandImage *s_command_image,*s_step_image;
static Layer *s_canvas_layer,*s_shoes_layer; //layer image


//use a bitmap
//static BitmapLayer *s_background_layer;
//static GBitmap *s_background_bitmap;

static Layer *BatteryCircle;


// Is step data available?
bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

// Todays current step count
static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}


static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
  static char s_emoji[5];

  if(s_step_count >= s_step_average) {
    text_layer_set_text_color(s_step_layer, color_winner);
    snprintf(s_emoji, sizeof(s_emoji), "\U0001F60C ");
  } else {
    text_layer_set_text_color(s_step_layer, color_loser);
    snprintf(s_emoji, sizeof(s_emoji), "\U0001F4A9 ");
  }

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d,%03d", thousands, hundreds);
		// snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
	//      "%d,%03d %s", thousands, hundreds, s_emoji);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d", hundreds);
  }

  text_layer_set_text(s_step_layer, s_current_steps_buffer);
	
	
}

static void health_handler(HealthEventType event, void *context) {
  /*if(event == HealthEventSignificantUpdate) {
    get_step_goal();
  }*/

  if(event != HealthEventSleepUpdate) {
    get_step_count();
  //  get_step_average();
    display_step_count();
//    layer_mark_dirty(s_progress_layer);
  //  layer_mark_dirty(s_average_layer);
  }
}

static void main_window_unload(Window *window) {
	 // Destroy TextLayer
  text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	//	text_layer_destroy(s_day_name_layer);
	// Unload GFont
fonts_unload_custom_font(s_time_font);
	
	// Destroy GBitmap
//gbitmap_destroy(s_background_bitmap);

// Destroy BitmapLayer
//bitmap_layer_destroy(s_background_layer);
	
	//Destroy Weather Layer and Font
	text_layer_destroy(s_weather_layer);
fonts_unload_custom_font(s_weather_font);
	fonts_unload_custom_font(s_step_font);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

	static char s_buffer_date[12];
	strftime(s_buffer_date,sizeof(s_buffer_date),"%d-%B", tick_time);
	
	static char s_buffer_day[12];
	strftime(s_buffer_day,sizeof(s_buffer_day),"%A", tick_time);
	
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
	
	text_layer_set_text(s_date_layer, s_buffer_date);
	
	text_layer_set_text(s_day_name_layer, s_buffer_day);
	
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
	// Get weather update every 30 minutes
if(tick_time->tm_min % 30 == 0) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}
}

//BATTERY UPDATE LEVEL
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
}
//BATTERY

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
		
		

		// Store incoming information weather
	static char temperature_buffer[8];
	static char conditions_buffer[32];
	static char weather_layer_buffer[32];

	// Read tuples for data
	Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
	Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

	// If all data is available, use it
	if(temp_tuple && conditions_tuple) {
		snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)temp_tuple->value->int32);
		snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
	}

	// Assemble full string and display
	//snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s,%s", temperature_buffer, conditions_buffer);
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
	text_layer_set_text(s_weather_layer, weather_layer_buffer);
	
	APP_LOG(APP_LOG_LEVEL_INFO,"Conditions: %s",conditions_buffer);
	 //weather
	 // s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_,"%s",conditions_buffer); //set image weather
	/*switch ("%s",conditions_buffer) {
    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image =  gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image =  gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image =  gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image =  gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    case WEATHER_APP_ICON_HEAVY_RAIN :
      s_command_image =  gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);

    default:
       s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_LIGHT_RAIN);
  }
	*/
	if (strcmp(conditions_buffer, "Rain")==0) {
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_RAIN);
	}
	else if (strcmp(conditions_buffer, "Clouds")==0) {
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_CLOUD);
	}
	else if (strcmp(conditions_buffer, "Clear")==0) {
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_SUN);
	}
	else if (strcmp(conditions_buffer, "Sun")==0) {
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_SUN);
	}
	else if (strcmp(conditions_buffer, "Sunset")==0) {
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_SUNSET);
	}
	else{
		s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ICON_GENERIC);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static int32_t get_angle_for_minute(int hour) {
  // Progress through 60 miunutes, out of 360 degrees
  return (hour * 360) / 100;
}
/*static int32_t get_angle_for_hour(int hour) {
  // Progress through 12 hours, out of 360 degrees
  return (hour * 360) / 12;
}*/

static void battery_update_proc(Layer *layer, GContext *ctx) {
	/*GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 114.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
	
	// */

	int minute_angle = get_angle_for_minute(s_battery_level);
	GRect bounds = layer_get_bounds(layer);
 GRect frame = grect_inset(bounds, GEdgeInsets(2));
	#ifdef PBL_COLOR
		graphics_context_set_fill_color(ctx, GColorBlack);
	#endif
	graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 5, 0, DEG_TO_TRIGANGLE(minute_angle));


}

static void update_proc(Layer *layer, GContext *ctx) {
  // Set the origin offset from the context for drawing the image
  GPoint origin = GPoint(10, 20);

  // Draw the GDrawCommandImage to the GContext
  gdraw_command_image_draw(ctx, s_command_image, origin);
	
	//gdraw_command_image_draw(ctx, s_step_image, origin);
	
}

static void update_proc_shoes(Layer *layer, GContext *ctx) {
  // Set the origin offset from the context for drawing the image
  GPoint origin = GPoint(10, 20);

  // Draw the GDrawCommandImage to the GContext
  gdraw_command_image_draw(ctx, s_step_image, origin);
	
	//gdraw_command_image_draw(ctx, s_step_image, origin);
	
}

/*static void face_update_proc(Layer *layer, GContext *ctx) {
// graphics_context_set_fill_color(ctx, GColorWhite);
// graphics_fill_circle(ctx, calculate_center_point(0,0), 70);
 //graphics_context_set_stroke_color(ctx, GColorRed);//set color circle
	//graphics_draw_circle(ctx, calculate_center_point(0,0), 70);
	int minute_angle = get_angle_for_minute(s_battery_level);
	GRect bounds = layer_get_bounds(layer);
 GRect frame = grect_inset(bounds, GEdgeInsets(10));
	graphics_context_set_fill_color(ctx, GColorCeleste);
	graphics_fill_radial(ctx, frame, GOvalScaleModeFitCircle, 3, 0, DEG_TO_TRIGANGLE(minute_angle));
	//APP_LOG(APP_LOG_LEVEL_INFO,"Battery: %i",minute_angle);
}*/


//main window section
static void main_window_load(Window *window) {
	// Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
	
	BatteryCircle = layer_create(bounds);
	layer_set_update_proc(BatteryCircle, battery_update_proc);
	layer_add_child(window_layer, BatteryCircle);
	
	//  TIME LAYER //
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(62, 56), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
 // text_layer_set_text(s_time_layer, "SUKA");
 // text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	
	// Create GFont
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_48));

	// Apply to TextLayer
	text_layer_set_font(s_time_layer, s_time_font);
	
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	
	/*
	*/
	// DATE LAYER
	
	// Create the DateLayer with specific bounds
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(120, 114), bounds.size.w, 40));
	
	// Apply to TextLayer
	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_14));
	text_layer_set_font(s_date_layer, s_date_font);
	//text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	
	text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
	
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	
	//text_layer_set_text(s_date_layer, "SUKA");
	
	
	//DAY NAME LAYER
	s_day_name_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(140, 136), bounds.size.w, 40));
	
	text_layer_set_background_color(s_day_name_layer, GColorClear);
  text_layer_set_text_color(s_day_name_layer, GColorBlack);
//	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	s_day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_14));
  text_layer_set_text_alignment(s_day_name_layer, GTextAlignmentCenter);
	text_layer_set_font(s_day_name_layer, s_day_font);
	//text_layer_set_text(s_day_name_layer, "DOMENICA");
	
	//WEATHER LAYER
	// Create temperature Layer
s_weather_layer = text_layer_create(
    GRect(25, 45, bounds.size.w/2, bounds.size.h)); //PBL_IF_ROUND_ELSE(30, 25)

// Style the text
text_layer_set_background_color(s_weather_layer, GColorClear);
text_layer_set_text_color(s_weather_layer, GColorBlack);
text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
//text_layer_set_text(s_weather_layer, "Loading...");
	s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_20));
text_layer_set_font(s_weather_layer, s_weather_font);	
	
	//create bitmap layer
	// Create GBitmap
//s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

// Create BitmapLayer to display the GBitmap
//s_background_layer = bitmap_layer_create(bounds);

// Set the bitmap onto the layer and add to the window
//bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
//layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	
	//Add date layer
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
	
	//Add day name layer
	layer_add_child(window_layer, text_layer_get_layer(s_day_name_layer));
	
	//Add weather layer
	// Create second custom font, apply it and add to Window
	
//s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
//text_layer_set_font(s_weather_layer, s_weather_font);
	
	s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_48));
	
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
	
	//BATTERY
	// Create battery meter Layer
//s_battery_layer_2 = layer_create(GRect(14, 54, 115, 2));
//layer_set_update_proc(s_battery_layer_2, battery_update_proc);

// Add to Window
//layer_add_child(window_get_root_layer(window), s_battery_layer_2);
	
	
	
	//layer_mark_dirty(s_battery_layer_2); //update the layer with battery status
	
	layer_add_child(window_layer, BatteryCircle); //Battery Circle Status
	
	
	//APP_LOG(APP_LOG_LEVEL_INFO, "With: %i - Height: %i", bounds.size.w,bounds.size.h);
	
	//WEATHER LAYER
// Create the canvas Layer
  s_canvas_layer = layer_create(GRect(90, 25, bounds.size.w, bounds.size.h));

  // Set the LayerUpdateProc
  layer_set_update_proc(s_canvas_layer, update_proc);

  // Add to parent Window
  layer_add_child(window_layer, s_canvas_layer);
	
	
	
	 // Create a layer to hold the current step count
  s_step_layer = text_layer_create(
      GRect(25, PBL_IF_ROUND_ELSE(16, 13), bounds.size.w/2, 38));
  text_layer_set_background_color(s_step_layer, GColorClear);
	
		s_step_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Round_Regular_20));
		text_layer_set_font(s_step_layer, s_step_font);	
//  text_layer_set_font(s_step_layer,
  //                    fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_step_layer));
	
	  if(step_data_is_available()) {
    health_service_events_subscribe(health_handler, NULL);
  }
	
	
	//SHOES LAYER
	
	s_shoes_layer = layer_create(GRect(90, 0, bounds.size.w, bounds.size.h));

  // Set the LayerUpdateProc
  layer_set_update_proc(s_shoes_layer, update_proc_shoes);

  // Add to parent Window
  layer_add_child(window_layer, s_shoes_layer);
	
	//step canvas
	s_step_image = gdraw_command_image_create_with_resource(RESOURCE_ID_IMAGE_SHOES);
	
}

static void init() {
	
	setlocale(LC_TIME, "");
	
	// Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	window_set_background_color(s_main_window, GColorWhite);

	
  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
	
		
	
	// Make sure the time is displayed from the start
	update_time();
	
// Register callbacks
app_message_register_inbox_received(inbox_received_callback);
app_message_register_inbox_dropped(inbox_dropped_callback);
app_message_register_outbox_failed(outbox_failed_callback);
app_message_register_outbox_sent(outbox_sent_callback);
	
	// Open AppMessage
			const int inbox_size = 128;
			const int outbox_size = 128;
			app_message_open(inbox_size, outbox_size);
	
	//app_message_open(512, 64); 

	battery_state_service_subscribe(battery_callback);
	battery_callback(battery_state_service_peek());
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

	
	color_loser = GColorBlack;
  color_winner = GColorBlack;

}



static void deinit() {
// Destroy Window
  window_destroy(s_main_window);
	//layer_destroy(s_battery_layer_2);
	layer_destroy(s_canvas_layer);
//	layer_destroy(s_shoes_layer);
  gdraw_command_image_destroy(s_command_image);
	gdraw_command_image_destroy(s_step_image);
	layer_destroy(text_layer_get_layer(s_step_layer));
	
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

