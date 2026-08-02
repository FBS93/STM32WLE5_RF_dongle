#ifndef COMMAND_MANAGER_H_
#define COMMAND_MANAGER_H_

#include <stdint.h>

#define COMMAND_MANAGER_MAX_PAYLOAD_SIZE ((uint8_t)255)

typedef enum
{
  SET_CONFIG = 0xF0,
  SEND_DATA = 0xF1,
  COMMAND_ERROR = 0xFF, /* Reserved command to notify HostToTargetOutput of a reception error */
} commandManager_hostToTargetCommands_t;

typedef enum
{
  DATA_RECEIVED = 0xE0,
  RADIO_ERROR = 0xE1,
  ACK = 0xEE,
  NACK = 0xEF,
} commandManager_targetToHostCommands_t;

typedef void (*commandManager_hostToTargetOutputCallback_t)(commandManager_hostToTargetCommands_t e_command, uint8_t u8_len, uint8_t *pu8_data);
typedef void (*commandManager_hostToTargetInputCallback_t)(uint16_t *pu16_len, uint8_t *pu8_data);
typedef void (*commandManager_targetToHostOutputCallback_t)(uint16_t u16_len, uint8_t *pu8_data);

void commandManager_init(commandManager_hostToTargetOutputCallback_t host_to_target_output_cb,
                         commandManager_hostToTargetInputCallback_t host_to_target_input_cb,
                         commandManager_targetToHostOutputCallback_t target_to_host_output_cb);
void commandManager_executePeriodicTask(void);
void commandManager_targetToHostInput(commandManager_targetToHostCommands_t e_command, uint8_t u8_len, uint8_t *pu8_data);

#endif /* COMMAND_MANAGER_H_ */
