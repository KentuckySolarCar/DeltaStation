/* date = March 31, 2025 8:22 PM */


#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <cstdint>

constexpr uint8_t MSG_LENGTH = 40;
constexpr uint8_t ECC_LENGTH = 12;

constexpr static size_t BUFFER_LENGTH = MSG_LENGTH + ECC_LENGTH;

constexpr int FIRST_BYTE_HEADER = 0;
constexpr int SECOND_BYTE_HEADER = 1;
constexpr int THIRD_BYTE_HEADER = 2;
constexpr int FOURTH_BYTE_HEADER = 3;
constexpr int MESSAGE_TYPE_BYTE = 4;
constexpr int ACTUAL_MESSAGE_LENGTH_OFFSET = 5;
constexpr int TIME_OFFSET = 6;
constexpr int DATA_OFFSET = TIME_OFFSET + sizeof(int);
constexpr int DATA_LENGTH = BUFFER_LENGTH - DATA_OFFSET;

constexpr int UNDEFINED_MESSAGE = 0;
constexpr int LEFT_MOTOR_MESSAGE = 1;
constexpr int RIGHT_MOTOR_MESSAGE = 2;
constexpr int GPS_MESSAGE = 3;
constexpr int MPPT_MESSAGE = 4;
constexpr int BATTERY_MESSAGE = 5;
constexpr int DRIVER_INPUT_MESSAGE = 6;
constexpr int STATUS_MESSAGE = 7;
constexpr int SENSOR_MESSAGE = 8;

#endif //COMMON_H
