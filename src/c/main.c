#include <pebble.h>
#include "main.h"

static Window *s_main_window;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static BitmapLayer *s_weather_icon_layer;
static BitmapLayer *s_forecast_icon_layer_1, *s_forecast_icon_layer_2, *s_forecast_icon_layer_3, *s_forecast_icon_layer_4;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static GBitmap *s_weather_icon_bitmap;
static GBitmap *s_forecast_icon_bitmap_1, *s_forecast_icon_bitmap_2, *s_forecast_icon_bitmap_3, *s_forecast_icon_bitmap_4;
static TextLayer *s_battery_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_top_layer, *s_date_btm_layer;
static TextLayer *s_temperature_layer, *s_local_layer;
static TextLayer *s_forecast_layer_1, *s_forecast_layer_2, *s_forecast_layer_3, *s_forecast_layer_4;
static GFont s_battery_font;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_temperature_font, s_local_font, s_forecast_font;
static char s_battery_buffer[16];

// A struct for our specific settings (see main.h)
ClaySettings settings;

// Initialize the default settings
static void prv_default_settings() {
  snprintf(settings.Temperature_scale, sizeof(settings.Temperature_scale), "%s", "fahrenheit");
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void battery_handler(BatteryChargeState charge_state) {
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  if(charge_state.charge_percent <= 20) {
    text_layer_set_text_color(s_battery_layer, GColorRed);
  } else if(charge_state.charge_percent <= 50) {
    text_layer_set_text_color(s_battery_layer, GColorChromeYellow);
  } else {
    text_layer_set_text_color(s_battery_layer, GColorWhite);    
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  static char s_date_top_buffer[16];
  strftime(s_date_top_buffer, sizeof(s_date_top_buffer), "%A", tick_time);
  static char s_date_btm_buffer[16];
  strftime(s_date_btm_buffer, sizeof(s_date_btm_buffer), "%e %B", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_text(s_date_top_layer, s_date_top_buffer);
  text_layer_set_text(s_date_btm_layer, s_date_btm_buffer);
  //Date Color
  if(strcmp(s_date_top_buffer, "Sunday") == 0) {
    text_layer_set_text_color(s_date_top_layer, GColorRed);
  } else if(strcmp(s_date_top_buffer, "Saturday") == 0) {
    text_layer_set_text_color(s_date_top_layer, GColorBlue);
  } else {
    text_layer_set_text_color(s_date_top_layer, GColorWhite);
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_DISCONN);
  s_weather_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_W_99);
  s_forecast_icon_bitmap_1 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_W_99_M);
  s_forecast_icon_bitmap_2 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_W_99_M);
  s_forecast_icon_bitmap_3 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_W_99_M);
  s_forecast_icon_bitmap_4 = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_W_99_M);
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  // bluetooth
  s_bt_icon_layer = bitmap_layer_create(GRect(10, 12, 18, 18));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  // weather
  s_weather_icon_layer = bitmap_layer_create(GRect(10, 98, 50, 30));
  bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_weather_icon_layer));
  // forecast
  s_forecast_icon_layer_1 = bitmap_layer_create(GRect(11, 134, 30, 18));
  bitmap_layer_set_bitmap(s_forecast_icon_layer_1, s_forecast_icon_bitmap_1);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_forecast_icon_layer_1));
  s_forecast_icon_layer_2 = bitmap_layer_create(GRect(42, 134, 30, 18));
  bitmap_layer_set_bitmap(s_forecast_icon_layer_2, s_forecast_icon_bitmap_2);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_forecast_icon_layer_2));
  s_forecast_icon_layer_3 = bitmap_layer_create(GRect(73, 134, 30, 18));
  bitmap_layer_set_bitmap(s_forecast_icon_layer_3, s_forecast_icon_bitmap_3);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_forecast_icon_layer_3));
  s_forecast_icon_layer_4 = bitmap_layer_create(GRect(104, 134, 30, 18));
  bitmap_layer_set_bitmap(s_forecast_icon_layer_4, s_forecast_icon_bitmap_4);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_forecast_icon_layer_4));

  // Create the TextLayer with specific bounds
  s_battery_layer = text_layer_create(GRect(10, 5, bounds.size.w - 20, 18));
  s_time_layer = text_layer_create(GRect(0, 12, bounds.size.w, 46));
  s_date_top_layer = text_layer_create(GRect(16, 55, bounds.size.w - 32, 20));
  s_date_btm_layer = text_layer_create(GRect(16, 72, bounds.size.w - 32, 20));
  s_temperature_layer = text_layer_create(GRect(50, 92, bounds.size.w - 60, 24));
  s_local_layer = text_layer_create(GRect(50, 112, bounds.size.w - 60, 16));
  s_forecast_layer_1 = text_layer_create(GRect(11, 144, 30, 12));
  s_forecast_layer_2 = text_layer_create(GRect(42, 144, 30, 12));
  s_forecast_layer_3 = text_layer_create(GRect(73, 144, 30, 12));
  s_forecast_layer_4 = text_layer_create(GRect(104, 144, 30, 12));
  // background color
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_date_top_layer, GColorClear);
  text_layer_set_background_color(s_date_btm_layer, GColorClear);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_background_color(s_local_layer, GColorClear);
  text_layer_set_background_color(s_forecast_layer_1, GColorClear);
  text_layer_set_background_color(s_forecast_layer_2, GColorClear);
  text_layer_set_background_color(s_forecast_layer_3, GColorClear);
  text_layer_set_background_color(s_forecast_layer_4, GColorClear);
  // forecolor
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorOrange);
  text_layer_set_text_color(s_date_top_layer, GColorWhite);
  text_layer_set_text_color(s_date_btm_layer, GColorYellow);
  text_layer_set_text_color(s_temperature_layer, GColorWhite);
  text_layer_set_text_color(s_local_layer, GColorWhite);
  text_layer_set_text_color(s_forecast_layer_1, GColorWhite);
  text_layer_set_text_color(s_forecast_layer_2, GColorWhite);
  text_layer_set_text_color(s_forecast_layer_3, GColorWhite);
  text_layer_set_text_color(s_forecast_layer_4, GColorWhite);
  // font
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_14));
  text_layer_set_font(s_battery_layer, s_battery_font);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_40));
  text_layer_set_font(s_time_layer, s_time_font);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_16));
  text_layer_set_font(s_date_top_layer, s_date_font);
  text_layer_set_font(s_date_btm_layer, s_date_font);
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_20));
  text_layer_set_font(s_temperature_layer, s_temperature_font);
  s_local_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_14));
  text_layer_set_font(s_local_layer, s_local_font);
  s_forecast_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_12));
  text_layer_set_font(s_forecast_layer_1, s_forecast_font);
  text_layer_set_font(s_forecast_layer_2, s_forecast_font);
  text_layer_set_font(s_forecast_layer_3, s_forecast_font);
  text_layer_set_font(s_forecast_layer_4, s_forecast_font);
  // alignment
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_top_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_btm_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_local_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_forecast_layer_1, GTextAlignmentRight);
  text_layer_set_text_alignment(s_forecast_layer_2, GTextAlignmentRight);
  text_layer_set_text_alignment(s_forecast_layer_3, GTextAlignmentRight);
  text_layer_set_text_alignment(s_forecast_layer_4, GTextAlignmentRight);

  // Battery
  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  if(charge_state.charge_percent <= 20) {
    text_layer_set_text_color(s_battery_layer, GColorRed);
  } else if(charge_state.charge_percent <= 50) {
    text_layer_set_text_color(s_battery_layer, GColorChromeYellow);
  } else {
    text_layer_set_text_color(s_battery_layer, GColorWhite);    
  }
  // Time
  text_layer_set_text(s_time_layer, "00:00");
  // Date
  text_layer_set_text(s_date_top_layer, "Sunday");
  text_layer_set_text(s_date_btm_layer, "26 October");
  // Weather
  text_layer_set_text(s_temperature_layer, "Loading...");
  text_layer_set_text(s_local_layer, "Loading...");
  // Forecast
  text_layer_set_text(s_forecast_layer_1, "");
  text_layer_set_text(s_forecast_layer_2, "");
  text_layer_set_text(s_forecast_layer_3, "");
  text_layer_set_text(s_forecast_layer_4, "");

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_top_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_btm_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temperature_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_local_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer_1));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer_2));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer_3));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer_4));

  // Show the correct state of the BT connection from the start
  bluetooth_callback(connection_service_peek_pebble_app_connection());

  battery_state_service_subscribe(battery_handler);
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_top_layer);
  text_layer_destroy(s_date_btm_layer);
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(s_local_layer);
  text_layer_destroy(s_forecast_layer_1);
  text_layer_destroy(s_forecast_layer_2);
  text_layer_destroy(s_forecast_layer_3);
  text_layer_destroy(s_forecast_layer_4);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);
  gbitmap_destroy(s_weather_icon_bitmap);
  gbitmap_destroy(s_forecast_icon_bitmap_1);
  gbitmap_destroy(s_forecast_icon_bitmap_2);
  gbitmap_destroy(s_forecast_icon_bitmap_3);
  gbitmap_destroy(s_forecast_icon_bitmap_4);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  bitmap_layer_destroy(s_weather_icon_layer);
  bitmap_layer_destroy(s_forecast_icon_layer_1);
  bitmap_layer_destroy(s_forecast_icon_layer_2);
  bitmap_layer_destroy(s_forecast_icon_layer_3);
  bitmap_layer_destroy(s_forecast_icon_layer_4);
  // Unload GFont
  fonts_unload_custom_font(s_battery_font);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_temperature_font);
  fonts_unload_custom_font(s_local_font);
  fonts_unload_custom_font(s_forecast_font);
  // Battery State Service
  battery_state_service_unsubscribe();
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

uint32_t get_weather_resource_id_by_iconname(char iconname_buffer[8]) {
  uint32_t resource_id = RESOURCE_ID_IMAGE_W_99;
  if(strcmp(iconname_buffer, "01d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_01D;
  } else if(strcmp(iconname_buffer, "01n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_01N;
  } else if(strcmp(iconname_buffer, "02d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_02D;
  } else if(strcmp(iconname_buffer, "02n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_02N;
  } else if(strcmp(iconname_buffer, "03d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_03D;
  } else if(strcmp(iconname_buffer, "03n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_03N;
  } else if(strcmp(iconname_buffer, "04d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_04D;
  } else if(strcmp(iconname_buffer, "04n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_04N;
  } else if(strcmp(iconname_buffer, "09d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_09D;
  } else if(strcmp(iconname_buffer, "09n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_09N;
  } else if(strcmp(iconname_buffer, "10d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_10D;
  } else if(strcmp(iconname_buffer, "10n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_10N;
  } else if(strcmp(iconname_buffer, "11d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_11D;
  } else if(strcmp(iconname_buffer, "11n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_11N;
  } else if(strcmp(iconname_buffer, "13d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_13D;
  } else if(strcmp(iconname_buffer, "13n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_13N;
  } else if(strcmp(iconname_buffer, "50d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_50D;
  } else if(strcmp(iconname_buffer, "50n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_50N;
  }
  return resource_id;
}

uint32_t get_forecast_resource_id_by_iconname(char iconname_buffer[8]) {
  uint32_t resource_id = RESOURCE_ID_IMAGE_W_99_M;
  if(strcmp(iconname_buffer, "01d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_01D_M;
  } else if(strcmp(iconname_buffer, "01n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_01N_M;
  } else if(strcmp(iconname_buffer, "02d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_02D_M;
  } else if(strcmp(iconname_buffer, "02n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_02N_M;
  } else if(strcmp(iconname_buffer, "03d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_03D_M;
  } else if(strcmp(iconname_buffer, "03n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_03N_M;
  } else if(strcmp(iconname_buffer, "04d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_04D_M;
  } else if(strcmp(iconname_buffer, "04n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_04N_M;
  } else if(strcmp(iconname_buffer, "09d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_09D_M;
  } else if(strcmp(iconname_buffer, "09n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_09N_M;
  } else if(strcmp(iconname_buffer, "10d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_10D_M;
  } else if(strcmp(iconname_buffer, "10n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_10N_M;
  } else if(strcmp(iconname_buffer, "11d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_11D_M;
  } else if(strcmp(iconname_buffer, "11n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_11N_M;
  } else if(strcmp(iconname_buffer, "13d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_13D_M;
  } else if(strcmp(iconname_buffer, "13n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_13N_M;
  } else if(strcmp(iconname_buffer, "50d") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_50D_M;
  } else if(strcmp(iconname_buffer, "50n") == 0) {
    resource_id = RESOURCE_ID_IMAGE_W_50N_M;
  }
  return resource_id;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Read config
  Tuple *temp_scale_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE_SCALE);
  if(temp_scale_tuple) {
    snprintf(settings.Temperature_scale, sizeof(settings.Temperature_scale), "%s", temp_scale_tuple->value->cstring);
    prv_save_settings();
  }

  // Store incoming information
  static char temperature_buffer[8];
  static char icon_buffer[8];
  static char local_buffer[32];
  
  static char forecasttime1_buffer[4];
  static char forecasttime2_buffer[4];
  static char forecasttime3_buffer[4];
  static char forecasttime4_buffer[4];
  static char forecasticon1_buffer[8];
  static char forecasticon2_buffer[8];
  static char forecasticon3_buffer[8];
  static char forecasticon4_buffer[8];

  // Read tuples for data
  Tuple *temp_tuple_f = dict_find(iterator, MESSAGE_KEY_TEMPERATURE_F);
  Tuple *temp_tuple_c = dict_find(iterator, MESSAGE_KEY_TEMPERATURE_C);
  Tuple *icon_tuple = dict_find(iterator, MESSAGE_KEY_ICONNAME);
  Tuple *local_tuple = dict_find(iterator, MESSAGE_KEY_LOCALNAME);

  Tuple *fcsttime1_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTTIME1);
  Tuple *fcsttime2_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTTIME2);
  Tuple *fcsttime3_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTTIME3);
  Tuple *fcsttime4_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTTIME4);
  Tuple *fcsticon1_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTICONS1);
  Tuple *fcsticon2_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTICONS2);
  Tuple *fcsticon3_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTICONS3);
  Tuple *fcsticon4_tuple = dict_find(iterator, MESSAGE_KEY_FORECASTICONS4);

  // If all data is available, use it
  if(temp_tuple_f && icon_tuple) {
    if (strcmp(settings.Temperature_scale, "celsius") == 0){
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°c", (int)temp_tuple_c->value->int32);
    }else{
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°f", (int)temp_tuple_f->value->int32);
    }
    snprintf(icon_buffer, sizeof(icon_buffer), "%s", icon_tuple->value->cstring);
    snprintf(local_buffer, sizeof(local_buffer), "%s", local_tuple->value->cstring);
    // Assemble full string and display
    text_layer_set_text(s_temperature_layer, temperature_buffer);
    text_layer_set_text(s_local_layer, local_buffer);
    // Weather icon
    uint32_t weather_resource_id = get_weather_resource_id_by_iconname(icon_buffer);
    s_weather_icon_bitmap = gbitmap_create_with_resource(weather_resource_id);
    bitmap_layer_set_bitmap(s_weather_icon_layer, s_weather_icon_bitmap);
  }

  if(fcsttime1_tuple && fcsticon1_tuple) {
    snprintf(forecasttime1_buffer, sizeof(forecasttime1_buffer), "%s", fcsttime1_tuple->value->cstring);
    snprintf(forecasttime2_buffer, sizeof(forecasttime2_buffer), "%s", fcsttime2_tuple->value->cstring);
    snprintf(forecasttime3_buffer, sizeof(forecasttime3_buffer), "%s", fcsttime3_tuple->value->cstring);
    snprintf(forecasttime4_buffer, sizeof(forecasttime4_buffer), "%s", fcsttime4_tuple->value->cstring);
    snprintf(forecasticon1_buffer, sizeof(forecasticon1_buffer), "%s", fcsticon1_tuple->value->cstring);
    snprintf(forecasticon2_buffer, sizeof(forecasticon2_buffer), "%s", fcsticon2_tuple->value->cstring);
    snprintf(forecasticon3_buffer, sizeof(forecasticon3_buffer), "%s", fcsticon3_tuple->value->cstring);
    snprintf(forecasticon4_buffer, sizeof(forecasticon4_buffer), "%s", fcsticon4_tuple->value->cstring);

    text_layer_set_text(s_forecast_layer_1, forecasttime1_buffer);
    text_layer_set_text(s_forecast_layer_2, forecasttime2_buffer);
    text_layer_set_text(s_forecast_layer_3, forecasttime3_buffer);
    text_layer_set_text(s_forecast_layer_4, forecasttime4_buffer);

    uint32_t forecast_resource_id_1 = get_forecast_resource_id_by_iconname(forecasticon1_buffer);
    s_forecast_icon_bitmap_1 = gbitmap_create_with_resource(forecast_resource_id_1);
    bitmap_layer_set_bitmap(s_forecast_icon_layer_1, s_forecast_icon_bitmap_1);
    uint32_t forecast_resource_id_2 = get_forecast_resource_id_by_iconname(forecasticon2_buffer);
    s_forecast_icon_bitmap_2 = gbitmap_create_with_resource(forecast_resource_id_2);
    bitmap_layer_set_bitmap(s_forecast_icon_layer_2, s_forecast_icon_bitmap_2);
    uint32_t forecast_resource_id_3 = get_forecast_resource_id_by_iconname(forecasticon3_buffer);
    s_forecast_icon_bitmap_3 = gbitmap_create_with_resource(forecast_resource_id_3);
    bitmap_layer_set_bitmap(s_forecast_icon_layer_3, s_forecast_icon_bitmap_3);
    uint32_t forecast_resource_id_4 = get_forecast_resource_id_by_iconname(forecasticon4_buffer);
    s_forecast_icon_bitmap_4 = gbitmap_create_with_resource(forecast_resource_id_4);
    bitmap_layer_set_bitmap(s_forecast_icon_layer_4, s_forecast_icon_bitmap_4);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  text_layer_set_text(s_temperature_layer, "ERROR");
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  // Write the current hours and minutes into a buffer
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_local_layer, s_time_buffer);
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void init() {
  prv_load_settings();

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}