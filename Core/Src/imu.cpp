#include "imu.h"

void Vec::SetFromSensorReadout(uint8_t *data) {
  x_ = SplitBytesToInt16(data);
  y_ = SplitBytesToInt16(&data[2]);
  z_ = SplitBytesToInt16(&data[4]);
}

int16_t SplitBytesToInt16(uint8_t *data) { return (data[0] << 8) + data[1]; }

IMU::IMU(I2C_HandleTypeDef *hi2c, uint16_t i2c_address)
    : hi2c_(hi2c), address_(i2c_address) {
  // Power modes.
}

void IMU::WriteRegister(uint8_t reg, uint8_t value) {
  uint8_t test[2] = {reg, value};
  HAL_I2C_Master_Transmit(hi2c_, address_, test, 2, kTimeout_);
}

uint8_t IMU::ReadRegister(uint8_t reg) {
  uint8_t result;
  HAL_I2C_Master_Transmit(hi2c_, address_, &reg, 1, kTimeout_);
  HAL_I2C_Master_Receive(hi2c_, address_, &result, 1, kTimeout_);
  return result;
}

void IMU::MultiRead(uint8_t first_reg, uint8_t *data, uint8_t length) {
  HAL_I2C_Master_Transmit(hi2c_, address_, &first_reg, 1, kTimeout_);
  HAL_I2C_Master_Receive(hi2c_, address_, data, length, kTimeout_);
}

void IMU::UpdateValues() {
  // Update internal variables
  uint8_t temp[6];
  MultiRead(MPU9250::kAccelXOutMSB, temp, 6);
  accel_.SetFromSensorReadout(temp);
  MultiRead(MPU9250::kGyroXOutMSB, temp, 6);
  gyro_.SetFromSensorReadout(temp);
  ;
}

struct Vec IMU::GetAcceleration() {
  return accel_;
}
struct Vec IMU::GetRotation() {
  return gyro_;
}
