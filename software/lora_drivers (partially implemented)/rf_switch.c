#include "rf_switch.h"
#include "stm32wlxx_hal.h"

/**
 * RF Switch Constants
 */
#define RF_SWITCH_CTRL_PIN GPIO_PIN_4
#define RF_SWITCH_CTRL_GPIO_PORT GPIOA
#define RF_SWITCH_CTRL_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#define RF_SWITCH_CTRL_GPIO_CLK_DISABLE() __HAL_RCC_GPIOA_CLK_DISABLE()

/**
 * @brief  Configure antenna switches
 * @param  switch_state: Specifies the antenna switches state.
 *         This parameter can be one of following parameters:
 *           @arg SWITCH_OFF
 *           @arg SWITCH_RX
 *           @arg SWITCH_RFO_LP
 *           @arg SWITCH_RFO_HP
 */
void rfSwitch_setState(rfSwitch_States_t switch_state)
{
  switch (switch_state)
  {
  case SWITCH_OFF:
  {
    /* Turn off switch */
    HAL_GPIO_WritePin(RF_SWITCH_CTRL_GPIO_PORT, RF_SWITCH_CTRL_PIN, GPIO_PIN_RESET);
    break;
  }
  case SWITCH_RX:
  {
    /* Turns On in Rx Mode the RF Switch */
    HAL_GPIO_WritePin(RF_SWITCH_CTRL_GPIO_PORT, RF_SWITCH_CTRL_PIN, GPIO_PIN_RESET);
    break;
  }
  case SWITCH_RFO_LP:
  {
    /* Turns On in Tx Low Power the RF Switch */
    /* NOT SUPPORTED */
    break;
  }
  case SWITCH_RFO_HP:
  {
    /* Turns On in Tx High Power the RF Switch */
    HAL_GPIO_WritePin(RF_SWITCH_CTRL_GPIO_PORT, RF_SWITCH_CTRL_PIN, GPIO_PIN_SET);
    break;
  }
  default:
    break;
  }
}

/*!
 * \brief TODO
 */
void rfSwitch_init(void)
{
  GPIO_InitTypeDef gpio_init_structure = {0};

  /* Enable the Radio Switch Clock */
  RF_SWITCH_CTRL_GPIO_CLK_ENABLE();

  /* Configure the Radio Switch pin */
  gpio_init_structure.Pin = RF_SWITCH_CTRL_PIN;
  gpio_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

  HAL_GPIO_Init(RF_SWITCH_CTRL_GPIO_PORT, &gpio_init_structure);

  // Switch off by default
  rfSwitch_setState(SWITCH_OFF);
}
