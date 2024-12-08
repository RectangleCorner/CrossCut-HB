#include "modes/Rivals2.hpp"

#define ANALOG_STICK_MIN 0
#define ANALOG_STICK_NEUTRAL 128
#define ANALOG_STICK_MAX 255

Rivals2::Rivals2() : ControllerMode() {}

void Rivals2::UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs) {
    outputs.a = inputs.rt1;
    outputs.b = inputs.rf1;
    outputs.x = inputs.rf2;
    outputs.y = inputs.rf6;
    outputs.buttonR = inputs.rf3;
    if (inputs.nunchuk_connected) {
        // Lightshield with C button.
        if (inputs.nunchuk_c) {
            outputs.triggerLAnalog = 49;
        }
        outputs.triggerLDigital = inputs.nunchuk_z;
    } else {
        outputs.triggerLDigital = inputs.lf4;
    }
    outputs.triggerRDigital = inputs.rf5;

    // function layer
    if (inputs.lt2) {
        outputs.home = inputs.mb1;
    } else if (inputs.lt1) {
        outputs.select = inputs.mb1;
    } else {
        outputs.start = inputs.mb1;
    }

    outputs.leftStickClick = inputs.rf7;
    outputs.rightStickClick = inputs.rf8;
    outputs.buttonL = inputs.rf9; // LB available to be bound
    outputs.dpadLeft = inputs.mb3;
    outputs.dpadRight = inputs.mb4;
    // Activate D-Pad layer by holding Mod X + Mod Y.
    if ((inputs.lt1 && inputs.lt2) || inputs.mb2) {
        outputs.dpadUp = inputs.rt4;
        outputs.dpadDown = inputs.rt2;
        outputs.dpadLeft = inputs.rt3;
        outputs.dpadRight = inputs.rt5;
    } else { // dpad can be bound
        outputs.dpadUp = inputs.lt3;
        outputs.dpadDown = inputs.lt4;
        outputs.dpadLeft = inputs.lt5;
        outputs.dpadRight = inputs.lt6;
    }
}

void Rivals2::UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs) {
    // Coordinate calculations to make modifier handling simpler.
    UpdateDirections(
        inputs.lf3, // Left
        inputs.lf1, // Right
        inputs.lf2, // Down
        inputs.rf4, // Up
        inputs.rt3, // C-Left
        inputs.rt5, // C-Right
        inputs.rt2, // C-Down
        inputs.rt4, // C-Up
        ANALOG_STICK_MIN,
        ANALOG_STICK_NEUTRAL,
        ANALOG_STICK_MAX,
        outputs
    );

    bool shield_button_pressed = inputs.lf4 || inputs.rf5;
    if (directions.diagonal) {
        if (directions.y == -1 && (shield_button_pressed)) {
            outputs.leftStickX = 128 + (directions.x * 99);
            outputs.leftStickY = 128 + (directions.y * 71);
        } else {
            outputs.leftStickX = 128 + (directions.x * 127);
            outputs.leftStickY = 128 + (directions.y * 127);
        }
    }
    // 48 total DI angles, 24 total Up b angles, 16 total airdodge angles

    if (inputs.lt1) {
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 88);
        }

        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 71);
        }

        /* Extra DI, Air Dodge, and Up B angles */
        if (directions.diagonal &&
            ((!inputs.rf1 && !inputs.rf3) || (inputs.rf1 && inputs.rf3))) { // 80% mag
            outputs.leftStickX = 128 + (directions.x * 74);
            outputs.leftStickY = 128 + (directions.y * 47);

            if (inputs.rt1) { // angled ftilt
                outputs.leftStickX = 128 + (directions.x * 60);
                outputs.leftStickY = 128 + (directions.y * 53);
            }

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 71);
                outputs.leftStickY = 128 + (directions.y * 49);
            }

            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 68);
                outputs.leftStickY = 128 + (directions.y * 51);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 65);
                outputs.leftStickY = 128 + (directions.y * 54);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 61);
                outputs.leftStickY = 128 + (directions.y * 57);
            }
        } else if (directions.diagonal && inputs.rf1 && !inputs.rf3) { // 100% mag
            outputs.leftStickX = 128 + (directions.x * 92);
            outputs.leftStickY = 128 + (directions.y * 58);

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 89);
                outputs.leftStickY = 128 + (directions.y * 61);
            }

            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 85);
                outputs.leftStickY = 128 + (directions.y * 64);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 81);
                outputs.leftStickY = 128 + (directions.y * 67);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 76);
                outputs.leftStickY = 128 + (directions.y * 71);
            }
        } else if (directions.diagonal && !inputs.rf1 && inputs.rf3) { // 60% mag
            outputs.leftStickX = 128 + (directions.x * 55);
            outputs.leftStickY = 128 + (directions.y * 34);

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 53);
                outputs.leftStickY = 128 + (directions.y * 37);
            }
            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 51);
                outputs.leftStickY = 128 + (directions.y * 38);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 48);
                outputs.leftStickY = 128 + (directions.y * 40);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 46);
                outputs.leftStickY = 128 + (directions.y * 43);
            }
        }
    }

    if (inputs.lt2) {
        if (directions.horizontal) {
            outputs.leftStickX = 128 + (directions.x * 75);
        }

        if (directions.vertical) {
            outputs.leftStickY = 128 + (directions.y * 98);
        }

        /* Extra DI, Air Dodge, and Up B angles */
        if (directions.diagonal &&
            ((!inputs.rf1 && !inputs.rf3) || (inputs.rf1 && inputs.rf3))) { // 80% mag
            outputs.leftStickX = 128 + (directions.x * 47);
            outputs.leftStickY = 128 + (directions.y * 74);

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 49);
                outputs.leftStickY = 128 + (directions.y * 71);
            }

            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 51);
                outputs.leftStickY = 128 + (directions.y * 68);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 54);
                outputs.leftStickY = 128 + (directions.y * 65);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 57);
                outputs.leftStickY = 128 + (directions.y * 61);
            }
        } else if (directions.diagonal && inputs.rf1 && !inputs.rf3) { // 100% mag
            outputs.leftStickX = 128 + (directions.x * 58);
            outputs.leftStickY = 128 + (directions.y * 92);

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 61);
                outputs.leftStickY = 128 + (directions.y * 89);
            }

            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 64);
                outputs.leftStickY = 128 + (directions.y * 85);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 67);
                outputs.leftStickY = 128 + (directions.y * 81);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 71);
                outputs.leftStickY = 128 + (directions.y * 76);
            }
        } else if (directions.diagonal && !inputs.rf1 && inputs.rf3) { // 60% mag
            outputs.leftStickX = 128 + (directions.x * 35);
            outputs.leftStickY = 128 + (directions.y * 55);

            // Angles just for DI and Up B
            if (inputs.rt2) {
                outputs.leftStickX = 128 + (directions.x * 37);
                outputs.leftStickY = 128 + (directions.y * 53);
            }

            // Angles just for DI
            if (inputs.rt3) {
                outputs.leftStickX = 128 + (directions.x * 38);
                outputs.leftStickY = 128 + (directions.y * 51);
            }

            if (inputs.rt4) {
                outputs.leftStickX = 128 + (directions.x * 40);
                outputs.leftStickY = 128 + (directions.y * 48);
            }

            if (inputs.rt5) {
                outputs.leftStickX = 128 + (directions.x * 43);
                outputs.leftStickY = 128 + (directions.y * 46);
            }
        }
    }

    // Shut off C-stick when using D-Pad layer.
    if ((inputs.lt1 && inputs.lt2) || inputs.mb2) {
        outputs.rightStickX = 128;
        outputs.rightStickY = 128;
    }

    // Nunchuk overrides left stick.
    if (inputs.nunchuk_connected) {
        outputs.leftStickX = inputs.nunchuk_x;
        outputs.leftStickY = inputs.nunchuk_y;
    }
}
