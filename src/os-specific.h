/* Soul Trace, 2013, Distributed under GPLv2 Terms */
/* Definitions of OS-specific commands and so      */
#ifdef __amd64
//Тестовые значения
#define SORT_COMMAND "sort --version-sort"

#define BATTERY_CAPACITY "src/power_supply/bat/capacity"
#define BATTERY_CURRENT  "src/power_supply/bat/current_now"
#define BATTERY_VOLT "src/power_supply/bat/voltage_now"
#define BATTERY_TEMP "src/power_supply/bat/temp"

#define BATTERY_CHIP_TEMP "src/power_supply/bat/chip_temp"
#define BATTERY_CHIP_VOLT "src/power_supply/bat/chip_volt"

#define BATTERY_STATUS "src/power_supply/bat/status"
#define BATTERY_POWER_SUPPLIER "src/power_supply/bat/power_supplier"

#define BATTERY_TIME_TO_FULL  "src/power_supply/bat/time_to_full_now"
#define BATTERY_TIME_TO_EMPTY "src/power_supply/bat/time_to_empty_now"

#define USB_CURRENT "src/power_supply/usb/current_now" 
#define USB_VOLTAGE "src/power_supply/usb/voltage_now"

#define AC_CURRENT "src/power_supply/ac/current_now"
#define AC_VOLTAGE "src/power_supply/ac/voltage_now"

#else
#define SORT_COMMAND "natsort"

#define BATTERY_CAPACITY "/sys/class/power_supply/bat/capacity"
#define BATTERY_CURRENT  "/sys/class/power_supply/bat/current_now"
#define BATTERY_VOLT "/sys/class/power_supply/bat/voltage_now"
#define BATTERY_TEMP "/sys/class/power_supply/bat/temp"

#define BATTERY_CHIP_TEMP "/sys/class/power_supply/bat/chip_temp"
#define BATTERY_CHIP_VOLT "/sys/class/power_supply/bat/chip_volt"

#define BATTERY_STATUS "/sys/class/power_supply/bat/status"
#define BATTERY_POWER_SUPPLIER "/sys/class/power_supply/bat/power_supplier"
// #define BATTERY_CHARGER "/sys/class/power_supply/bat/batt_charger" //Всегда No charger

#define BATTERY_TIME_TO_FULL "/sys/class/power_supply/bat/time_to_full_now"
#define BATTERY_TIME_TO_EMPTY "/sys/class/power_supply/bat/time_to_empty_now"

#define USB_CURRENT "/sys/class/power_supply/usb/current_now"
#define USB_VOLTAGE  "/sys/class/power_supply/usb/voltage_now"

#define AC_CURRENT "/sys/class/power_supply/ac/current_now"
#define AC_VOLTAGE  "/sys/class/power_supply/ac/voltage_now"

#endif
