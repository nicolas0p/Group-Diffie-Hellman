// EPOS Cortex Smart Plug Mediator Implementation

#include <system/config.h>

#ifdef __SMART_PLUG_H

#include <machine/cortex/smart_plug.h>

__BEGIN_SYS

// Class attributes
Smart_Plug::Observed Smart_Plug::_observed;
Power_Meter * Smart_Plug::_power_meter[2];
Smart_Plug::Actuator0 * Smart_Plug::_actuator0;
Smart_Plug::Actuator1 * Smart_Plug::_actuator1;

// Methods

__END_SYS

#endif
