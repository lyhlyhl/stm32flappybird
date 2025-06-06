/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flappybird.h
  * @brief   Header for flappybird.c module.
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
#ifndef __FLAPPYBIRD_H
#define __FLAPPYBIRD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ili9341.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
// 黑白颜色定义（bit 表示）
#define Buffer_COLOR_BLACK 0
#define Buffer_COLOR_WHITE 1
#define FRAMEBUFFER_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT + 7) / 8)


void InitGame(void);
void Task_BirdControl(void *pvParameters);
void Task_Display(void *pvParameters);
void Task_ColumnMove(void *pvParameters);
void Task_CollisionCheck(void *pvParameters);

void lcdRefresh(uint8_t *buffer);
void swapBuffers(void);
void frameBuffer_fillRect(uint8_t *buffer,uint16_t x, uint16_t y, uint16_t size, uint8_t color);
void frameBuffer_DrawBird(uint8_t *buffer,uint16_t x, uint16_t y, uint8_t color);
void frameBuffer_fill(uint8_t *buffer,uint8_t color);
void frameBuffer_DrawColumn(uint8_t *buffer);
void frameBuffer_fillRectWH(uint8_t *buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
/* USER CODE END EM */
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __FLAPPYBIRD_H */
