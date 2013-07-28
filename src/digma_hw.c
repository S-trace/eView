/**
 * Copyright (c) 2011 Alexander Drozdoff <hatred@inbox.ru>
 * Update for Qt support by S-trace <s-trace@list.ru>, 2013
 * Ditributed under GPLv2 Terms
 * http://hatred.homelinux.net/~hatred/digma
 */

#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "digma_hw.h"

int hardware_has_backlight=FALSE, hardware_has_LED=FALSE;
char *LED_path=NULL, *backlight_path=NULL;
extern int framebuffer_descriptor;
extern int QT;
extern int enable_refresh;//Принудительно запретить обновлять экран в особых случаях
extern int LED_notify; // Оповещение светодиодом об обновлении панелей и загрузке

/* Helper FB update function */
static void epaperUpdate(int ioctl_call, int mode)
{
  if (! enable_refresh)
  {
    #ifdef debug_printf
    printf ("Display refresh was locked, IGNORED (ioctl %d mode %d)!\n", ioctl_call, mode);
    #endif
    return;
  }
  #ifndef __amd64
  if (framebuffer_descriptor >= 0)
  {
    if (QT)
      usleep(205000);
      ioctl(framebuffer_descriptor, ioctl_call, &mode);
  }
  #endif
}

/**
 * Update E-Ink screen fully
 */
void epaperUpdateFull(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 3);
  else
    epaperUpdate(EPAPER_UPDATE_FULL, 3); // Выполняется за 0,847s
}

/**
 * Update only local (???)
 */
void epaperUpdateLocal(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 2);
  else
    epaperUpdate(EPAPER_UPDATE_LOCAL, 2); // Выполняется за 0,847s
}

/**
 * Untested E-Ink function
 */
void epaperUpdatePart(void)
{
  if (QT)
    epaperUpdate(EPAPER_UPDATE_DISPLAY_QT, 1);
  else
    epaperUpdate(EPAPER_UPDATE_PART, 1); // Выполняется за 0,847s
}

int check_for_file (char *fpath)  // Проверка наличия файла в файловой системе
{
  if (access(fpath, F_OK))  // F_OK просто проверяет существование файла
  {  
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}

void detect_hardware(void) // Обнаружение оборудования и его возможностей
{  
  hardware_has_backlight=check_for_file ("/sys/class/backlight/boeye_backlight/bl_power");
  if (hardware_has_backlight) 
  {
    backlight_path="/sys/class/backlight/boeye_backlight/bl_power";
    #ifdef debug_printf
    printf ("Found backlight control at file %s\n", backlight_path);
    #endif
  }
  hardware_has_LED=check_for_file ("/sys/class/leds/charger-led/brightness");
  if (hardware_has_LED) 
  {
    LED_path="/sys/class/leds/charger-led/brightness";
    #ifdef debug_printf
    printf ("Found LED control at file %s\n", LED_path);
    #endif
  }
  else
  {
    hardware_has_LED=check_for_file ("/sys/class/leds/axp192-led-classdev/brightness");
    if (hardware_has_LED) 
    {
      LED_path="/sys/class/leds/axp192-led-classdev/brightness";
      #ifdef debug_printf
      printf ("Found LED control at file %s\n", LED_path);
      #endif
    }
  }
  #ifdef debug_printf
  if (! LED_notify)
  {
    printf ("LED control not found\n");
  }
  if (! hardware_has_backlight)
  {
    printf ("Backlight control not found\n");
  }
  #endif
}


void set_led_state (int state)
{
  if (LED_notify)
  {
    #ifdef debug_printf
    printf("writing %d to %s\n", state, LED_path);
    #endif
    FILE *file_descriptor=fopen(LED_path,"wt");
    if (!file_descriptor)
    {
      #ifdef debug_printf
      printf("UNABLE TO OPEN %s FILE FOR WRITING!\n", LED_path);
      #endif
      return ;
    }
    else
    {
      fprintf(file_descriptor, "%d", state);
      fclose(file_descriptor);
    }
  }
}