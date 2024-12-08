#ifndef _COMMS_GAMECUBEBACKEND_HPP
#define _COMMS_GAMECUBEBACKEND_HPP

#include "core/CommunicationBackend.hpp"

#include <GamecubeConsole.hpp>
#include <hardware/pio.h>

class GamecubeBackend : public CommunicationBackend {
  public:
    GamecubeBackend(
        InputState &inputs,
        InputSource **input_sources,
        size_t input_source_count,
        uint data_pin,
        uint rumble_pin,
        bool rumble_option,
        PIO pio = pio0,
        int sm = -1,
        int offset = -1
    );
    CommunicationBackendId BackendId();
    void SendReport(bool isMelee, bool travelTime);
    int GetOffset();

  private:
    GamecubeConsole _gamecube;
    gc_report_t _report;
    uint _rumble_pin;
    bool _rumble_option;
};

#endif
