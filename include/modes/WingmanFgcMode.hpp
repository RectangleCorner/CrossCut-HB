#ifndef _MODES_WINGMANFGCMODE_HPP
#define _MODES_WINGMANFGCMODE_HPP

#include "core/ControllerMode.hpp"
#include "core/state.hpp"

class WingmanFgcMode : public ControllerMode {
  public:
    WingmanFgcMode();

  private:
    void UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs);
    void UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs);
};

#endif
