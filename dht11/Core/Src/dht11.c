#include "dht11.h"
#include "main.h"

extern TIM_HandleTypeDef htim2;

#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_0

void Delay_us(uint16_t us) {
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    while (__HAL_TIM_GET_COUNTER(&htim2) < us);
}

void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void DHT11_Init(void) {
    Set_Pin_Output(DHT11_PORT, DHT11_PIN);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
}

uint8_t DHT11_Read(uint8_t *temperature, uint8_t *humidity) {
    uint8_t data[5] = {0, 0, 0, 0, 0};

    // Starting signal
    Set_Pin_Output(DHT11_PORT, DHT11_PIN);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(18);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    Delay_us(30);
    Set_Pin_Input(DHT11_PORT, DHT11_PIN);

    // Wait for the answer signal
    uint32_t timeout = 10000;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && timeout--);
    if (timeout == 0) return 1;

    timeout = 10000;
    while (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && timeout--);
    if (timeout == 0) return 1;

    timeout = 10000;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) && timeout--);
    if (timeout == 0) return 1;

    // Read 40 bit data
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            while (!HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));
            Delay_us(40);
            if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN))
                data[i] |= (1 << (7 - j));
            while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN));
        }
    }

    // data control
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) return 1;

    *humidity = data[0];
    *temperature = data[2];

    return 0;
}
