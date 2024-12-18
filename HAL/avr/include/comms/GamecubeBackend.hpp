#ifndef _COMMS_GAMECUBEBACKEND_HPP
#define _COMMS_GAMECUBEBACKEND_HPP

#include "core/CommunicationBackend.hpp"
#include "core/state.hpp"

#include <Nintendo.h>

class GamecubeBackend : public CommunicationBackend {
  public:
    GamecubeBackend(
        InputState &inputs,
        InputSource **input_sources,
        size_t input_source_count,
        int polling_rate,
        int data_pin
    );
    CommunicationBackendId BackendId();
    void SendReport(bool isMelee);

  private:
    CGamecubeConsole _gamecube;
    Gamecube_Data_t _data;
    int _delay;
};

#endif
