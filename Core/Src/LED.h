#include "LED_Base.h"

class LED : LED_Base {
    private:
        LED_Color_Data led;

    public:
        LED(LED_Color_Data &led) : led(led) {}
         
        void setColor(Color color) {
            state = color;
            setPin(led.GPIO_Port, led.GPIO_Pin, color);
        }

        void On() {
            setColor(Color::WHITE);
        }


};
