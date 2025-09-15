#pragma once

#include "stm32f405xx.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

#define wait HAL_Delay

typedef struct LED_Color_Data {
        GPIO_TypeDef *GPIO_Port;
        uint16_t GPIO_Pin;
} LED_Color_Data;

typedef struct LED_RGB_Data {
        LED_Color_Data r;
        LED_Color_Data g;
        LED_Color_Data b;
} LED_RGB_Data;

enum Color {
    WHITE = 0b111,
    BLUE = 0b001,
    GREEN = 0b010,
    RED = 0b100,
    CYAN = 0b011,
    MAGENTA = 0b101,
    YELLOW = 0b110,
    OFF = 0b000
};

class LED_Base {
    protected:
        Color state = Color::OFF;

        void setPin(GPIO_TypeDef *port, uint16_t pin, bool state) {
            if (state)
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
            else
                HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
        }

    public:
        virtual void setColor(Color color) {
            state = color;
        };

        virtual ~LED_Base() = default;

        virtual void Off() {
            setColor(Color::OFF);
        };

        virtual void OK() {
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
            wait(300);
            Off();
            wait(300);
            setColor(state);
            wait(300);
            Off();
        }
};
