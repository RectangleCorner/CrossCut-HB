#include "HAL/pico/include/input/GamecubeControllerInput.hpp"
#include "comms/backend_init.hpp"
#include "config_defaults.hpp"
#include "core/CommunicationBackend.hpp"
#include "core/KeyboardMode.hpp"
#include "core/Persistence.hpp"
#include "core/mode_selection.hpp"
#include "core/pinout.hpp"
#include "core/state.hpp"
#include "hardware/watchdog.h"
#include "reboot.hpp"
// #include "display/DisplayMode.hpp"
// #include "display/InputDisplay.hpp"
// #include "display/RgbBrightnessMenu.hpp"
#include "input/DebouncedGpioButtonInput.hpp"
#include "input/NunchukInput.hpp"
#include "reboot.hpp"
#include "stdlib.hpp"

// #include <Adafruit_SSD1306.h>
// #include <Wire.h>
#include <config.pb.h>
#include <cstring> // display
#include <lib/OneBitDisplay/OneBitDisplay.h> //display
#include <pico/bootrom.h>
#include <string> //display

#ifndef DISPLAY_I2C_ADDR
#define DISPLAY_I2C_ADDR -1
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN 8 // Corresponds to GPIO pin connected to OLED display's SDA pin.
#endif

#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN 9 // Corresponds to GPIO pin connected to OLED display's SCL pin.
#endif

#ifndef I2C_BLOCK
#define I2C_BLOCK i2c0 // Set depending on which pair of pins you are using - see below.
#endif
#ifndef I2C_SPEED
#define I2C_SPEED \
    1000000 // Common values are 100000 for standard, 400000 for fast and 800000 ludicrous speed.
#endif

#ifndef DISPLAY_SIZE
#define DISPLAY_SIZE OLED_128x64
#endif

#ifndef DISPLAY_FLIP
#define DISPLAY_FLIP 0
#endif

#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 0
#endif

#ifndef DISPLAY_USEWIRE
#define DISPLAY_USEWIRE 1
#endif
std::string dispCommBackend = "BACKEND";
std::string dispMode = "";
std::string leftLayout;
std::string centerLayout;
std::string rightLayout;

OBDISP obd;
uint8_t ucBackBuffer[1024];

typedef enum {
    Contraption = 0,
    PSPassthru = 1
} Core1Option;

Config config = default_config;

GpioButtonMapping button_mappings[] = {
    { BTN_LF1, 2  }, //  RIGHT
    { BTN_LF2, 3  }, //  DOWN
    { BTN_LF3, 4  }, //  LEFT
    { BTN_LF4, 5  }, //

    { BTN_LT1, 6  }, //  Up
    { BTN_LT2, 7  }, //  FN

    { BTN_MB1, 0  }, //  S2
    { BTN_MB2, 10 },
    { BTN_MB3, 11 },

    { BTN_RT1, 14 }, //  L3
    { BTN_RT2, 15 }, //  A1
    { BTN_RT3, 13 }, //  S1
    { BTN_RT4, 12 }, //
    { BTN_RT5, 16 }, //

    { BTN_RF1, 26 }, //  B1
    { BTN_RF2, 21 }, //  B2
    { BTN_RF3, 19 }, //  R2
    { BTN_RF4, 17 }, //  L2

    { BTN_RF5, 27 }, //  B3
    { BTN_RF6, 22 }, //  B4
    { BTN_RF7, 20 }, //  R1
    { BTN_RF8, 18 }, //  L1
};
const size_t button_count = sizeof(button_mappings) / sizeof(GpioButtonMapping);

DebouncedGpioButtonInput<button_count> gpio_input(button_mappings);

const Pinout pinout = { .joybus_data = 28,
                        .mux = -1,
                        .nunchuk_detect = -1,
                        .nunchuk_sda = -1,
                        .nunchuk_scl = -1,
                        .rumble = 23,
                        .display_sda = 8,
                        .display_scl = 9 };

CommunicationBackend **backends = nullptr;
size_t backend_count;
KeyboardMode *current_kb_mode = nullptr;

void setup() {
    static InputState inputs;

    // Create GPIO input source and use it to read button states for checking button holds.
    gpio_input.UpdateInputs(inputs);

    // Check bootsel button hold as early as possible for safety.
    if (inputs.rt2 && inputs.rt3 && inputs.rt4 && inputs.rt5) {
        reboot_bootloader();
    }

    // Turn on LED to indicate firmware booted.
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(pinout.rumble);
    gpio_set_dir(pinout.rumble, GPIO_OUT);

    Core1Option core1_option = Contraption;

    // Attempt to load config, or write default config to flash if failed to load config.
    if (!persistence.LoadConfig(config)) {
        persistence.SaveConfig(config);
    }

    // Create array of input sources to be used.
    static InputSource *input_sources[] = { &gpio_input };
    size_t input_source_count = sizeof(input_sources) / sizeof(InputSource *);

    backend_count =
        initialize_backends(backends, inputs, input_sources, input_source_count, config, pinout);

    setup_mode_activation_bindings(config.game_mode_configs, config.game_mode_configs_count);
}

void loop() {
    select_mode(backends, backend_count, config);

    if (dispMode == "MELEE") {
        if (dispCommBackend != "GCN") {
            // if (true) {
            //  get exactly 2 khz input scanning
            const uint32_t interval = 500; // microseconds
            const uint32_t quarterInterval = interval / 4; // unit of 4 microseconds
            const uint32_t beforeMicros = micros();
            uint32_t afterMicros = beforeMicros;
            while ((afterMicros - beforeMicros) < interval) {
                afterMicros = micros();
            }
        }
        for (size_t i = 0; i < backend_count; i++) {
            backends[i]->SendReport(true, config.travelTime);
        }
    } else {
        for (size_t i = 0; i < backend_count; i++) {
            backends[i]->SendReport(false, false);
        }
    }
    if (current_kb_mode != nullptr) {
        current_kb_mode->SendReport(backends[0]->GetInputs(), false);
    }
}

/* Button inputs are read from the second core */
GamecubeControllerInput *gcc = nullptr;

// Adafruit_SSD1306 display(128, 64, &Wire1);

// IntegratedDisplay *display_backend = nullptr;

void setup1() {
    while (!backend_count || backends == nullptr) {
        delay(1);
    }

    gpio_pull_up(1); // pull up gcc pin so it has a status of off instead of undeterminable

    // These have to be initialized after backends.
    CommunicationBackendId primary_backend_id = backends[0]->BackendId();
    switch (primary_backend_id) {
        case COMMS_BACKEND_DINPUT:
            dispCommBackend = "DINPUT";
            break;
        case COMMS_BACKEND_NINTENDO_SWITCH:
            dispCommBackend = "SWITCH";
            break;
        case COMMS_BACKEND_XINPUT:
            dispCommBackend = "XINPUT";
            break;
        case COMMS_BACKEND_GAMECUBE:
            dispCommBackend = "GCN";
            break;
        case COMMS_BACKEND_N64:
            dispCommBackend = "N64";
            break;
        case COMMS_BACKEND_SNES:
            dispCommBackend = "SNES";
        case COMMS_BACKEND_UNSPECIFIED: // Fall back to configurator if invalid
                                        // backend selected.
        case COMMS_BACKEND_CONFIGURATOR:
        default:
            dispCommBackend = "CONFIG";
            dispMode = "MODE";
    }
    if (dispCommBackend != "N64") {
        gcc = new GamecubeControllerInput(1, 1000, pio1);
    }

    // static MenuButtonHints menu_button_hints(primary_backend_id);
    /* static InputDisplay input_display(
         platform_fighter_buttons,
         platform_fighter_buttons_count,
         primary_backend_id
     );
    */

    // Wire1.setSDA(8);
    // Wire1.setSCL(9);
    //  Wire1.setClock(1'000'000UL);
    //  Wire1.begin();

    while (backends == nullptr) {
        tight_loop_contents();
    }
    // Initialize OLED.
    obdI2CInit(
        &obd,
        DISPLAY_SIZE,
        DISPLAY_I2C_ADDR,
        DISPLAY_FLIP,
        DISPLAY_INVERT,
        DISPLAY_USEWIRE,
        I2C_SDA_PIN,
        I2C_SCL_PIN,
        I2C_BLOCK,
        -1,
        I2C_SPEED
    );

    // Configure display layout options. Change the strings below to make a selection.
    leftLayout = "circles"; // circles, circlesWASD, squares, squaresWASD, htangl
    centerLayout = "circles"; // circles, circles3Button, squares, squares3Button, htangl
    rightLayout = "circles"; // circles, squares, circles19Button, squares19Button, htangl

    obdSetBackBuffer(&obd, ucBackBuffer);
    // Clear screen and render.
    obdFill(&obd, 0, 1);
}

void loop1() {

    if (gcc != nullptr) {
        gcc->UpdateInputs(backends[0]->GetInputs());
    }

    if (dispCommBackend != "CONFIG") {
        dispMode = backends[0]->CurrentGameMode()->GetConfig()->name;
    }
    if (dispMode == "FGC" || dispMode == "FGC ALT") {
        leftLayout = "FGC";
        rightLayout = "FGC";
    } else if (dispMode == "SMASH 64") {
        leftLayout = "SMASH 64";
        rightLayout = "SMASH 64";
    } else {
        leftLayout = "circles";
        rightLayout = "circles";
    }
    obdFill(&obd, 0, 0);
    // Write communication backend to OLED starting in the top left.
    char char_dispCommBackend[dispCommBackend.length() + 1];
    strcpy(char_dispCommBackend, dispCommBackend.c_str()); // convert string to char
    obdWriteString(&obd, 0, 0, 0, char_dispCommBackend, FONT_6x8, 0, 0);

    // Write current mode to OLED in the top right.
    // For the x position of the string, we are subtracting the max position (128 for a 128x64
    // px display) by the number of characters * the font width in px.
    char char_dispMode[dispMode.length() + 1];
    strcpy(char_dispMode, dispMode.c_str()); // convert string to char
    obdWriteString(&obd, 0, 128 - (dispMode.length() * 6), 0, char_dispMode, FONT_6x8, 0, 0);
    if (leftLayout == "circles") {
        obdPreciseEllipse(&obd, 6, 29, 4, 4, 1, backends[0]->GetInputs().lf4);

        obdPreciseEllipse(&obd, 15, 23, 4, 4, 1, backends[0]->GetInputs().lf3);
        obdPreciseEllipse(&obd, 25, 22, 4, 4, 1, backends[0]->GetInputs().lf2);
        obdPreciseEllipse(&obd, 35, 27, 4, 4, 1, backends[0]->GetInputs().lf1);
        obdPreciseEllipse(&obd, 38, 52, 4, 4, 1, backends[0]->GetInputs().lt1);
        obdPreciseEllipse(&obd, 46, 58, 4, 4, 1, backends[0]->GetInputs().lt2);
    } else if (leftLayout == "SMASH 64") {
        obdPreciseEllipse(&obd, 6, 29, 4, 4, 1, backends[0]->GetInputs().lf4);
        obdPreciseEllipse(&obd, 15, 23, 4, 4, 1, backends[0]->GetInputs().lf3);
        obdPreciseEllipse(&obd, 25, 22, 4, 4, 1, backends[0]->GetInputs().lf2);
        obdPreciseEllipse(&obd, 35, 27, 4, 4, 1, backends[0]->GetInputs().lf1);
        obdPreciseEllipse(&obd, 38, 52, 4, 4, 1, backends[0]->GetInputs().lt1);
        // obdPreciseEllipse(&obd, 46, 58, 4, 4, 1, backends[0]->GetInputs().mod_y);
    } else if (leftLayout == "FGC" || leftLayout == "FGC ALT") {
        // obdPreciseEllipse(&obd, 6, 29, 4, 4, 1, backends[0]->GetInputs().l);
        obdPreciseEllipse(&obd, 15, 23, 4, 4, 1, backends[0]->GetInputs().lf3);

        obdPreciseEllipse(&obd, 25, 22, 4, 4, 1, backends[0]->GetInputs().lf2);

        obdPreciseEllipse(&obd, 35, 27, 4, 4, 1, backends[0]->GetInputs().lf1);

        obdPreciseEllipse(&obd, 38, 52, 4, 4, 1, backends[0]->GetInputs().lt1);
        obdPreciseEllipse(&obd, 46, 58, 4, 4, 1, backends[0]->GetInputs().lt2);
    }
    // if (centerLayout == "circles") {
    obdPreciseEllipse(&obd, 64, 27, 4, 4, 1, backends[0]->GetInputs().mb1);
    //}
    if (rightLayout == "circles") {
        obdPreciseEllipse(&obd, 90, 52, 4, 4, 1, backends[0]->GetInputs().rt1);
        obdPreciseEllipse(&obd, 82, 58, 4, 4, 1, backends[0]->GetInputs().rt2);

        obdPreciseEllipse(&obd, 82, 46, 4, 4, 1, backends[0]->GetInputs().rt3);
        obdPreciseEllipse(&obd, 90, 40, 4, 4, 1, backends[0]->GetInputs().rt4);
        obdPreciseEllipse(&obd, 98, 46, 4, 4, 1, backends[0]->GetInputs().rt5);
        obdPreciseEllipse(&obd, 93, 27, 4, 4, 1, backends[0]->GetInputs().rf1);
        obdPreciseEllipse(&obd, 102, 23, 4, 4, 1, backends[0]->GetInputs().rf2);
        obdPreciseEllipse(&obd, 112, 24, 4, 4, 1, backends[0]->GetInputs().rf3);
        obdPreciseEllipse(&obd, 122, 29, 4, 4, 1, backends[0]->GetInputs().rf4);

        obdPreciseEllipse(&obd, 93, 17, 4, 4, 1, backends[0]->GetInputs().rf5);
        obdPreciseEllipse(&obd, 103, 13, 4, 4, 1, backends[0]->GetInputs().rf6);
        obdPreciseEllipse(&obd, 113, 14, 4, 4, 1, backends[0]->GetInputs().rf7);
        obdPreciseEllipse(&obd, 122, 19, 4, 4, 1, backends[0]->GetInputs().rf8);
    } else if (rightLayout == "SMASH 64") {
        obdPreciseEllipse(&obd, 82, 46, 4, 4, 1, backends[0]->GetInputs().rt3);
        // obdPreciseEllipse(&obd, 82, 58, 4, 4, 1, backends[0]->GetInputs().c_down);
        // obdPreciseEllipse(&obd, 90, 40, 4, 4, 1, backends[0]->GetInputs().c_up);
        obdPreciseEllipse(&obd, 90, 52, 4, 4, 1, backends[0]->GetInputs().rt1);
        // obdPreciseEllipse(&obd, 98, 46, 4, 4, 1, backends[0]->GetInputs().c_right);
        obdPreciseEllipse(&obd, 93, 17, 4, 4, 1, backends[0]->GetInputs().rf5);
        obdPreciseEllipse(&obd, 93, 27, 4, 4, 1, backends[0]->GetInputs().rf1);
        obdPreciseEllipse(&obd, 103, 13, 4, 4, 1, backends[0]->GetInputs().rf6);
        obdPreciseEllipse(&obd, 102, 23, 4, 4, 1, backends[0]->GetInputs().rf2);
        // obdPreciseEllipse(&obd, 113, 14, 4, 4, 1, backends[0]->GetInputs().lightshield);
        obdPreciseEllipse(&obd, 112, 24, 4, 4, 1, backends[0]->GetInputs().rf3);
        // obdPreciseEllipse(&obd, 122, 19, 4, 4, 1, backends[0]->GetInputs().midshield);
        obdPreciseEllipse(&obd, 122, 29, 4, 4, 1, backends[0]->GetInputs().rf4);
    } else if (rightLayout == "FGC" || rightLayout == "FGC ALT") {
        obdPreciseEllipse(&obd, 82, 46, 4, 4, 1, backends[0]->GetInputs().rt3);
        obdPreciseEllipse(&obd, 90, 40, 4, 4, 1, backends[0]->GetInputs().rt4);
        obdPreciseEllipse(&obd, 90, 52, 4, 4, 1, backends[0]->GetInputs().rt1);
        obdPreciseEllipse(&obd, 93, 17, 4, 4, 1, backends[0]->GetInputs().rf5);
        obdPreciseEllipse(&obd, 93, 27, 4, 4, 1, backends[0]->GetInputs().rf1);
        obdPreciseEllipse(&obd, 103, 13, 4, 4, 1, backends[0]->GetInputs().rf6);
        obdPreciseEllipse(&obd, 102, 23, 4, 4, 1, backends[0]->GetInputs().rf2);
        obdPreciseEllipse(&obd, 113, 14, 4, 4, 1, backends[0]->GetInputs().rf7);
        obdPreciseEllipse(&obd, 112, 24, 4, 4, 1, backends[0]->GetInputs().rf3);
        obdPreciseEllipse(&obd, 122, 19, 4, 4, 1, backends[0]->GetInputs().rf8);
        obdPreciseEllipse(&obd, 122, 29, 4, 4, 1, backends[0]->GetInputs().rf4);
    }
    obdDumpBuffer(&obd, NULL); // Sends buffered content to display.
}
