/*******************************************************************************
 * @brief SysTick-based application tick source.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include "app.h"

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void SysTick_Handler(void)
{
  app_onTick();
}
