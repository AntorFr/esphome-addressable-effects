#pragma once

#include <utility>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/addressable_light_effect.h"

namespace esphome {
namespace light {

// Simplified palette structure for ESPHome (16 entries)
struct TwinkleFoxPaletteEntry {
  uint8_t index;
  uint8_t r, g, b;
};

// Predefined palettes
enum TwinkleFoxPaletteType {
  PALETTE_PARTY_COLORS = 0,
  PALETTE_OCEAN_COLORS,
  PALETTE_LAVA_COLORS,
  PALETTE_FOREST_COLORS,
  PALETTE_RAINBOW_COLORS,
  PALETTE_SNOW_COLORS,
  PALETTE_HOLLY_COLORS,
  PALETTE_ICE_COLORS,
  PALETTE_FAIRY_LIGHT,
  PALETTE_RETRO_C9,
};

class AddressableTwinkleFoxEffect : public AddressableLightEffect {
 public:
  explicit AddressableTwinkleFoxEffect(const char *name) : AddressableLightEffect(name) {}

  void start() override {
    auto &it = *this->get_addressable_();
    it.all() = Color::BLACK;
    this->setup_palette();
  }

  void apply(AddressableLight &it, const Color &current_color) override {
    const uint32_t now = millis();
    uint16_t prng16 = 11337;

    // Calculate background color
    Color bg = this->calculate_background();
    uint8_t background_brightness = (bg.r + bg.g + bg.b) / 3;

    for (int i = 0; i < it.size(); i++) {
      // Generate pseudo-random values for this pixel
      prng16 = (uint16_t)(prng16 * 2053) + 1384;
      uint16_t clock_offset = prng16;
      prng16 = (uint16_t)(prng16 * 2053) + 1384;
      
      // Speed multiplier (8/8 to 23/8)
      uint8_t speed_mult = ((((prng16 & 0xFF) >> 4) + (prng16 & 0x0F)) & 0x0F) + 0x08;
      uint32_t pixel_clock = (uint32_t)((now * speed_mult) >> 3) + clock_offset;
      uint8_t salt = prng16 >> 8;

      // Compute twinkle color for this pixel
      Color c = this->compute_one_twinkle(pixel_clock, salt);
      
      uint8_t c_brightness = (c.r + c.g + c.b) / 3;
      int16_t delta_bright = c_brightness - background_brightness;

      if (delta_bright >= 32 || (bg.r == 0 && bg.g == 0 && bg.b == 0)) {
        it[i] = c;
      } else if (delta_bright > 0) {
        // Blend between background and twinkle color
        uint8_t blend_amount = delta_bright * 8;
        it[i] = Color(
          ((uint16_t)bg.r * (255 - blend_amount) + (uint16_t)c.r * blend_amount) >> 8,
          ((uint16_t)bg.g * (255 - blend_amount) + (uint16_t)c.g * blend_amount) >> 8,
          ((uint16_t)bg.b * (255 - blend_amount) + (uint16_t)c.b * blend_amount) >> 8
        );
      } else {
        it[i] = bg;
      }
    }
    it.schedule_show();
  }

  void set_twinkle_speed(uint8_t speed) { this->twinkle_speed_ = speed; }
  void set_twinkle_density(uint8_t density) { this->twinkle_density_ = density; }
  void set_cool_like_incandescent(bool cool) { this->cool_like_incandescent_ = cool; }
  void set_background_color(Color color) { this->background_color_ = color; }
  void set_auto_background(bool auto_bg) { this->auto_background_ = auto_bg; }
  void set_palette(TwinkleFoxPaletteType palette) { this->palette_type_ = palette; }

 protected:
  uint8_t twinkle_speed_{4};
  uint8_t twinkle_density_{5};
  bool cool_like_incandescent_{true};
  bool auto_background_{false};
  Color background_color_{Color::BLACK};
  TwinkleFoxPaletteType palette_type_{PALETTE_PARTY_COLORS};
  
  // Current palette (16 RGB entries)
  Color palette_[16];

  void setup_palette() {
    switch (this->palette_type_) {
      case PALETTE_PARTY_COLORS:
        this->set_party_palette();
        break;
      case PALETTE_OCEAN_COLORS:
        this->set_ocean_palette();
        break;
      case PALETTE_LAVA_COLORS:
        this->set_lava_palette();
        break;
      case PALETTE_FOREST_COLORS:
        this->set_forest_palette();
        break;
      case PALETTE_RAINBOW_COLORS:
        this->set_rainbow_palette();
        break;
      case PALETTE_SNOW_COLORS:
        this->set_snow_palette();
        break;
      case PALETTE_HOLLY_COLORS:
        this->set_holly_palette();
        break;
      case PALETTE_ICE_COLORS:
        this->set_ice_palette();
        break;
      case PALETTE_FAIRY_LIGHT:
        this->set_fairy_light_palette();
        break;
      case PALETTE_RETRO_C9:
        this->set_retro_c9_palette();
        break;
      default:
        this->set_party_palette();
    }
  }

  Color calculate_background() {
    if (this->auto_background_ && 
        this->palette_[0].r == this->palette_[1].r &&
        this->palette_[0].g == this->palette_[1].g &&
        this->palette_[0].b == this->palette_[1].b) {
      Color bg = this->palette_[0];
      uint8_t bg_light = (bg.r + bg.g + bg.b) / 3;
      if (bg_light > 64) {
        return Color(bg.r >> 4, bg.g >> 4, bg.b >> 4);  // Scale to 1/16
      } else if (bg_light > 16) {
        return Color(bg.r >> 2, bg.g >> 2, bg.b >> 2);  // Scale to 1/4
      } else {
        return Color((bg.r * 86) >> 8, (bg.g * 86) >> 8, (bg.b * 86) >> 8);  // Scale to 1/3
      }
    }
    return this->background_color_;
  }

  Color compute_one_twinkle(uint32_t ms, uint8_t salt) {
    uint16_t ticks = ms >> (8 - this->twinkle_speed_);
    uint8_t fast_cycle = ticks & 0xFF;
    uint16_t slow_cycle16 = (ticks >> 8) + salt;
    slow_cycle16 += this->sin8_c(slow_cycle16 & 0xFF);
    slow_cycle16 = (slow_cycle16 * 2053) + 1384;
    uint8_t slow_cycle8 = (slow_cycle16 & 0xFF) + (slow_cycle16 >> 8);

    uint8_t bright = 0;
    if (((slow_cycle8 & 0x0E) / 2) < this->twinkle_density_) {
      bright = this->attack_decay_wave8(fast_cycle);
    }

    uint8_t hue = slow_cycle8 - salt;
    Color c;
    if (bright > 0) {
      c = this->color_from_palette(hue, bright);
      if (this->cool_like_incandescent_) {
        this->cool_like_incandescent_fn(c, fast_cycle);
      }
    } else {
      c = Color::BLACK;
    }
    return c;
  }

  uint8_t attack_decay_wave8(uint8_t i) {
    if (i < 86) {
      return i * 3;
    } else {
      i -= 86;
      return 255 - (i + (i / 2));
    }
  }

  void cool_like_incandescent_fn(Color &c, uint8_t phase) {
    if (phase < 128) return;
    uint8_t cooling = (phase - 128) >> 4;
    c.g = (c.g > cooling) ? c.g - cooling : 0;
    c.b = (c.b > cooling * 2) ? c.b - cooling * 2 : 0;
  }

  // Simple sin8 approximation
  uint8_t sin8_c(uint8_t theta) {
    static const uint8_t sin8_table[64] = {
      0, 6, 12, 19, 25, 31, 37, 43, 49, 54, 60, 65, 71, 76, 81, 85,
      90, 94, 98, 102, 106, 109, 112, 115, 118, 120, 122, 124, 126, 127, 128, 128,
      128, 128, 127, 126, 124, 122, 120, 118, 115, 112, 109, 106, 102, 98, 94, 90,
      85, 81, 76, 71, 65, 60, 54, 49, 43, 37, 31, 25, 19, 12, 6, 0
    };
    uint8_t section = theta >> 6;
    uint8_t index = theta & 0x3F;
    uint8_t result;
    switch (section) {
      case 0: result = 128 + sin8_table[index]; break;
      case 1: result = 128 + sin8_table[63 - index]; break;
      case 2: result = 128 - sin8_table[index]; break;
      default: result = 128 - sin8_table[63 - index]; break;
    }
    return result;
  }

  Color color_from_palette(uint8_t index, uint8_t brightness) {
    // Get palette index (0-15)
    uint8_t palette_idx = index >> 4;
    Color c = this->palette_[palette_idx];
    // Apply brightness
    return Color(
      ((uint16_t)c.r * brightness) >> 8,
      ((uint16_t)c.g * brightness) >> 8,
      ((uint16_t)c.b * brightness) >> 8
    );
  }

  // Palette definitions
  void set_party_palette() {
    // Magenta, red, yellow, blue, purple sequence
    palette_[0] = Color(128, 0, 128);   // Purple
    palette_[1] = Color(255, 0, 128);   // Magenta
    palette_[2] = Color(255, 0, 0);     // Red
    palette_[3] = Color(255, 64, 0);    // Orange
    palette_[4] = Color(255, 128, 0);   // Orange-Yellow
    palette_[5] = Color(255, 255, 0);   // Yellow
    palette_[6] = Color(128, 255, 0);   // Yellow-Green
    palette_[7] = Color(0, 255, 0);     // Green
    palette_[8] = Color(0, 255, 128);   // Green-Cyan
    palette_[9] = Color(0, 255, 255);   // Cyan
    palette_[10] = Color(0, 128, 255);  // Cyan-Blue
    palette_[11] = Color(0, 0, 255);    // Blue
    palette_[12] = Color(64, 0, 255);   // Blue-Purple
    palette_[13] = Color(128, 0, 255);  // Purple
    palette_[14] = Color(192, 0, 192);  // Magenta
    palette_[15] = Color(255, 0, 128);  // Pink
  }

  void set_ocean_palette() {
    for (int i = 0; i < 16; i++) {
      uint8_t blue = 128 + (i * 8);
      uint8_t green = 64 + (i * 6);
      palette_[i] = Color(0, green, blue);
    }
  }

  void set_lava_palette() {
    palette_[0] = Color(0, 0, 0);
    palette_[1] = Color(32, 0, 0);
    palette_[2] = Color(64, 0, 0);
    palette_[3] = Color(96, 0, 0);
    palette_[4] = Color(128, 0, 0);
    palette_[5] = Color(160, 16, 0);
    palette_[6] = Color(192, 32, 0);
    palette_[7] = Color(224, 48, 0);
    palette_[8] = Color(255, 64, 0);
    palette_[9] = Color(255, 96, 0);
    palette_[10] = Color(255, 128, 0);
    palette_[11] = Color(255, 160, 0);
    palette_[12] = Color(255, 192, 0);
    palette_[13] = Color(255, 224, 0);
    palette_[14] = Color(255, 255, 64);
    palette_[15] = Color(255, 255, 255);
  }

  void set_forest_palette() {
    for (int i = 0; i < 16; i++) {
      uint8_t green = 64 + (i * 12);
      uint8_t red = i * 4;
      uint8_t blue = i * 2;
      palette_[i] = Color(red, green, blue);
    }
  }

  void set_rainbow_palette() {
    // HSV rainbow
    palette_[0] = Color(255, 0, 0);     // Red
    palette_[1] = Color(255, 48, 0);
    palette_[2] = Color(255, 96, 0);
    palette_[3] = Color(255, 144, 0);
    palette_[4] = Color(255, 192, 0);   // Yellow
    palette_[5] = Color(192, 255, 0);
    palette_[6] = Color(96, 255, 0);
    palette_[7] = Color(0, 255, 0);     // Green
    palette_[8] = Color(0, 255, 96);
    palette_[9] = Color(0, 255, 192);   // Cyan
    palette_[10] = Color(0, 192, 255);
    palette_[11] = Color(0, 96, 255);
    palette_[12] = Color(0, 0, 255);    // Blue
    palette_[13] = Color(96, 0, 255);
    palette_[14] = Color(192, 0, 255);  // Purple
    palette_[15] = Color(255, 0, 192);  // Pink
  }

  void set_snow_palette() {
    // White and light blue
    for (int i = 0; i < 16; i++) {
      uint8_t white = 200 + (i % 4) * 14;
      uint8_t blue_tint = (i % 3) * 20;
      palette_[i] = Color(white - blue_tint, white - blue_tint / 2, white);
    }
  }

  void set_holly_palette() {
    // Red and green for Christmas
    palette_[0] = Color(255, 0, 0);     // Red
    palette_[1] = Color(255, 0, 0);
    palette_[2] = Color(200, 0, 0);
    palette_[3] = Color(160, 0, 0);
    palette_[4] = Color(0, 255, 0);     // Green
    palette_[5] = Color(0, 255, 0);
    palette_[6] = Color(0, 200, 0);
    palette_[7] = Color(0, 160, 0);
    palette_[8] = Color(255, 0, 0);     // Red
    palette_[9] = Color(255, 0, 0);
    palette_[10] = Color(200, 0, 0);
    palette_[11] = Color(160, 0, 0);
    palette_[12] = Color(0, 255, 0);    // Green
    palette_[13] = Color(0, 255, 0);
    palette_[14] = Color(0, 200, 0);
    palette_[15] = Color(0, 160, 0);
  }

  void set_ice_palette() {
    // Light blues and whites
    palette_[0] = Color(255, 255, 255);
    palette_[1] = Color(224, 240, 255);
    palette_[2] = Color(192, 224, 255);
    palette_[3] = Color(160, 208, 255);
    palette_[4] = Color(128, 192, 255);
    palette_[5] = Color(96, 176, 255);
    palette_[6] = Color(64, 160, 255);
    palette_[7] = Color(32, 144, 255);
    palette_[8] = Color(64, 160, 255);
    palette_[9] = Color(96, 176, 255);
    palette_[10] = Color(128, 192, 255);
    palette_[11] = Color(160, 208, 255);
    palette_[12] = Color(192, 224, 255);
    palette_[13] = Color(224, 240, 255);
    palette_[14] = Color(255, 255, 255);
    palette_[15] = Color(240, 248, 255);
  }

  void set_fairy_light_palette() {
    // Warm white incandescent look
    Color fairy_white = Color(255, 200, 100);
    for (int i = 0; i < 16; i++) {
      palette_[i] = fairy_white;
    }
  }

  void set_retro_c9_palette() {
    // Classic C9 Christmas bulb colors
    palette_[0] = Color(255, 0, 0);     // Red
    palette_[1] = Color(255, 0, 0);
    palette_[2] = Color(255, 128, 0);   // Orange
    palette_[3] = Color(255, 128, 0);
    palette_[4] = Color(255, 255, 0);   // Yellow
    palette_[5] = Color(255, 255, 0);
    palette_[6] = Color(0, 255, 0);     // Green
    palette_[7] = Color(0, 255, 0);
    palette_[8] = Color(0, 0, 255);     // Blue
    palette_[9] = Color(0, 0, 255);
    palette_[10] = Color(128, 0, 255);  // Purple
    palette_[11] = Color(128, 0, 255);
    palette_[12] = Color(255, 0, 128);  // Pink
    palette_[13] = Color(255, 0, 128);
    palette_[14] = Color(255, 255, 255); // White
    palette_[15] = Color(255, 255, 255);
  }
};

}  // namespace light
}  // namespace esphome
