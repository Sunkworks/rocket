#ifndef BARO_H_
#define BARO_H_

#include <cstdint>
#include "stm32l4xx_hal.h"

namespace BMP280 {

typedef enum : uint8_t {
  kT1LSB = 0x88,
  kT2LSB = 0x8B,
  KT3LSB = 0x8C,
  kID = 0xD0,
  kReset = 0xE0,
  kStatus = 0xF3,
  kCtrlMeas = 0xF4,
  kConfig = 0xF5,
  kPressureMSB = 0xF7,
  kPressureLSB = 0xF8,
  kPressureXLSB = 0xF9,
  kTempMSB = 0xFA,
} Register;

typedef enum : uint8_t {
  kSleepMode = 0,
  kForcedMode = 0b01,
  kNormalMode = 0b11,
} PowerMode;

struct TemperatureTrimming {
  uint16_t T1;
  int16_t T2;
  int16_t T3;
  int32_t GetCompensatedTemperature(int32_t uncompensated_temperature);
};

}  // namespace BMP280

class Barometer {
 public:
  Barometer(I2C_HandleTypeDef *hi2c, uint16_t i2c_address);
  void WriteRegister(uint8_t reg, uint8_t value);
  uint8_t ReadRegister(uint8_t reg);
  void MultiRead(uint8_t first_reg, uint8_t *data, uint8_t length);
  void Init();
  void InitTemperatureTrimming();
  void SetMode(BMP280::PowerMode new_mode);
  void EnablePressureReading();
  void EnableTemperatureReading();
  uint32_t GetPressure();
  uint32_t GetTemperature();
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
  struct BMP280::TemperatureTrimming temp_compensator;
};

#endif /* BARO_H_ */
