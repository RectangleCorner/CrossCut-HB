#ifndef _MODES_MELEENERFS_HPP
#define _MODES_MELEENERFS_HPP

#include "core/ControllerMode.hpp"
#include "core/state.hpp"

#include <config.pb.h>

class MeleeNerfs : public ControllerMode {
  public:
    MeleeNerfs();
    void SetConfig(
        GameModeConfig &config,
        const MeleeOptions options,
        const bool cwos,
        uint32_t modx_x_wd,
        uint32_t modx_y_wd,
        uint32_t mody_x_wd,
        uint32_t mody_y_wd
    );

  protected:
    void UpdateDigitalOutputs(const InputState &inputs, OutputState &outputs);
    void UpdateAnalogOutputs(const InputState &inputs, OutputState &outputs);

  private:
    bool _cwos;
    uint32_t _modx_x_wd;
    uint32_t _modx_y_wd;
    uint32_t _mody_x_wd;
    uint32_t _mody_y_wd;
    bool _horizontal_socd;

    void HandleSocd(InputState &inputs);
};

#endif
