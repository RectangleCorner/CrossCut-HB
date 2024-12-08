#ifndef _MODES_HAXLIMITS_HPP
#define _MODES_HAXLIMITS_HPP

// #include "core/socd.hpp"
#include "core/state.hpp"

// override the socd assigned, just for melee configurations
// #define MELEE_SOCD socd::SOCD_NEUTRAL

// #define MELEE_SOCD socd::SOCD_2IP_NO_REAC

void limitOutputsHax(
    const uint16_t sampleSpacing,
    const abtest whichAB,
    const InputState &inputs,
    const OutputState &rawOutput,
    OutputState &finalOutput
);

#endif