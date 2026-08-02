#include <stdint.h>
#include <stddef.h>

#include "command_manager.h"

/* Frame format
 *
 * | Start of frame = COMMAND_MANAGER_START_OF_FRAME | Command (1 Byte) |Payload length (1 Byte) | Payload | Checksum (1 Byte) |
 *
 */

#define COMMAND_MANAGER_START_OF_FRAME ((uint8_t)0xA5)

typedef enum
{
  WAITING_FOR_START_OF_FRAME = 0x00,
  WAITING_FOR_COMMAND,
  WAITING_FOR_PAYLOAD_LEN,
  WAITING_FOR_PAYLOAD,
  WAITING_FOR_CHECKSUM,
} commandManager_hostToTargetCommandDecoderStateMachineStates_t;

typedef struct
{
  commandManager_hostToTargetCommandDecoderStateMachineStates_t state;
  commandManager_hostToTargetCommands_t command;
  uint8_t payload_len;
  uint8_t payload_counter;
  uint8_t payload[COMMAND_MANAGER_MAX_PAYLOAD_SIZE];
} commandManager_hostToTargetCommandDecoderStateMachineInfo_t;

typedef struct
{
  commandManager_hostToTargetOutputCallback_t host_to_target_output_cb;
  commandManager_hostToTargetInputCallback_t host_to_target_input_cb;
  commandManager_targetToHostOutputCallback_t target_to_host_output_cb;
} commandManager_callbacks_t;

static commandManager_hostToTargetCommandDecoderStateMachineInfo_t s_commandManager_stateMachineInfo;

static commandManager_callbacks_t s_commandManager_callbacks;

// simple state machine with only one event (Byte received)
static void commandManager_hostToTargetCommandDecoderStateMachine(uint8_t data)
{
  uint8_t checksum;

  switch (s_commandManager_stateMachineInfo.state)
  {
  case WAITING_FOR_START_OF_FRAME:

    if (data == COMMAND_MANAGER_START_OF_FRAME)
    {
      s_commandManager_stateMachineInfo.state = WAITING_FOR_COMMAND;
    }
    else
    {
      s_commandManager_callbacks.host_to_target_output_cb(COMMAND_ERROR, 0, NULL);
    }

    break;

  case WAITING_FOR_COMMAND:

    if (((commandManager_hostToTargetCommands_t)data) == SET_CONFIG ||
        ((commandManager_hostToTargetCommands_t)data) == SEND_DATA)
    {
      s_commandManager_stateMachineInfo.command = (commandManager_hostToTargetCommands_t)data;
      s_commandManager_stateMachineInfo.state = WAITING_FOR_PAYLOAD_LEN;
    }
    else
    {
      s_commandManager_callbacks.host_to_target_output_cb(COMMAND_ERROR, 0, NULL);
      s_commandManager_stateMachineInfo.state = WAITING_FOR_START_OF_FRAME;
    }

    break;

  case WAITING_FOR_PAYLOAD_LEN:
    s_commandManager_stateMachineInfo.payload_len = data;

    if (s_commandManager_stateMachineInfo.payload_len > 0)
    {
      s_commandManager_stateMachineInfo.payload_counter = 0;
      s_commandManager_stateMachineInfo.state = WAITING_FOR_PAYLOAD;
    }
    else
    {
      s_commandManager_stateMachineInfo.state = WAITING_FOR_CHECKSUM;
    }

    break;

  case WAITING_FOR_PAYLOAD:

    if (s_commandManager_stateMachineInfo.payload_counter <
        (s_commandManager_stateMachineInfo.payload_len - 1))
    {
      s_commandManager_stateMachineInfo.payload[s_commandManager_stateMachineInfo.payload_counter] = data;
      s_commandManager_stateMachineInfo.payload_counter++;
    }
    else
    {
      s_commandManager_stateMachineInfo.payload[s_commandManager_stateMachineInfo.payload_counter] = data;
      s_commandManager_stateMachineInfo.state = WAITING_FOR_CHECKSUM;
    }

    break;

  case WAITING_FOR_CHECKSUM:
    checksum = COMMAND_MANAGER_START_OF_FRAME +
               ((uint8_t)s_commandManager_stateMachineInfo.command) +
               s_commandManager_stateMachineInfo.payload_len;

    for (uint8_t u8_i = 0; u8_i < s_commandManager_stateMachineInfo.payload_len; u8_i++)
    {
      checksum += s_commandManager_stateMachineInfo.payload[u8_i];
    }

    if (checksum == data)
    {
      s_commandManager_callbacks.host_to_target_output_cb(s_commandManager_stateMachineInfo.command,
                                                          s_commandManager_stateMachineInfo.payload_len,
                                                          s_commandManager_stateMachineInfo.payload);
    }
    else
    {
      s_commandManager_callbacks.host_to_target_output_cb(COMMAND_ERROR, 0, NULL);
    }

    s_commandManager_stateMachineInfo.state = WAITING_FOR_START_OF_FRAME;
    break;

  default:
    break;
  }
}

void commandManager_init(commandManager_hostToTargetOutputCallback_t host_to_target_output_cb,
                         commandManager_hostToTargetInputCallback_t host_to_target_input_cb,
                         commandManager_targetToHostOutputCallback_t target_to_host_output_cb)
{

  s_commandManager_callbacks.host_to_target_output_cb = host_to_target_output_cb;
  s_commandManager_callbacks.host_to_target_input_cb = host_to_target_input_cb;
  s_commandManager_callbacks.target_to_host_output_cb = target_to_host_output_cb;

  s_commandManager_stateMachineInfo.state = WAITING_FOR_START_OF_FRAME;
}

void commandManager_executePeriodicTask(void)
{
  uint8_t pu8_data[1024];
  uint16_t u16_len;

  s_commandManager_callbacks.host_to_target_input_cb(&u16_len, pu8_data);

  for (uint16_t u16_i = 0; u16_i < u16_len; u16_i++)
  {
    commandManager_hostToTargetCommandDecoderStateMachine(pu8_data[u16_i]);
  }
}

void commandManager_targetToHostInput(commandManager_targetToHostCommands_t e_command, uint8_t u8_len, uint8_t *pu8_data)
{
  /* size of pu8_frame --> Start of frame + Command + Payload length + COMMAND_MANAGER_MAX_PAYLOAD_SIZE + Checksum */
  uint8_t pu8_frame[4 + COMMAND_MANAGER_MAX_PAYLOAD_SIZE];
  uint16_t u16_frame_index;
  uint8_t checksum;

  pu8_frame[0] = COMMAND_MANAGER_START_OF_FRAME; // Start of frame
  pu8_frame[1] = e_command;                      // Command
  pu8_frame[2] = u8_len;                         // Payload length

  checksum = pu8_frame[0] + pu8_frame[1] + pu8_frame[2];
  u16_frame_index = 3;

  /* Copy payload */
  for (uint8_t u8_i = 0; u8_i < u8_len; u8_i++)
  {
    pu8_frame[u16_frame_index] = pu8_data[u8_i];
    checksum += pu8_frame[u16_frame_index];
    u16_frame_index++;
  }

  pu8_frame[u16_frame_index] = checksum; // Checksum

  /* Send frame through target_to_host_output_callback */
  s_commandManager_callbacks.target_to_host_output_cb(u16_frame_index + 1, pu8_frame);
}
