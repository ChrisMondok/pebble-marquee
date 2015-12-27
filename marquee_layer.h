#pragma once

#include<pebble.h>

typedef Layer MarqueeLayer;

MarqueeLayer* marquee_layer_create(GRect frame);
void marquee_layer_destroy(MarqueeLayer* marquee_layer);

Layer* marquee_layer_get_layer(MarqueeLayer* marquee_layer);

void marquee_layer_set_text(MarqueeLayer* marquee_layer, const char* text);
void marquee_layer_set_font(MarqueeLayer* marquee_layer, GFont font);
void marquee_layer_set_text_color(MarqueeLayer* marquee_layer, GColor text_color);
void marquee_layer_set_background_color(MarqueeLayer* marquee_layer, GColor background_color);

void marquee_layer_set_speed(MarqueeLayer* marquee_layer, uint16_t pixels_per_second);
void marquee_layer_set_delay(MarqueeLayer* marquee_layer, uint32_t initial_delay);

void marquee_layer_set_enable_backlight(MarqueeLayer* marquee_layer, bool enable_backlight);
