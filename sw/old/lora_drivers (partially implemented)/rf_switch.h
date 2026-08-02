#ifndef HAL_RF_SWITCH_H_
#define HAL_RF_SWITCH_H_

/*!
 * \brief Declare the available states of the RF switch
 */
typedef enum
{
  SWITCH_OFF = 0,
  SWITCH_RX = 1,
  SWITCH_RFO_LP = 2,
  SWITCH_RFO_HP = 3,
} rfSwitch_States_t;

void rfSwitch_init(void);
void rfSwitch_setState(rfSwitch_States_t switch_state);

#endif /* HAL_RF_SWITCH_H_ */
