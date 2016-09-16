#include <pebble.h>

static Window *s_main_window;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;
static TextLayer *s_battery_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_top_layer;
static TextLayer *s_date_btm_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_forecast_layer;
static GFont s_battery_font;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_weather_font;
static GFont s_forecast_font;
static char s_battery_buffer[16];

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
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BT_DISCONN);
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
  // bluetooth
  s_bt_icon_layer = bitmap_layer_create(GRect(10, 12, 18, 18));
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));

  // Create the TextLayer with specific bounds
  s_battery_layer = text_layer_create(GRect(10, 10, bounds.size.w - 20, 18));
  s_time_layer = text_layer_create(GRect(0, 26, bounds.size.w, 46));
  s_date_top_layer = text_layer_create(GRect(18, 72, bounds.size.w - 36, 18));
  s_date_btm_layer = text_layer_create(GRect(10, 86, bounds.size.w - 20, 18));
  s_weather_layer = text_layer_create(GRect(0, 106, bounds.size.w, 24));
  s_forecast_layer = text_layer_create(GRect(0, 130, bounds.size.w, 30));
  // background color
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_background_color(s_date_top_layer, GColorClear);
  text_layer_set_background_color(s_date_btm_layer, GColorClear);
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_background_color(s_forecast_layer, GColorClear);
  // forecolor
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorOrange);
  text_layer_set_text_color(s_date_top_layer, GColorWhite);
  text_layer_set_text_color(s_date_btm_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_forecast_layer, GColorWhite);
  // font
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_12));
  text_layer_set_font(s_battery_layer, s_battery_font);
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_36));
  text_layer_set_font(s_time_layer, s_time_font);
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_14));
  text_layer_set_font(s_date_top_layer, s_date_font);
  text_layer_set_font(s_date_btm_layer, s_date_font);
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_18));
  text_layer_set_font(s_weather_layer, s_weather_font);
  s_forecast_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_APPLE_GARAMOND_BOLD_12));
  text_layer_set_font(s_forecast_layer, s_forecast_font);
  // alignment
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_date_top_layer, GTextAlignmentLeft);
  text_layer_set_text_alignment(s_date_btm_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_forecast_layer, GTextAlignmentCenter);

  // Battery
  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);
  text_layer_set_text(s_battery_layer, s_battery_buffer);
  // Time
  text_layer_set_text(s_time_layer, "00:00");
  // Date
  text_layer_set_text(s_date_top_layer, "Sunday");
  text_layer_set_text(s_date_btm_layer, "26 October");
  // Weather
  text_layer_set_text(s_weather_layer, "Loading...");
  // Forecast
  text_layer_set_text(s_forecast_layer, "Loading...");

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_top_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_btm_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_forecast_layer));

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
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_forecast_layer);
  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  gbitmap_destroy(s_bt_icon_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  bitmap_layer_destroy(s_bt_icon_layer);
  // Unload GFont
  fonts_unload_custom_font(s_battery_font);
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_date_font);
  fonts_unload_custom_font(s_weather_font);
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

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°c", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s  %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
  text_layer_set_text(s_forecast_layer, "");
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

static void init() {
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
