#include "flappybird.h"
#include <stdlib.h>

// 鸟的状态
typedef struct {
    int16_t y;        // 鸟的 Y 坐标
    int16_t velocity; // 当前速度（用于模拟重力）
} Bird_t;

// 柱子状态
typedef struct {
    int16_t y;       // X 坐标
    int16_t gapTop;  // 上方柱子底部坐标
    int16_t gapBot;  // 下方柱子顶部坐标
    uint8_t passed;  // 是否已经通过
} Column_t;
Bird_t bird;
Column_t columns[3]; // 最多同时显示5组柱子
static uint8_t columnCount = 0;
static uint16_t score = 0;
SemaphoreHandle_t xJumpSemaphore = NULL;
volatile uint8_t gameOver = 0;
// 定义两个实际的缓冲区
uint8_t frameBufferA[FRAMEBUFFER_SIZE];  // 缓冲区 A
uint8_t frameBufferB[FRAMEBUFFER_SIZE];  // 缓冲区 B

// 使用两个指针来切换前后缓冲
uint8_t *frameBufferFront = frameBufferA;
uint8_t *frameBufferBack  = frameBufferB;
void InitGame(void) {
    bird.y = SCREEN_HEIGHT / 2;
    bird.velocity = 0;
    columnCount = 0;
    score = 0;
    gameOver = 0;

    // 创建信号量
    xJumpSemaphore = xSemaphoreCreateBinary();

    // 创建任务
    xTaskCreate(Task_BirdControl, "Bird", 128, NULL, 2, NULL);
    xTaskCreate(Task_ColumnMove, "Column", 128, NULL, 2, NULL);
    xTaskCreate(Task_Display, "Display", 256, NULL, 1, NULL);

}

void Task_BirdControl(void *pvParameters) {
    while (1) { 
        if (xSemaphoreTake(xJumpSemaphore, 0) == pdTRUE) {
            bird.velocity = 50; // 向上跳
        } else {
            bird.velocity -= 8;  // 重力加速
        }

        bird.y += bird.velocity / 10; // 控制下落速度

        // 限制不能飞出屏幕
        if (bird.y < 0) bird.y = 0;
        if (bird.y > SCREEN_WIDTH - 22) bird.y = SCREEN_WIDTH - 22;

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Task_ColumnMove(void *pvParameters) {
    TickType_t lastTime = xTaskGetTickCount();
    static int columnflag = 0;
    while (!gameOver) {
        // 每隔一段时间生成新柱子
        if ((xTaskGetTickCount() - lastTime) > pdMS_TO_TICKS(4000)) {
            Column_t newCol = {
                .y = SCREEN_HEIGHT,
                .gapTop = rand() % (SCREEN_WIDTH - 60) + 10,
                .gapBot = newCol.gapTop + 80,
                .passed = 0
            };
            if (columnCount < 3)
                columns[columnCount++] = newCol;
            lastTime = xTaskGetTickCount();
        }
        if (columnflag == 0 && columnCount > 0) {
            columnflag = 1;
            xTaskCreate(Task_CollisionCheck, "Collision", 128, NULL, 3, NULL);
        }
        

        // 移动所有柱子
        for (int i = 0; i < columnCount; i++) {
            columns[i].y -= 1;
            if (columns[i].y < -40) {
                // 移除超出屏幕的柱子
                for (int j = i; j < columnCount - 1; j++)
                    columns[j] = columns[j + 1];
                columnCount--;
            }

            // 判断是否得分
            if (!columns[i].passed && columns[i].y < 30) {
                score++;
                columns[i].passed = 1;
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
void Task_CollisionCheck(void *pvParameters) {
    while (1) {
        if (columns[0].y <= 60 && columns[0].y >=0) { // 鸟的位置附近
            if (bird.y < columns[0].gapTop || bird.y > columns[0].gapBot) {
                gameOver = 1;
                vTaskSuspendAll(); // 暂停所有任务
                break;
            }
        }

        if (bird.y >= SCREEN_HEIGHT || bird.y < 0) {
            gameOver = 1;
            vTaskSuspendAll();
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}
void Task_Display(void *pvParameters) {
		
    while (1) {
        // 绘制鸟（简单像素点）
        frameBuffer_fill(frameBufferFront,Buffer_COLOR_BLACK);
        frameBuffer_DrawBird(frameBufferFront,bird.y, 40, Buffer_COLOR_WHITE);
        frameBuffer_DrawColumn(frameBufferFront);
        //显示部分
        lcdRefresh(frameBufferBack);
		swapBuffers();
        // 显示分数（使用更大的字体和更醒目的位置）
        // char buf[20];
        // sprintf(buf, "Score: %d", score);
        // LCD_DrawString(10, 10, buf, WHITE);

        // if (gameOver) {
        //     LCD_DrawString(50, 50, "GAME OVER", RED);
        //     sprintf(buf, "Final Score: %d", score);
        //     LCD_DrawString(30, 70, buf, RED);
        // }
        
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

//下面的部分都是和显示相关的
void swapBuffers(void)
{
    uint8_t *temp = frameBufferFront;
    frameBufferFront = frameBufferBack;
    frameBufferBack = temp;
}
void lcdRefresh(uint8_t *buffer)
{
    lcdSetWindow(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);

    for (uint32_t i = 0; i < FRAMEBUFFER_SIZE; i++)
    {
        uint8_t byte = buffer[i];
        for (int b = 7; b >= 0; b--)
        {
            uint16_t color = (byte & (1 << b)) ? COLOR_BLACK :COLOR_WHITE;
            MylcdWriteData(color);
        }
    }
}
void frameBuffer_fill(uint8_t *buffer,uint8_t color)
{
    uint8_t fill_data = (color == Buffer_COLOR_BLACK) ? 0xFF : 0x00;

    for (uint32_t i = 0; i < FRAMEBUFFER_SIZE; i++)
    {
        buffer[i] = fill_data;
    }
}
void frameBuffer_fillRect(uint8_t *buffer,uint16_t x, uint16_t y, uint16_t size, uint8_t color)
{
    for (uint16_t py = 0; py < size; py++)
    {
        for (uint16_t px = 0; px < size; px++)
        {
            int pixel_x = x + px;
            int pixel_y = y + py;

            if (pixel_x >= SCREEN_WIDTH || pixel_y >= SCREEN_HEIGHT)
                continue;

            uint32_t index = (pixel_y * SCREEN_WIDTH + pixel_x) / 8;
            uint8_t bit_pos = 7 - ((pixel_y * SCREEN_WIDTH + pixel_x) % 8);

            if (color == Buffer_COLOR_BLACK)
                buffer[index] |= (1 << bit_pos);
            else
                buffer[index] &= ~(1 << bit_pos);
        }
    }
}
void frameBuffer_fillRectWH(uint8_t *buffer, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    for (uint16_t py = 0; py < height; py++) {
        for (uint16_t px = 0; px < width; px++) {
            int pixel_x = x + px;
            int pixel_y = y + py;

            // 防止越界
            if (pixel_x >= SCREEN_WIDTH || pixel_y >= SCREEN_HEIGHT)
            continue;

            // 计算在 buffer 中的位置
            uint32_t index = (pixel_y * SCREEN_WIDTH + pixel_x) / 8;
            uint8_t bit_pos = 7 - ((pixel_y * SCREEN_WIDTH + pixel_x) % 8);

            if (color == Buffer_COLOR_BLACK)
            buffer[index] |= (1 << bit_pos);   // 设置像素点
            else
            buffer[index] &= ~(1 << bit_pos); // 清除像素点
        }
    }
}
void frameBuffer_DrawBird(uint8_t *buffer,uint16_t x, uint16_t y, uint8_t color)
{
    // 确保不会越界
    if (x + 20 >= SCREEN_WIDTH || y + 20 >= SCREEN_HEIGHT)
        return;

    frameBuffer_fillRect(buffer,x, y, 20, color);
}
void frameBuffer_DrawColumn(uint8_t *buffer)
{
    // 上方柱子：从 y=0 到 gapTop
    for (int i = 0; i < columnCount; i++) {
        if (columns[i].y < 0)
        {
            frameBuffer_fillRectWH(buffer,0,0, columns[i].gapTop, 40+columns[i].y, Buffer_COLOR_WHITE);
            frameBuffer_fillRectWH(buffer,columns[i].gapBot,0, SCREEN_HEIGHT - columns[i].gapBot,40+columns[i].y, Buffer_COLOR_WHITE);
        }
        else
        {
            frameBuffer_fillRectWH(buffer,0,columns[i].y, columns[i].gapTop, 40, Buffer_COLOR_WHITE);
            frameBuffer_fillRectWH(buffer,columns[i].gapBot,columns[i].y, SCREEN_HEIGHT - columns[i].gapBot,40, Buffer_COLOR_WHITE);
        }
    }
}