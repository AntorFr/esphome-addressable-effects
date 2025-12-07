#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/components/light/addressable_light_effect.h"

namespace esphome {
namespace light {

// Color Twinkles palette types
enum ColorTwinklesPaletteType {
  COLOR_TWINKLES_PALETTE_CLOUD_COLORS,
  COLOR_TWINKLES_PALETTE_RAINBOW_COLORS,
  COLOR_TWINKLES_PALETTE_SNOW_COLORS,
  COLOR_TWINKLES_PALETTE_INCANDESCENT,
  COLOR_TWINKLES_PALETTE_PARTY_COLORS,
  COLOR_TWINKLES_PALETTE_OCEAN_COLORS,
  COLOR_TWINKLES_PALETTE_FOREST_COLORS,
  COLOR_TWINKLES_PALETTE_LAVA_COLORS,
};

class AddressableColorTwinklesEffect : public AddressableLightEffect {
 public:
  AddressableColorTwinklesEffect(const char *name) : AddressableLightEffect(name) {}

  void set_starting_brightness(uint8_t brightness) { starting_brightness_ = brightness; }
  void set_fade_in_speed(uint8_t speed) { fade_in_speed_ = speed; }
  void set_fade_out_speed(uint8_t speed) { fade_out_speed_ = speed; }
  void set_density(uint8_t density) { density_ = density; }
  void set_palette(ColorTwinklesPaletteType palette) { palette_type_ = palette; }

  void start() override {
    // Allocate direction flags (1 bit per LED, packed into bytes)
    auto light_state = this->get_addressable_();
    size_t num_leds = light_state->size();
    direction_flags_size_ = (num_leds + 7) / 8;
    direction_flags_ = new uint8_t[direction_flags_size_];
    memset(direction_flags_, 0, direction_flags_size_);
    
    // Setup palette based on type
    setup_palette();
  }

  void stop() override {
    if (direction_flags_ != nullptr) {
      delete[] direction_flags_;
      direction_flags_ = nullptr;
    }
  }

  void apply(AddressableLight &it, const Color &current_color) override {
    const uint32_t now = millis();
    
    // Only update every ~40ms for smooth animation
    if (now - last_update_ < 40) {
      return;
    }
    last_update_ = now;

    const size_t num_leds = it.size();

    // Make each pixel brighter or darker, depending on its 'direction' flag
    brighten_or_darken_each_pixel(it, fade_in_speed_, fade_out_speed_);

    // Now consider adding a new random twinkle
    if ((random_uint32() % 256) < density_) {
      uint16_t pos = random_uint32() % num_leds;
      // Only light up if pixel is currently off (or very dim)
      if (it[pos].get().is_on() == false || 
          (it[pos].get_red() < 10 && it[pos].get_green() < 10 && it[pos].get_blue() < 10)) {
        Color c = color_from_palette(random_uint32() % 256, starting_brightness_);
        it[pos] = c;
        set_pixel_direction(pos, GETTING_BRIGHTER);
      }
    }

    it.schedule_show();
  }

 protected:
  enum Direction { GETTING_DARKER = 0, GETTING_BRIGHTER = 1 };

  void setup_palette() {
    // Setup 16-color palette based on palette type
    switch (palette_type_) {
      case COLOR_TWINKLES_PALETTE_CLOUD_COLORS:
        // Blue and white, like clouds
        palette_[0] = Color(0, 0, 255);      // Blue
        palette_[1] = Color(0, 64, 255);
        palette_[2] = Color(64, 128, 255);
        palette_[3] = Color(128, 192, 255);
        palette_[4] = Color(192, 220, 255);
        palette_[5] = Color(255, 255, 255);  // White
        palette_[6] = Color(192, 220, 255);
        palette_[7] = Color(128, 192, 255);
        palette_[8] = Color(64, 128, 255);
        palette_[9] = Color(0, 64, 255);
        palette_[10] = Color(0, 0, 255);
        palette_[11] = Color(64, 128, 255);
        palette_[12] = Color(128, 192, 255);
        palette_[13] = Color(192, 220, 255);
        palette_[14] = Color(255, 255, 255);
        palette_[15] = Color(128, 192, 255);
        break;

      case COLOR_TWINKLES_PALETTE_RAINBOW_COLORS:
        // Full rainbow
        palette_[0] = Color(255, 0, 0);      // Red
        palette_[1] = Color(255, 64, 0);     // Orange
        palette_[2] = Color(255, 128, 0);
        palette_[3] = Color(255, 192, 0);
        palette_[4] = Color(255, 255, 0);    // Yellow
        palette_[5] = Color(128, 255, 0);
        palette_[6] = Color(0, 255, 0);      // Green
        palette_[7] = Color(0, 255, 128);
        palette_[8] = Color(0, 255, 255);    // Cyan
        palette_[9] = Color(0, 128, 255);
        palette_[10] = Color(0, 0, 255);     // Blue
        palette_[11] = Color(128, 0, 255);
        palette_[12] = Color(255, 0, 255);   // Magenta
        palette_[13] = Color(255, 0, 128);
        palette_[14] = Color(255, 0, 64);
        palette_[15] = Color(255, 0, 0);
        break;

      case COLOR_TWINKLES_PALETTE_SNOW_COLORS:
        // White and light blue, like snow
        palette_[0] = Color(255, 255, 255);  // White
        palette_[1] = Color(240, 248, 255);  // Alice Blue
        palette_[2] = Color(230, 240, 255);
        palette_[3] = Color(220, 235, 255);
        palette_[4] = Color(200, 220, 255);
        palette_[5] = Color(180, 200, 255);
        palette_[6] = Color(200, 220, 255);
        palette_[7] = Color(220, 235, 255);
        palette_[8] = Color(255, 255, 255);
        palette_[9] = Color(245, 250, 255);
        palette_[10] = Color(235, 245, 255);
        palette_[11] = Color(225, 240, 255);
        palette_[12] = Color(255, 255, 255);
        palette_[13] = Color(240, 248, 255);
        palette_[14] = Color(230, 245, 255);
        palette_[15] = Color(255, 255, 255);
        break;

      case COLOR_TWINKLES_PALETTE_INCANDESCENT:
        // Warm incandescent colors (fire orange/yellow)
        palette_[0] = Color(255, 147, 41);   // Candle
        palette_[1] = Color(255, 160, 50);
        palette_[2] = Color(255, 170, 60);
        palette_[3] = Color(255, 180, 70);
        palette_[4] = Color(255, 190, 80);
        palette_[5] = Color(255, 200, 90);
        palette_[6] = Color(255, 190, 80);
        palette_[7] = Color(255, 180, 70);
        palette_[8] = Color(255, 170, 60);
        palette_[9] = Color(255, 160, 50);
        palette_[10] = Color(255, 147, 41);
        palette_[11] = Color(255, 140, 35);
        palette_[12] = Color(255, 130, 30);
        palette_[13] = Color(255, 140, 35);
        palette_[14] = Color(255, 150, 45);
        palette_[15] = Color(255, 160, 55);
        break;

      case COLOR_TWINKLES_PALETTE_PARTY_COLORS:
        // Vibrant party colors
        palette_[0] = Color(85, 0, 171);     // Purple
        palette_[1] = Color(132, 0, 255);
        palette_[2] = Color(255, 0, 0);      // Red
        palette_[3] = Color(255, 85, 0);
        palette_[4] = Color(255, 170, 0);    // Orange
        palette_[5] = Color(255, 255, 0);    // Yellow
        palette_[6] = Color(0, 255, 0);      // Green
        palette_[7] = Color(0, 171, 85);
        palette_[8] = Color(0, 85, 171);
        palette_[9] = Color(0, 0, 255);      // Blue
        palette_[10] = Color(85, 0, 171);
        palette_[11] = Color(171, 0, 85);
        palette_[12] = Color(255, 0, 0);
        palette_[13] = Color(255, 85, 0);
        palette_[14] = Color(255, 170, 0);
        palette_[15] = Color(255, 255, 0);
        break;

      case COLOR_TWINKLES_PALETTE_OCEAN_COLORS:
        // Ocean blues and greens
        palette_[0] = Color(0, 0, 128);      // Navy
        palette_[1] = Color(0, 0, 170);
        palette_[2] = Color(0, 32, 192);
        palette_[3] = Color(0, 64, 200);
        palette_[4] = Color(0, 96, 200);
        palette_[5] = Color(0, 128, 200);    // Medium blue
        palette_[6] = Color(0, 150, 180);
        palette_[7] = Color(0, 170, 160);
        palette_[8] = Color(0, 180, 140);
        palette_[9] = Color(0, 190, 120);
        palette_[10] = Color(0, 200, 100);   // Aqua green
        palette_[11] = Color(0, 190, 120);
        palette_[12] = Color(0, 170, 160);
        palette_[13] = Color(0, 128, 200);
        palette_[14] = Color(0, 64, 200);
        palette_[15] = Color(0, 0, 170);
        break;

      case COLOR_TWINKLES_PALETTE_FOREST_COLORS:
        // Forest greens
        palette_[0] = Color(0, 64, 0);       // Dark green
        palette_[1] = Color(0, 96, 0);
        palette_[2] = Color(0, 128, 0);      // Green
        palette_[3] = Color(32, 160, 0);
        palette_[4] = Color(64, 192, 0);
        palette_[5] = Color(96, 200, 32);
        palette_[6] = Color(64, 192, 0);
        palette_[7] = Color(32, 160, 0);
        palette_[8] = Color(0, 128, 0);
        palette_[9] = Color(0, 96, 32);
        palette_[10] = Color(0, 64, 0);
        palette_[11] = Color(32, 80, 0);
        palette_[12] = Color(64, 96, 0);
        palette_[13] = Color(32, 80, 0);
        palette_[14] = Color(0, 64, 16);
        palette_[15] = Color(0, 80, 0);
        break;

      case COLOR_TWINKLES_PALETTE_LAVA_COLORS:
        // Lava reds and oranges
        palette_[0] = Color(128, 0, 0);      // Dark red
        palette_[1] = Color(170, 0, 0);
        palette_[2] = Color(200, 0, 0);
        palette_[3] = Color(255, 0, 0);      // Red
        palette_[4] = Color(255, 64, 0);
        palette_[5] = Color(255, 128, 0);    // Orange
        palette_[6] = Color(255, 192, 0);
        palette_[7] = Color(255, 255, 0);    // Yellow
        palette_[8] = Color(255, 192, 0);
        palette_[9] = Color(255, 128, 0);
        palette_[10] = Color(255, 64, 0);
        palette_[11] = Color(255, 0, 0);
        palette_[12] = Color(200, 0, 0);
        palette_[13] = Color(170, 0, 0);
        palette_[14] = Color(128, 0, 0);
        palette_[15] = Color(96, 0, 0);
        break;
    }
  }

  Color color_from_palette(uint8_t index, uint8_t brightness) {
    uint8_t palette_index = index >> 4;  // 0-15
    Color c = palette_[palette_index];
    
    // Scale by brightness
    uint8_t r = (c.r * brightness) >> 8;
    uint8_t g = (c.g * brightness) >> 8;
    uint8_t b = (c.b * brightness) >> 8;
    
    return Color(r, g, b);
  }

  Color make_brighter(const Color &color, uint8_t how_much_brighter) {
    // Add a fraction of the color to itself
    uint8_t r = color.r + ((color.r * how_much_brighter) >> 8);
    uint8_t g = color.g + ((color.g * how_much_brighter) >> 8);
    uint8_t b = color.b + ((color.b * how_much_brighter) >> 8);
    
    // Clamp to 255
    if (r < color.r) r = 255;
    if (g < color.g) g = 255;
    if (b < color.b) b = 255;
    
    return Color(r, g, b);
  }

  Color make_darker(const Color &color, uint8_t how_much_darker) {
    // Subtract a fraction from the color
    uint8_t scale = 255 - how_much_darker;
    uint8_t r = (color.r * scale) >> 8;
    uint8_t g = (color.g * scale) >> 8;
    uint8_t b = (color.b * scale) >> 8;
    
    return Color(r, g, b);
  }

  bool get_pixel_direction(uint16_t i) {
    uint16_t index = i / 8;
    uint8_t bit_num = i & 0x07;
    uint8_t and_mask = 1 << bit_num;
    return (direction_flags_[index] & and_mask) != 0;
  }

  void set_pixel_direction(uint16_t i, Direction dir) {
    uint16_t index = i / 8;
    uint8_t bit_num = i & 0x07;
    uint8_t or_mask = 1 << bit_num;
    uint8_t and_mask = 255 - or_mask;
    uint8_t value = direction_flags_[index] & and_mask;
    if (dir == GETTING_BRIGHTER) {
      value += or_mask;
    }
    direction_flags_[index] = value;
  }

  void brighten_or_darken_each_pixel(AddressableLight &it, uint8_t fade_up_amount, uint8_t fade_down_amount) {
    const size_t num_leds = it.size();
    
    for (size_t i = 0; i < num_leds; i++) {
      Color current = it[i].get();
      
      if (get_pixel_direction(i) == GETTING_DARKER) {
        // This pixel is getting darker
        Color darker = make_darker(current, fade_down_amount);
        it[i] = darker;
      } else {
        // This pixel is getting brighter
        Color brighter = make_brighter(current, fade_up_amount);
        it[i] = brighter;
        
        // Check if we've maxed out the brightness
        if (brighter.r == 255 || brighter.g == 255 || brighter.b == 255) {
          // Turn around and start getting darker
          set_pixel_direction(i, GETTING_DARKER);
        }
      }
    }
  }

  uint8_t starting_brightness_{64};
  uint8_t fade_in_speed_{32};
  uint8_t fade_out_speed_{20};
  uint8_t density_{255};
  ColorTwinklesPaletteType palette_type_{COLOR_TWINKLES_PALETTE_RAINBOW_COLORS};
  
  uint8_t *direction_flags_{nullptr};
  size_t direction_flags_size_{0};
  uint32_t last_update_{0};
  
  Color palette_[16];
};

}  // namespace light
}  // namespace esphome
