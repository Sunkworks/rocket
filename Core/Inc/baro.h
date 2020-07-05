#ifndef BARO_H_
#define BARO_H_

#include <cstdint>
#include "stm32l4xx_hal.h"

namespace BMP280 {

typedef enum : uint8_t {
  kT1LSB = 0x88,
  kT2LSB = 0x8A,
  kT3LSB = 0x8C,
  kP1LSB = 0x8E,
  kP2LSB = 0x90,
  kP3LSB = 0x92,
  kP4LSB = 0x94,
  kP5LSB = 0x96,
  kP6LSB = 0x98,
  kP7LSB = 0x9A,
  kP8LSB = 0x9C,
  kP9LSB = 0x9E,
  kID = 0xD0,
  kReset = 0xE0,
  kStatus = 0xF3,
  kCtrlMeas = 0xF4,
  kConfig = 0xF5,
  kPressureMSB = 0xF7,
  kPressureLSB = 0xF8,
  kPressureXLSB = 0xF9,
  kTempMSB = 0xFA,
  kTempLSB = 0xFB,
  kTempXLSB = 0xFC,
} Register;

typedef enum : uint8_t {
  kSleepMode = 0,
  kForcedMode = 0b01,
  kNormalMode = 0b11,
} PowerMode;

struct ReadoutTrimming {
  uint16_t T1_;
  int16_t T2_;
  int16_t T3_;
  uint16_t P1_;
  int16_t P2_;
  int16_t P3_;
  int16_t P4_;
  int16_t P5_;
  int16_t P6_;
  int16_t P7_;
  int16_t P8_;
  int16_t P9_;
  int32_t GetCompensatedTemperature(int32_t uncomp_temp);
  int32_t GetCompensatedPressure(int32_t uncomp_pressure);
  int32_t t_fine_;
};

}  // namespace BMP280

class Barometer {
 public:
  Barometer(I2C_HandleTypeDef *hi2c, uint16_t i2c_address);
  void WriteRegister(uint8_t reg, uint8_t value);
  uint8_t ReadRegister(uint8_t reg);
  void MultiRead(uint8_t first_reg, uint8_t *data, uint8_t length);
  void Init();
  void InitTrimmingVariables();
  void SetMode(BMP280::PowerMode new_mode);
  void EnablePressureReading();
  void EnableTemperatureReading();
  void UpdateValues();
  int32_t GetPressure();
  int32_t GetTemperature();
  void SoftReset();

 private:
  I2C_HandleTypeDef *hi2c_;
  uint16_t address_;
  const uint32_t kTimeout_ = 10000;
  const uint8_t kSoftResetWord_ = 0xB6;
  const uint8_t kStartupTime_ =
      2;  //  milliseconds, Table 2 (section 1) of datasheet.
  uint8_t pressure_oversampling_ = 0b001;
  uint8_t temperature_oversampling_ = 0b001;
  BMP280::PowerMode mode_ = BMP280::kNormalMode;
  struct BMP280::ReadoutTrimming readout_compensator_;
  int16_t GetTrimmingValue(uint8_t lsb_reg);
  uint16_t GetUnsignedTrimmingValue(uint8_t lsb_reg);
  int32_t pressure_, temperature_;
};

#endif /* BARO_H_ */
