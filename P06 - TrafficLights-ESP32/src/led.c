#include "../include/led.h"

static gpio_int_type_t ledRojo, ledAmarillo, ledVerde; //estas variables solamente son accesibles en este archivo

void configurarLed(gpio_int_type_t ledR, gpio_int_type_t ledA, gpio_int_type_t ledV)
{
    ledRojo = ledR;
    ledAmarillo = ledA;
    ledVerde = ledV;
    gpio_set_direction(ledRojo, GPIO_MODE_OUTPUT);
    gpio_set_direction(ledAmarillo, GPIO_MODE_OUTPUT);
    gpio_set_direction(ledVerde, GPIO_MODE_OUTPUT);
}

void prenderLed(char led)
{
    switch (led)
    {
    case 'R':
    gpio_set_level(ledRojo,1);
    break;
    case 'A':
    gpio_set_level(ledAmarillo,1);
    break;
    case 'V':
    gpio_set_level(ledVerde,1);
    break;
    default:
        break;
    }
}

void apagarLed(char led)
{
    switch (led)
    {
    case 'R':
        gpio_set_level(ledRojo,0);
        break;
    case 'A':
        gpio_set_level(ledAmarillo,0);
        break;
    case 'V':
        gpio_set_level(ledVerde,0);
        break;
    default:
        break;
    }
}