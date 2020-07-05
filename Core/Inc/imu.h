#ifndef INC_IMU_H_
#define INC_IMU_H_
#include <cstdint>

#include "stm32l4xx_hal.h"

namespace MPU9250 {
typedef enum : uint8_t {
  kSelfTestXGyro = 0x00,
  kSelfTestYGyro = 0x01,
  kSelfTestZGyro = 0x02,
  kSelfTestXAccel = 0x0D,
  kSelfTestYAccel = 0x0E,
  kSelfTestZAccel = 0x0F,
  kConfig = 0x1A,
  kGyroConfig = 0x1B,
  kAccelConfig = 0x1C,
  kAccelConfig2 = 0x1D,
  kAccelXOutMSB = 0x3B,
  kAccelXOutLSB = 0x3C,
  kAccelZOutMSB = 0x3F,
  kAccelZOutLSB = 0x40,
  kWhoAmI = 117,
} Register;

}

enum Axis : uint8_t {
  kX = 0,
  kY = 1,
  kZ = 2,
};

class IMU {
 public:
  IMU(I2C_HandleTypeDef *hi2c, uint16_t i2c_address);
  void WriteRegister(uint8_t reg, uint8_t value);
  uint8_t ReadRegister(uint8_t reg);
  void MultiRead(uint8_t first_reg, uint8_t *data, uint8_t length);
  void UpdateValues();
  void GetAccelZ();

 private:
  I2C_HandleTypeDef *hi2c_;
  uint16_t address_;
  const uint32_t kTimeout_ = 10000;
  // accelerometer variables: scale, frequency etc
  // store variables privately, to decouple update and getting
};

#endif /* INC_IMU_H_ */
