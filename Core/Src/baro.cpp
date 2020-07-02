#include "baro.h"

Barometer::Barometer(I2C_HandleTypeDef *hi2c, uint16_t i2c_address)
    : hi2c_(hi2c), address_(i2c_address) {
  // set filter vals
  //
  SoftReset();
}

void Barometer::WriteRegister(uint8_t reg, uint8_t value) {
  uint8_t test[2] = {reg, value};
  HAL_I2C_Master_Transmit(hi2c_, address_, test, 2, kTimeout_);
  //  HAL_I2C_Master_Transmit(hi2c_, address_, &reg, 1, kTimeout_);
  //  HAL_I2C_Master_Transmit(hi2c_, address_, &value, 1, kTimeout_);
}

uint8_t Barometer::ReadRegister(uint8_t reg) {
  uint8_t result;
  HAL_I2C_Master_Transmit(hi2c_, address_, &reg, 1, kTimeout_);
  HAL_I2C_Master_Receive(hi2c_, address_, &result, 1, kTimeout_);
  return result;
}

void Barometer::EnablePressureReading() {
  uint8_t oversampling;
  uint8_t ctrl_meas;
  WriteRegister(BMP280::kCtrlMeas, 0b00100011);
}

uint32_t Barometer::GetPressure() {
  uint8_t press_msb = ReadRegister(BMP280::kPressureMSB);
  uint8_t press_lsb = ReadRegister(BMP280::kPressureLSB);
  return 0;
}

void Barometer::SoftReset() {
  WriteRegister(BMP280::kReset, kSoftResetWord_);
  HAL_Delay(kStartupTime_);
}
