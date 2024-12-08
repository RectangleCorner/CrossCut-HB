#ifndef _COMMS_DINPUTBACKEND_HPP
#define _COMMS_DINPUTBACKEND_HPP

#include "core/CommunicationBackend.hpp"
#include "core/ControllerMode.hpp"
#include "core/InputSource.hpp"
#include "stdlib.hpp"

#include <Joystick.h>

class DInputBackend : public CommunicationBackend {
  public:
    DInputBackend(InputState &inputs, InputSource **input_sources, size_t input_source_count);
    ~DInputBackend();
    CommunicationBackendId BackendId();
    void SendReport(bool isMelee);

  private:
    Joystick_ _joystick;
};

#endif
