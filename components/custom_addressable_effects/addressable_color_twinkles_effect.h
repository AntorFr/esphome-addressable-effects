#pragma once

#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "esphome/components/light/addressable_light_effect.h"

namespace esphome {
namespace light {

static const char *const TAG_TWINKLES = "color_twinkles";

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
    auto &it = *this->get_addressable_();
    size_t num_leds = it.size();
    
    // Allocate direction flags (1 bit per LED, packed into bytes)
    direction_flags_size_ = (num_leds + 7) / 8;
    direction_flags_ = new uint8_t[direction_flags_size_];
    memset(direction_flags_, 0, direction_flags_size_);
    
    // Allocate color indices (1 byte per LED for palette index)
    color_indices_ = new uint8_t[num_leds];
    memset(color_indices_, 0, num_leds);
    
    // Initialize all effect_data to 0 (brightness) and turn off LEDs
    for (auto view : it) {
      view.set_effect_data(0);  // brightness = 0
    }
    it.all() = Color::BLACK;
    it.schedule_show();
    
    // Setup palette based on type
    setup_palette();
  }

  void stop() override {
    if (direction_flags_ != nullptr) {
      delete[] direction_flags_;
      direction_flags_ = nullptr;
    }
    if (color_indices_ != nullptr) {
      delete[] color_indices_;
      color_indices_ = nullptr;
    }
  }

  void apply(AddressableLight &it, const Color &current_color) override {
    const uint32_t now = millis();
    
    // Only update every ~40ms for smooth animation
    if (now - last_update_ < 40) {
      return;
    }
    last_update_ = now;
    frame_count_++;

    const size_t num_leds = it.size();

    // Debug log every 1000 frames
    if (frame_count_ % 1000 == 0) {
      ESP_LOGD(TAG_TWINKLES, "=== Frame %lu - LED status ===", frame_count_);
      for (size_t i = 0; i < num_leds && i < 20; i++) {
        uint8_t brightness = it[i].get_effect_data();
        const char* dir = get_pixel_direction(i) == GETTING_BRIGHTER ? "UP" : "DOWN";
        ESP_LOGD(TAG_TWINKLES, "LED[%d]: bright=%d color_idx=%d dir=%s", i, brightness, color_indices_[i], dir);
      }
    }

    // Update each pixel brightness and render
    size_t idx = 0;
    for (auto view : it) {
      uint8_t brightness = view.get_effect_data();
      
      if (brightness > 0) {
        if (get_pixel_direction(idx) == GETTING_DARKER) {
          // Fade down
          if (brightness > fade_out_speed_) {
            brightness -= fade_out_speed_;
          } else {
            brightness = 0;
          }
        } else {
          // Fade up
          uint16_t new_bright = brightness + fade_in_speed_;
          if (new_bright >= 255) {
            brightness = 255;
            set_pixel_direction(idx, GETTING_DARKER);  // Start fading down
          } else {
            brightness = new_bright;
          }
        }
        
        view.set_effect_data(brightness);
        
        // Render color from palette scaled by brightness
        if (brightness > 0) {
          view = color_from_palette(color_indices_[idx], brightness);
        } else {
          view = Color::BLACK;
        }
      } else {
        view = Color::BLACK;
      }
      
      idx++;
    }

    // Now consider adding a new random twinkle
    if ((random_uint32() % 256) < density_) {
      uint16_t pos = random_uint32() % num_leds;
      uint8_t brightness = it[pos].get_effect_data();
      
      // Only light up if pixel is currently off
      if (brightness == 0) {
        color_indices_[pos] = random_uint32() % 256;  // Random palette position
        it[pos].set_effect_data(starting_brightness_);
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

  uint8_t starting_brightness_{64};
  uint8_t fade_in_speed_{8};
  uint8_t fade_out_speed_{4};
  uint8_t density_{80};
  ColorTwinklesPaletteType palette_type_{COLOR_TWINKLES_PALETTE_RAINBOW_COLORS};
  
  uint8_t *direction_flags_{nullptr};
  size_t direction_flags_size_{0};
  uint8_t *color_indices_{nullptr};
  uint32_t last_update_{0};
  uint32_t frame_count_{0};
  
  Color palette_[16];
};

}  // namespace light
}  // namespace esphome
