#include "baro.h"

int32_t BMP280::TemperatureTrimming::GetCompensatedTemperature(
    int32_t uncompensated_temperature) {}
Barometer::Barometer(I2C_HandleTypeDef *hi2c, uint16_t i2c_address)
    : hi2c_(hi2c), address_(i2c_address) {}

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

void Barometer::MultiRead(uint8_t first_reg, uint8_t *data, uint8_t length) {
  HAL_I2C_Master_Transmit(hi2c_, address_, &first_reg, 1, kTimeout_);
  HAL_I2C_Master_Receive(hi2c_, address_, data, length, kTimeout_);
}

void Barometer::SetMode(BMP280::PowerMode new_mode) {
  mode_ = new_mode;  // TODO: make fail safe in case of communication failure.
  uint8_t ctrl_meas = ReadRegister(BMP280::kCtrlMeas);
  ctrl_meas &= ~(0b11);  //  Clear mode bits.
  ctrl_meas |= mode_;
  WriteRegister(BMP280::kCtrlMeas, ctrl_meas);
}

void Barometer::Init() {
  SoftReset();
  InitTemperatureTrimming();
  SetMode(mode_);
  EnablePressureReading();
  EnableTemperatureReading();
}

void Barometer::InitTemperatureTrimming() {
  // Define functions that read LSB/MSB pair from registers for trimming.
}

void Barometer::EnablePressureReading() {
  uint8_t power_mode = ReadRegister(BMP280::kCtrlMeas);
  power_mode = power_mode | (pressure_oversampling_ << 2);
  WriteRegister(BMP280::kCtrlMeas, power_mode);
}

void Barometer::EnableTemperatureReading() {
  uint8_t current_mode = ReadRegister(BMP280::kCtrlMeas);
  current_mode = current_mode | (temperature_oversampling_ << 5);
  WriteRegister(BMP280::kCtrlMeas, current_mode);
}

uint32_t Barometer::GetPressure() {
  uint8_t pressure[3];
  uint32_t result;
  uint8_t length = pressure_oversampling_ != 0b001 ? 3 : 2;
  MultiRead(BMP280::kPressureMSB, pressure, length);
  result = (pressure[0] << 8) + pressure[1];
  // TODO: add support for oversampling
  return result;
}

uint32_t Barometer::GetTemperature() {
  uint8_t temperature[3];
  uint32_t result;
  uint8_t length = temperature_oversampling_ != 0b001 ? 3 : 2;
  MultiRead(BMP280::kTempMSB, temperature, length);
  // TODO: add support for oversampling
  result = (temperature[0] << 8) + temperature[1];
  return result;
}

void Barometer::SoftReset() {
  WriteRegister(BMP280::kReset, kSoftResetWord_);
  HAL_Delay(kStartupTime_);
}
