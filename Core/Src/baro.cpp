#include "baro.h"
// uncomp_temp: 16-bit temperature readout
// Returns: temperature celsius. Two last decimal digits are fraction
// See sec. 3.11.3 of datasheet for more details.
int32_t BMP280::TemperatureTrimming::GetCompensatedTemperature(
    int32_t uncomp_temp) {
  int32_t var1, var2, comp_temp, t_fine;
  uncomp_temp = uncomp_temp << 4;
  var1 = ((((uncomp_temp >> 3) - ((int32_t)T1_ << 1))) * ((int32_t)T2_)) >> 11;
  var2 = (((((uncomp_temp >> 4) - ((int32_t)T1_)) *
            ((uncomp_temp >> 4) - ((int32_t)T1_))) >>
           12) *
          ((int32_t)T3_)) >>
         14;
  t_fine = var1 + var2;
  comp_temp = (t_fine * 5 + 128) >> 8;
  return comp_temp;
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
  InitTemperatureTrimming();
  SetMode(mode_);
  EnablePressureReading();
  EnableTemperatureReading();
}

void Barometer::InitTemperatureTrimming() {
  // Define functions that read LSB/MSB pair from registers for trimming.
  // TODO: figure out if these are correct
  temp_compensator_.T1_ = GetUnsignedTrimmingValue(BMP280::kT1LSB);
  temp_compensator_.T2_ = GetTrimmingValue(BMP280::kT2LSB);
  temp_compensator_.T3_ = GetTrimmingValue(BMP280::kT3LSB);
  auto x = GetUnsignedTrimmingValue(BMP280::kP1LSB);
  auto y = GetTrimmingValue(BMP280::kP8LSB);
  ;
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

int32_t Barometer::GetTemperature() {
  uint8_t temperature[3];
  int32_t result;
  uint8_t length = temperature_oversampling_ != 0b001 ? 3 : 2;
  MultiRead(BMP280::kTempMSB, temperature, length);
  // TODO: add support for oversampling
  result = (temperature[0] << 8) + temperature[1];
  //  return result;
  return temp_compensator_.GetCompensatedTemperature(result);
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
