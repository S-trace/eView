/*
 Based on  libro_hw.h by Andrey Markeev, 2011, digma_hw.c by Alexander Drozdoff <hatred@inbox.ru>
 Update for Qt support by S-trace <s-trace@list.ru>, 2013
 Distributed under GPLv2 Terms
 http://hatred.homelinux.net/~hatred/digma
 */
#ifndef _DIGMA_HW_H_
#define _DIGMA_HW_H_

/*
  Refresh functionality
*/

/* IOCTL calls for E-Ink paper update */
#define EPAPER_UPDATE_LOCAL 0x101         /** Update localarea */
#define EPAPER_UPDATE_PART  0x102         /** ???              */
#define EPAPER_UPDATE_FULL  0x103         /** Fully update     */
#define EPAPER_UPDATE_DISPLAY_QT  0x120d  /** Update all display */

#define KEY_HOME   0xFFC5
#define KEY_PGUP   0xFF55
#define KEY_PGDOWN 0xFF56
#define KEY_UP     0xFF52
#define KEY_DOWN   0xFF54
#define KEY_RIGHT  0xFF53
#define KEY_LEFT   0xFF51
#define KEY_OK     0xFF0D
#define KEY_MENU   0xFFC6
#define KEY_BACK   0xFF1B
#define KEY_SHIFT  0xFFE1

#define KEY_MENU_LIBROII 0x2E   /* Кнопка МЕНЮ на прошивке для LIBRO II. */
#define KEY_SETTINGS_LIBROII 0x2C /* Кнопка НАСТРОЙКИ на прошивке для LIBRO II. */
#define KEY_REFRESH_LIBROII 0x00  /* Кнопка ОБНОВИТЬ на прошивке для LIBRO II. */

#define KEY_REFRESH_QT 0xFFC2
#define KEY_MENU_QT    0xFFC3
#define KEY_OPTIONS_QT 0xFFC4
#define KEY_POWER_QT   0xFF13 /* BREAK на PC */

void epaperUpdateFull(void);
void epaperUpdateLocal(void);
void epaperUpdatePart(void);
void set_brightness(int value);
void set_led_state (int state);
void detect_hardware(void);
extern int hardware_has_backlight, hardware_has_LED;
extern pthread_t suspend_helper_tid;

enum
{
  LED_OFF,
  LED_BLINK_SLOW,
  LED_BLINK_FAST,
  LED_ON,
  LED_STATES
};
extern int LED_state[LED_STATES]; /* Состояния светодиода */
extern int previous_backlight_level; /* Уровень подсветки перед запуском eView */
extern int suspended; /* Текущее состояние книги */
extern int was_in_picture_viewer; /* Находились ли мы в смотрелке перед вызовом усыплятора */
void suspend_hardware(void); /* Тупо усыпляет железяку (проснётся от любого нажатия клавиши и отработает его, словно и не спала вовсе) */
#endif /* _DIGMA_HW_H_ */
