/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CurrentBit1_Pin GPIO_PIN_0
#define CurrentBit1_GPIO_Port GPIOA
#define CurrentBit2_Pin GPIO_PIN_1
#define CurrentBit2_GPIO_Port GPIOA
#define CurrentBit3_Pin GPIO_PIN_2
#define CurrentBit3_GPIO_Port GPIOA
#define CurrentBit4_Pin GPIO_PIN_3
#define CurrentBit4_GPIO_Port GPIOA
#define StepperStep_Pin GPIO_PIN_0
#define StepperStep_GPIO_Port GPIOB
#define StepperDir_Pin GPIO_PIN_1
#define StepperDir_GPIO_Port GPIOB
#define StepperEn_Pin GPIO_PIN_2
#define StepperEn_GPIO_Port GPIOB
#define RevBtn_Pin GPIO_PIN_10
#define RevBtn_GPIO_Port GPIOB
#define StopBtn_Pin GPIO_PIN_11
#define StopBtn_GPIO_Port GPIOB
#define FwdBtn_Pin GPIO_PIN_12
#define FwdBtn_GPIO_Port GPIOB
#define EncBtn_Pin GPIO_PIN_13
#define EncBtn_GPIO_Port GPIOB
#define EncB_Pin GPIO_PIN_14
#define EncB_GPIO_Port GPIOB
#define StepperSleep_Pin GPIO_PIN_3
#define StepperSleep_GPIO_Port GPIOB
#define StepperRst_Pin GPIO_PIN_4
#define StepperRst_GPIO_Port GPIOB
#define LCDcs_Pin GPIO_PIN_5
#define LCDcs_GPIO_Port GPIOB
#define LCDrst_Pin GPIO_PIN_6
#define LCDrst_GPIO_Port GPIOB
#define LCDa0_Pin GPIO_PIN_7
#define LCDa0_GPIO_Port GPIOB
#define CurrBtn_Pin GPIO_PIN_8
#define CurrBtn_GPIO_Port GPIOB
#define ModeBtn_Pin GPIO_PIN_9
#define ModeBtn_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
