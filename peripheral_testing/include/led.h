#ifndef LED_H
#define LED_H

void led_init(void);
void led_set(int red, int green, int blue);
void led_off(void);
void led_blink_all(int count);
void led_test(void);

#endif
