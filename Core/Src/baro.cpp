#include "baro.h"
// uncomp_temp: 16-bit temperature readout
// Returns: temperature celsius. Two last decimal digits are fraction
// See sec. 3.11.3 of datasheet for more details.
int32_t BMP280::ReadoutTrimming::GetCompensatedTemperature(
    int32_t uncomp_temp) {
  int32_t var1, var2, comp_temp;
  uncomp_temp = uncomp_temp << 4;
  var1 = ((((uncomp_temp >> 3) - ((int32_t)T1_ << 1))) * ((int32_t)T2_)) >> 11;
  var2 = (((((uncomp_temp >> 4) - ((int32_t)T1_)) *
            ((uncomp_temp >> 4) - ((int32_t)T1_))) >>
           12) *
          ((int32_t)T3_)) >>
         14;
  t_fine_ = var1 + var2;
  comp_temp = (t_fine_ * 5 + 128) >> 8;
  return comp_temp;
}

int32_t BMP280::ReadoutTrimming::GetCompensatedPressure(
    int32_t uncomp_pressure) {
  int64_t var1, var2, p;
  uncomp_pressure = uncomp_pressure << 4;
  var1 = ((int64_t)t_fine_) - 128000;
  var2 = var1 * var1 * (int64_t)P6_;
  var2 = var2 + ((var1 * (int64_t)P5_) << 17);
  var2 = var2 + (((int64_t)P4_) << 35);
  var1 = ((var1 * var1 * (int64_t)P3_) >> 8) + ((var1 * (int64_t)P2_) << 12);
  var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)P1_) >> 33;
  if (var1 == 0) {
    return 0;  // avoid exception caused by division by zero
  }
  p = 1048576 - uncomp_pressure;
  p = (((p << 31) - var2) * 3125) / var1;
  var1 = (((int64_t)P9_) * (p >> 13) * (p >> 13)) >> 25;
  var2 = (((int64_t)P8_) * p) >> 19;
  p = ((p + var1 + var2) >> 8) + (((int64_t)P7_) << 4);
  return (int32_t)p;
}

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
  InitTrimmingVariables();
  SetMode(mode_);
  EnablePressureReading();
  EnableTemperatureReading();
}

void Barometer::InitTrimmingVariables() {
  // Define functions that read LSB/MSB pair from registers for trimming.
  // TODO: figure out if these are correct
  readout_compensator_.T1_ = GetUnsignedTrimmingValue(BMP280::kT1LSB);
  readout_compensator_.T2_ = GetTrimmingValue(BMP280::kT2LSB);
  readout_compensator_.T3_ = GetTrimmingValue(BMP280::kT3LSB);
  readout_compensator_.P1_ = GetUnsignedTrimmingValue(BMP280::kP1LSB);
  readout_compensator_.P2_ = GetTrimmingValue(BMP280::kP2LSB);
  readout_compensator_.P3_ = GetTrimmingValue(BMP280::kP3LSB);
  readout_compensator_.P4_ = GetTrimmingValue(BMP280::kP4LSB);
  readout_compensator_.P5_ = GetTrimmingValue(BMP280::kP5LSB);
  readout_compensator_.P6_ = GetTrimmingValue(BMP280::kP6LSB);
  readout_compensator_.P7_ = GetTrimmingValue(BMP280::kP7LSB);
  readout_compensator_.P8_ = GetTrimmingValue(BMP280::kP8LSB);
  readout_compensator_.P9_ = GetTrimmingValue(BMP280::kP9LSB);
  readout_compensator_.t_fine_ = 0;
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

int32_t Barometer::GetPressure() {
  uint8_t pressure[3];
  int32_t result;
  uint8_t length = pressure_oversampling_ != 0b001 ? 3 : 2;
  MultiRead(BMP280::kPressureMSB, pressure, length);
  result = (pressure[0] << 8) + pressure[1];
  // TODO: add support for oversampling
  return readout_compensator_.GetCompensatedPressure(result);
}

int32_t Barometer::GetTemperature() {
  uint8_t temperature[3];
  int32_t result;
  uint8_t length = temperature_oversampling_ != 0b001 ? 3 : 2;
  MultiRead(BMP280::kTempMSB, temperature, length);
  // TODO: add support for oversampling
  result = (temperature[0] << 8) + temperature[1];
  //  return result;
  return readout_compensator_.GetCompensatedTemperature(result);
}

void Barometer::SoftReset() {
  WriteRegister(BMP280::kReset, kSoftResetWord_);
  HAL_Delay(kStartupTime_);
}

int16_t Barometer::GetTrimmingValue(uint8_t lsb_reg) {
  uint8_t temp[2];
  MultiRead(lsb_reg, temp, 2);
  return (temp[1] << 8) + temp[0];
}
uint16_t Barometer::GetUnsignedTrimmingValue(uint8_t lsb_reg) {
  uint8_t temp[2];
  MultiRead(lsb_reg, temp, 2);
  return (temp[1] << 8) + temp[0];
}
