#include "marquee_layer.h"

static const int16_t FRAME_TIMEOUT = 1000/20;

static void marquee_layer_update(MarqueeLayer* marquee_layer, GContext* ctx);
static int16_t marquee_layer_get_overflow_size(MarqueeLayer* marquee_layer);
static void marquee_layer_timer_handler(void* context);
static void marquee_layer_schedule_tick(MarqueeLayer* marquee_layer, uint32_t timeout);

typedef struct {
	const char* text;
	AppTimer* timer;
	int32_t scrolledness;
	uint8_t pixels_per_frame;
	GFont font;
	GColor text_color;
	GColor background_color;
	uint32_t initial_delay;
	bool enable_backlight;
} MarqueeLayerData;

MarqueeLayer* marquee_layer_create(GRect frame) {
	MarqueeLayer* marquee_layer = layer_create_with_data(frame, sizeof(MarqueeLayerData));
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	marquee_layer_set_text(marquee_layer, "");
	data->font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
	marquee_layer_set_speed(marquee_layer, 150);
	data->initial_delay = 3000;
	data->enable_backlight = false;
	marquee_layer_set_text(marquee_layer, "");
	layer_set_update_proc(marquee_layer, marquee_layer_update);
	return marquee_layer;
}

void marquee_layer_destroy(MarqueeLayer* marquee_layer) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	if(data->timer != NULL)
		app_timer_cancel(data->timer);
	layer_destroy(marquee_layer);
}

Layer* marquee_layer_get_layer(MarqueeLayer* layer) {
	return layer; //well this is boring.
}

void marquee_layer_set_text(MarqueeLayer* marquee_layer, const char* text) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->text = text;
	data->scrolledness = 0;
	marquee_layer_schedule_tick(marquee_layer, data->initial_delay);
	layer_mark_dirty(marquee_layer);
}

void marquee_layer_set_text_color(MarqueeLayer* marquee_layer, GColor color) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->text_color = color;
	layer_mark_dirty(marquee_layer);
}

void marquee_layer_set_background_color(MarqueeLayer* marquee_layer, GColor color) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->background_color = color;
	layer_mark_dirty(marquee_layer);
}

void marquee_layer_set_font(MarqueeLayer* marquee_layer, GFont font) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->font = font;
	layer_mark_dirty(marquee_layer);
}

void marquee_layer_set_speed(MarqueeLayer* marquee_layer, uint16_t pixels_per_second) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->pixels_per_frame = pixels_per_second / FRAME_TIMEOUT;
	if(!data->pixels_per_frame)
		data->pixels_per_frame = 1;
}

void marquee_layer_set_delay(MarqueeLayer* marquee_layer, uint32_t initial_delay) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->initial_delay = initial_delay;
	marquee_layer_schedule_tick(marquee_layer, initial_delay);
}

void marquee_layer_set_enable_backlight(MarqueeLayer* marquee_layer, bool enable_backlight) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->enable_backlight = enable_backlight;
}

static int16_t marquee_layer_get_overflow_size(MarqueeLayer* marquee_layer) {
	GRect container = layer_get_bounds(marquee_layer);
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	static GRect max_bounds = {
		.origin = { .x = 0, .y = 0 },
		.size = { .w = INT16_MAX, .h = INT16_MAX }
	};
	GSize text_size = graphics_text_layout_get_content_size(data->text, data->font, max_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft);
	int16_t overflow = text_size.w - container.size.w;
	return overflow > 0 ? overflow : 0;
}

static void marquee_layer_update(MarqueeLayer* marquee_layer, GContext* ctx) {
	GRect bounds = layer_get_bounds(marquee_layer);
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	
	GRect box = {
		.origin = { .x = bounds.origin.x - data->scrolledness, .y = bounds.origin.y },
		.size = { .w = INT16_MAX, .h = bounds.size.h }
	};
	
	graphics_context_set_text_color(ctx, data->text_color);
	graphics_context_set_fill_color(ctx, data->background_color);
	
	graphics_fill_rect(ctx, bounds, 0, GCornerNone);
	
	graphics_draw_text(ctx, data->text, data->font, box, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
}

static void marquee_layer_schedule_tick(MarqueeLayer* marquee_layer, uint32_t timeout) {
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	if(data->timer)
		app_timer_reschedule(data->timer, timeout);
	else
		data->timer = app_timer_register(FRAME_TIMEOUT, marquee_layer_timer_handler, marquee_layer);
}

static void marquee_layer_timer_handler(void* context) {
	MarqueeLayer* marquee_layer = (MarqueeLayer*) context;
	MarqueeLayerData* data = layer_get_data(marquee_layer);
	data->timer = NULL;
	int32_t overflow =  marquee_layer_get_overflow_size(marquee_layer);
	data->scrolledness += data->pixels_per_frame;
	if(data->scrolledness > overflow)
		data->scrolledness = overflow;
	layer_mark_dirty(marquee_layer);
	
	if(data->enable_backlight)
		light_enable_interaction();
	
	if(data->scrolledness < overflow)
		marquee_layer_schedule_tick(marquee_layer, FRAME_TIMEOUT);
}
