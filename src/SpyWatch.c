#include <pebble.h>

#define NUMBER_OF_STATUS_BARS 10

#define STATUS_BAR_WIDTH 8
#define STATUS_BAR_HEIGHT 66

#define STATUS_BAR_X_ORIGIN 23
#define STATUS_BAR_X_OFFSET 10
#define STATUS_BAR_Y_ORIGIN 14

#define SCREEN_WIDTH 144
#define SCREEN_HEIGHT 168

static Window *window;
static GBitmap *bg_image;
static GBitmap *empty_image;
static GBitmap *full_image;
static BitmapLayer *bg_layer;
static BitmapLayer *status_layer[NUMBER_OF_STATUS_BARS];
static int level = 0;
static int diff = 1;

static void tick()
{
    level += diff;
    if (level == 0 || level == 10)
    {
        diff = -diff;
    }
    
}

static void update_gui()
{
    // Set status bars
    for (int i = 0; i < NUMBER_OF_STATUS_BARS; i++)
    {
        if (i < level)
        {
            bitmap_layer_set_bitmap(status_layer[i], full_image);
        }
        else
        {
            bitmap_layer_set_bitmap(status_layer[i], empty_image);
        }
    }      
    
}

// Called once per second
static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
    tick();
    update_gui();
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
    tick_timer_service_subscribe(SECOND_UNIT, &handle_tick);
}

// Handle the shutdown of the app
static void deinit(void)
{
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
