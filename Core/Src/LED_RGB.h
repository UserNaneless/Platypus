#include "LED_Base.h"

class LED_RGB : LED_Base {
    private:
        LED_RGB_Data led;

    public:
        LED_RGB(LED_RGB_Data &led) : led(led) {}

        void setColor(Color color) {
            state = color;
            setPin(led.r.GPIO_Port, led.r.GPIO_Pin, (color >> 2) & 1);
            setPin(led.g.GPIO_Port, led.g.GPIO_Pin, (color >> 1) & 1);
            setPin(led.b.GPIO_Port, led.b.GPIO_Pin, color & 1);
        }

        void Off() {
            setColor(Color::OFF);
        }

        void OK() {
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

        void OK(Color color) {
            setColor(color);
            wait(300);
            Off();
            wait(300);
            setColor(color);
            wait(300);
            Off();
            wait(300);
            setColor(color);
            wait(300);
            Off();
        }
};
