#pragma once

#include <utility>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/light/addressable_light.h"
#include "esphome/components/light/addressable_light_effect.h"

namespace esphome {
namespace light {

struct AddressableColorStarsEffectColor {
    uint8_t r, g, b, w;
};

class AddressableStarsEffect : public AddressableLightEffect {
 public:
  explicit AddressableStarsEffect(const char *name) : AddressableLightEffect(name) {}
  void start() override {
    auto &it = *this->get_addressable_();
    it.all() = Color::BLACK;
    it.schedule_show(); 
  }
  
  void apply(AddressableLight &it, const Color &current_color) override {
    for (auto view : it) {     
        Color effect_color = (this->color_.is_on() ? this->color_ : current_color);

        if (view.get_effect_data()==0 && (random_float() * 500 < this->stars_probability_)){
              view.set_effect_data(255);
        } 
        if (view.get_effect_data() > 0) {
            float intensit = -1*pow(view.get_effect_data()/180.1,2);
            view = Color(effect_color.r * exp(intensit),
                         effect_color.g * exp(intensit),
                         effect_color.b * exp(intensit),
                         effect_color.w * exp(intensit));
        } else {
            view = Color::BLACK;
        }
        if (view.get_effect_data()%2 == 1) {
          view.set_effect_data((view.get_effect_data()>2)?view.get_effect_data()-2:2);
        }
        else if (view.get_effect_data() > 0) {
          view.set_effect_data((view.get_effect_data()<=254)?view.get_effect_data()+2:0);
        }
    }
  }
  
  void set_stars_probability(float stars_probability) { this->stars_probability_ = stars_probability * 500; }
  void set_color(AddressableColorStarsEffectColor color) { this->color_ = Color(color.r, color.g, color.b, color.w); }

 protected:
  float stars_probability_{50};
  Color color_{0, 0, 0, 0};
};

}  // namespace light
}  // namespace esphome
