#include "../include/sensors.h"

// this is needed because we treat the interrupt in a static function
Pluviometric_Sensor * Pluviometric_Sensor::_instance = 0;