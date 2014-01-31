#include <pebble.h>

#define NUMBER_OF_STATUS_BARS 10

#define STATUS_BAR_WIDTH 8
#define STATUS_BAR_HEIGHT 66

#define STATUS_BAR_X_ORIGIN 23
#define STATUS_BAR_X_OFFSET 10
#define STATUS_BAR_Y_ORIGIN 14

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

#define STATUS_RECHARGE 100
#define STATUS_CLOAK 62
#define STATUS_MAX STATUS_RECHARGE * STATUS_CLOAK

static Window *window;

static GBitmap *bg_image;
static GBitmap *empty_image;
static GBitmap *full_image;

static BitmapLayer *bg_layer;
static BitmapLayer *status_layer[NUMBER_OF_STATUS_BARS];

static AppTimer *timer;

static AccelData old_accel;
static int level = STATUS_MAX;

static int16_t abs_int16t(int16_t input)
{
    return input < 0 ? -input : input;
}

static int16_t deadzone_int16t(int16_t input, int16_t deadzone)
{
    int16_t output = input - deadzone;
    return output < 0 ? 0 : output;
}

static int clamp(int input, int min, int max)
{
    int output = input;
    if (input < min)
    {
        output = min;
    }
    else if (input > max)
    {
        output = max;
    }
    return output;
}

// Advance the display by one tick
static void tick()
{
    AccelData accel;
    accel_service_peek(&accel);
    int16_t movement = 0;
    movement += abs_int16t(accel.x - old_accel.x);
    movement += abs_int16t(accel.y - old_accel.y);
    movement += abs_int16t(accel.z - old_accel.z);
    movement = deadzone_int16t(movement, 150);
    old_accel = accel;
    
    if (movement > 0)
    {
        level = level - STATUS_RECHARGE;
    }
    else
    {
        level = level + STATUS_CLOAK;
    }
    level = clamp(level, 0, STATUS_MAX);
    
    app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, "Accel: %d, Level: %d, Status: %d", movement, level, level / (STATUS_MAX / 10));
}

// Set an individual status bar
static void set_bar(int i, bool status)
{
    if (status)
    {
        bitmap_layer_set_bitmap(status_layer[i], full_image);
    }
    else
    {
        bitmap_layer_set_bitmap(status_layer[i], empty_image);
    }
}

// Set status bars to a specified level
static void set_bars(int level)
{
    for (int i = 0; i < NUMBER_OF_STATUS_BARS; i++)
    {
        set_bar(i, i < level);
    }  
}

// Update GUI
static void update_gui()
{
    set_bars(level / (STATUS_MAX / 10));
}

// Called once per second
static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
    tick();
    update_gui();
}

static void handle_accel(AccelData *accel_data, uint32_t num_samples)
{
  // do nothing
}


static void handle_timer(void *data)
{
    tick();
    update_gui();
    timer = app_timer_register(100 /* milliseconds */, handle_timer, NULL);
}

// Handle the start-up of the app
static void init(void)
{
    // Create window
    window = window_create();
    window_stack_push(window, true);
    
    // Load images
    bg_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BG);
    empty_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EMPTY);
    full_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FULL);
    
    // Create bg layer
    bg_layer = bitmap_layer_create(GRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
    bitmap_layer_set_bitmap(bg_layer, bg_image);
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(bg_layer));
    
    // Create status layers
    for (int i = 0; i < NUMBER_OF_STATUS_BARS; i++)
    {
        status_layer[i] = bitmap_layer_create(GRect(STATUS_BAR_X_ORIGIN + STATUS_BAR_X_OFFSET * i, STATUS_BAR_Y_ORIGIN, STATUS_BAR_WIDTH, STATUS_BAR_HEIGHT));
        bitmap_layer_set_bitmap(status_layer[i], empty_image);
        layer_add_child(bitmap_layer_get_layer(bg_layer), bitmap_layer_get_layer(status_layer[i]));
    }
    
    // Subscribe minute handler
    //tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
    
    // Setup accelerator handling
    old_accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
    accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
    accel_service_set_samples_per_update(0);
    accel_data_service_subscribe(0, handle_accel);
    
    timer = app_timer_register(100 /* milliseconds */, handle_timer, NULL);
}

// Handle the shutdown of the app
static void deinit(void)
{
    // Disable accelerator handling
    accel_data_service_unsubscribe();
    
    // Destroy status layers
    for (int i = 0; i < NUMBER_OF_STATUS_BARS; i++)
    {
        bitmap_layer_destroy(status_layer[i]);
    }
    
    // Destroy bg layer
    bitmap_layer_destroy(bg_layer);
    
    // Unload images
    gbitmap_destroy(bg_image);
    gbitmap_destroy(empty_image);
    gbitmap_destroy(full_image);
    
    // Destroy window
    window_destroy(window);
}

// The main event/run loop for our app
int main(void)
{
    init();
    app_event_loop();
    deinit();
}
