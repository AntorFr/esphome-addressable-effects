# ESPHome Custom Addressable Effects

A collection of custom addressable LED effects for ESPHome.

## Installation

Add the following to your ESPHome configuration:

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-addressable-effects
      ref: main
    components: [custom_addressable_effects]

custom_addressable_effects:
```

Then add the effects to your light configuration:

```yaml
light:
  - platform: neopixelbus  # or esp32_rmt_led_strip, fastled_clockless, etc.
    # ... your light config ...
    effects:
      - addressable_twinklefox:
          name: "TwinkleFox"
      - addressable_color_twinkles:
          name: "Color Twinkles"
      - addressable_stars:
          name: "Stars"
      - addressable_christmas:
          name: "Christmas"
```

## Available Effects

### TwinkleFox

A twinkling effect inspired by the FastLED TwinkleFox pattern. LEDs randomly twinkle with colors from a selected palette.

```yaml
- addressable_twinklefox:
    name: "TwinkleFox"
    palette: party_colors      # Color palette (see below)
    twinkle_speed: 4           # Speed of twinkle animation (1-8, default: 4)
    twinkle_density: 5         # How many LEDs twinkle at once (1-8, default: 5)
    cool_like_incandescent: true  # Fade to warm colors like incandescent bulbs (default: true)
    auto_background: false     # Automatically set background from palette (default: false)
    color:                     # Background color (when auto_background is false)
      red: 0%
      green: 0%
      blue: 0%
```

#### Available Palettes

| Palette | Description |
|---------|-------------|
| `party_colors` | Vibrant party colors (default) |
| `rainbow_colors` | Full rainbow spectrum |
| `ocean_colors` | Blues and greens |
| `lava_colors` | Reds, oranges, and yellows |
| `forest_colors` | Greens and earth tones |
| `snow_colors` | Whites and light blues |
| `holly_colors` | Christmas red and green |
| `ice_colors` | Cool whites and blues |
| `fairy_light` | Warm white incandescent look |
| `retro_c9` | Classic C9 Christmas bulb colors |

### Color Twinkles

LEDs twinkle with colors from a selected palette, smoothly fading in and out.

```yaml
- addressable_color_twinkles:
    name: "Color Twinkles"
    palette: rainbow_colors    # Color palette (see below)
    starting_brightness: 64    # Initial brightness when a twinkle starts (0-255, default: 64)
    fade_in_speed: 8           # Speed of fade in (0-255, default: 8)
    fade_out_speed: 4          # Speed of fade out (0-255, default: 4)
    density: 80                # Probability of new twinkles (0-255, default: 80)
```

#### Available Palettes

| Palette | Description |
|---------|-------------|
| `rainbow_colors` | Full rainbow spectrum (default) |
| `cloud_colors` | Blues and whites, like clouds |
| `snow_colors` | Whites and light blues |
| `incandescent` | Warm incandescent colors |
| `party_colors` | Vibrant party colors |
| `ocean_colors` | Ocean blues and greens |
| `forest_colors` | Forest greens |
| `lava_colors` | Lava reds and oranges |

### Stars

LEDs randomly light up and fade like twinkling stars.

```yaml
- addressable_stars:
    name: "Stars"
    stars_probability: 10%     # Probability of a new star appearing (default: 10%)
    color:                     # Star color (uses light color if all zeros)
      red: 0%
      green: 0%
      blue: 0%
      white: 0%
```

### Christmas

Static alternating red and green pattern, perfect for Christmas decorations.

```yaml
- addressable_christmas:
    name: "Christmas"
    bit_size: 1                # Number of consecutive LEDs per color (default: 1)
    blank_size: 0              # Number of blank LEDs between colors (default: 0)
```

#### Examples

```yaml
# Single LED alternating: R G R G R G
- addressable_christmas:
    name: "Christmas Classic"
    bit_size: 1
    blank_size: 0

# Two LEDs per color: RR GG RR GG
- addressable_christmas:
    name: "Christmas Double"
    bit_size: 2
    blank_size: 0

# With spacing: R _ G _ R _ G _
- addressable_christmas:
    name: "Christmas Spaced"
    bit_size: 1
    blank_size: 1
```

## Full Example

```yaml
esphome:
  name: led-strip

esp32:
  board: esp32dev
  framework:
    type: arduino

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

logger:
api:
ota:
  platform: esphome

external_components:
  - source:
      type: git
      url: https://github.com/AntorFr/esphome-addressable-effects
      ref: main
    components: [custom_addressable_effects]

custom_addressable_effects:

light:
  - platform: neopixelbus
    type: GRB
    variant: WS2812
    pin: GPIO16
    num_leds: 60
    name: "LED Strip"
    effects:
      - addressable_twinklefox:
          name: "TwinkleFox Party"
          palette: party_colors
      - addressable_twinklefox:
          name: "TwinkleFox Snow"
          palette: snow_colors
          twinkle_speed: 3
      - addressable_twinklefox:
          name: "TwinkleFox Fairy"
          palette: fairy_light
          cool_like_incandescent: true
      - addressable_color_twinkles:
          name: "Color Twinkles Forest"
          palette: forest_colors
      - addressable_color_twinkles:
          name: "Color Twinkles Lava"
          palette: lava_colors
          fade_in_speed: 12
          fade_out_speed: 6
      - addressable_stars:
          name: "Stars"
          stars_probability: 15%
      - addressable_christmas:
          name: "Christmas"
          bit_size: 2
```

## Compatibility

- ESPHome 2025.11.0 and later
- Works with all addressable LED platforms (neopixelbus, fastled_clockless, esp32_rmt_led_strip, etc.)
- Tested on ESP8266 and ESP32

## License

MIT License - See [LICENSE](LICENSE) file for details.
