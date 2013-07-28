#define VERSION "eView 0.62 29.jul.2013"
#ifdef language_russian
#define EVIEW_IS_STARTING "eView запускается"
#define PLEASE_WAIT " Пожалуйста, подождите! " 

#define ERROR "Ошибка" 
#define UNKNOWN_OR_DAMAGED_ARCHIVE "Неизвестный тип архива\nлибо архив повреждён"
#define UNABLE_TO_ENTER_NEXT_DIRECTORY "Переход в следующий\nкаталог неудался!"
#define UNABLE_TO_ENTER_PREVIOUS_DIRECTORY "Переход в предыдущий\nкаталог неудался!"
#define UNABLE_TO_SHOW "Невозможно отобразить: \n"
#define GTK_PARTS_IS_OUTDATED "Ошибка eView: \nОбнаружены компоненты GTK версии %s, требуемая версия не ниже %d. \nПожалуйста, скачайте https://dl.dropboxusercontent.com/u/100376233/eView/GTK_parts.sh и обновите компоненты"
#define FAILED_TO_START_XFBDEV "Ошибка eView: \nНе удалось запустить Xfbdev. Пожалуйста, свяжитесь с разработком (S-trace@list.ru) чтобы попытаться исправить это"
#define FAILED_TO_OPEN_DEV_FB0 "Ошибка eView: \nНевозможно открыть /dev/fb0 - экран не будет обновляться! Пожалуйста, свяжитесь с разработком (S-trace@list.ru) чтобы попытаться исправить это"

#define DELETE_CONFIRM "Подтвердите удаление"
#define MOVE_CONFIRM "Подтвердите перемещение"
#define MOVE "Переместить"

#define POWER_STATUS "Энергопитание"
#define POWER_SOURCE_IS_BATTERY "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до разрядки: %s"
#define POWER_SOURCE_IS_USB "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до зарядки: %s\nТок USB (мА): %s\nНапряжение USB (мВ): %s"
#define POWER_SOURCE_IS_AC "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до зарядки: %s\nТок AC (мА): %s\nНапряжение AC (мВ): %s"

#define MAIN_MENU "Главное меню"
#define CREATE_TEMPORARY_DIRECTORY "Cоздать каталог temp000"
#define COPY "Копировать"
#define MOVE_FILE "Переместить"
#define DELETE "Удалить"
#define EXIT "Выход из eView"

#define OPTIONS "Опции"
#define FILEMANAGER_MODE "Файлменеджер (две панели)"
#define PARTIAL_UPDATE "Быстрое обновление"
#define CONFIRM_MOVE "Подтверждать перемещение"
#define SHOW_HIDDEN_FILES "Показывать скрытные файлы"
#define SHOW_PANEL "Показывать время и заряд"
#define LED_NOTIFY "Уведомления светодиодом"
#define RESET_CONFIGURATION "Сбросить конфигурацию"
#define BACKLIGHT "Подсветка"
#define ABOUT_PROGRAM "О программе"
#define ABOUT_PROGRAM_TEXT VERSION"\n\nОригинальная разработка Mini Gtk-file-manager:\nTito Ragusa (2002-2006) <tito-wolit@tiscali.it>\n\nНачальная разработка eView (до версии 0.44):\nNorin Maxim (2011)\n\nДоработка и усовершенствование eView (0.45++):\nSoul Trace (2013) <s-trace@list.ru>"

#define SETTINGS      "Настройки"
#define CROP_IMAGE    "Обрезать поля"
#define ROTATE_IMAGE  "Повернуть картинку"
#define FRAME_IMAGE   "Умное листание"
#define MANGA_MODE    "Листать как мангу"
#define KEEP_ASPECT   "Сохранять пропорции"
#define DOUBLE_REFRESH "Двойное обновление"
#define ACTION_ON_LAST_FILE "При последнем файле"
#define DO_NOTHING     "Ничего не делать"
#define LOOP_DIRECTORY "Зациклить каталог"
#define NEXT_DIRECTORY "Перейти в следующий каталог"
#define EXIT_TO_FILEMANAGER "Перейти в файлменеджер"
#define ALLOW_PRELOADING "Разрешить предзагрузку"
#define SUPPRESS_BATTERY_WARNINGS "Подавлять жалобы на батарейку"
#define BATTERY_CHARGE_PERCENT "Заряд батареи: %s%%"
#define VIEWED_PAGES "Просмотрено страниц: "
#define RESET_VIEWED_PAGES "Сбросить счётчик страниц?"

#define INFORMATION "Информация"
#define FIRST_FILE_REACHED      "Достигнут первый\nфайл в каталоге"
#define FIRST_FILE_REACHED_LOOP "Достигнут первый файл,\nначинаю с последнего!"
#define FIRST_FILE_REACHED_EXIT "Достигнут первый файл,\nвыхожу в файловый менеджер"
#define LAST_FILE_REACHED       "Достигнут последний\nфайл в каталоге"
#define LAST_FILE_REACHED_LOOP  "Достигнут последний файл,\nначинаю с первого!"
#define LAST_FILE_REACHED_EXIT  "Достигнут последний файл,\nвыхожу в файловый менеджер"

#endif

#ifdef language_english
#define EVIEW_IS_STARTING "Starting eView"
#define PLEASE_WAIT       " Please wait! " 

#define ERROR "Error" 
#define UNKNOWN_OR_DAMAGED_ARCHIVE "Unknown archive type\nor broken archive"
#define UNABLE_TO_ENTER_NEXT_DIRECTORY "Unable to enter\nnext directory!"
#define UNABLE_TO_ENTER_PREVIOUS_DIRECTORY "Unable to enter\nprevious directory!"
#define UNABLE_TO_SHOW "Unable to show: \n"
#define GTK_PARTS_IS_OUTDATED "eView error: \nFound GTK_parts version %s, needed version is %d or upper. \nPlease, download from https://dl.dropboxusercontent.com/u/100376233/eView/GTK_parts.sh and upgrade Your GTK_parts."
#define FAILED_TO_START_XFBDEV "eView error: \nXfbdev startup failed. Please contact eView developer (S-trace@list.ru) to try to fix it"
#define FAILED_TO_OPEN_DEV_FB0 "eView error: \nUnable to open /dev/fb0 - display won't refresh! Please contact eView developer (S-trace@list.ru) to try to fix it"

#define DELETE_CONFIRM "Please confirm deletion"
#define MOVE_CONFIRM "Please confirm moving"
#define MOVE "Move"

#define POWER_STATUS "Power information"
#define POWER_SOURCE_IS_BATTERY "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to discharge: %s"
#define POWER_SOURCE_IS_USB "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nUSB current (mA): %s\nUSB voltage (mV): %s"
#define POWER_SOURCE_IS_AC "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nAC current (mA): %s\nAC voltage (mV): %s"

#define MAIN_MENU "Main menu"
#define CREATE_TEMPORARY_DIRECTORY "Create temp000 directory"
#define COPY "Copy"
#define MOVE_FILE "Move"
#define DELETE "Delete"
#define EXIT "Exit eView"

#define OPTIONS "Options"
#define FILEMANAGER_MODE "Filemanager (two panels)"
#define PARTIAL_UPDATE "Use partial update"
#define CONFIRM_MOVE "Confirm move"
#define SHOW_HIDDEN_FILES "Show hidden files"
#define SHOW_PANEL "Show clock and battery"
#define LED_NOTIFY "LED notifications"
#define BACKLIGHT "Backlight"
#define RESET_CONFIGURATION "Reset configuration"
#define ABOUT_PROGRAM "About program"
#define ABOUT_PROGRAM_TEXT VERSION"\n\nOriginal Mini Gtk-file-manager developement:\nTito Ragusa (2002-2006) <tito-wolit@tiscali.it>\n\nOriginal eView developement (up to 0.44):\nNorin Maxim (2011)\n\neView updates and enhacements:\nSoul Trace (2013) <s-trace@list.ru>"

#define SETTINGS "Settings"
#define CROP_IMAGE    "Crop margins"
#define ROTATE_IMAGE  "Rotate image"
#define FRAME_IMAGE   "Smart scrolling"
#define MANGA_MODE    "Manga pages order"
#define KEEP_ASPECT   "Keep aspect rate"
#define DOUBLE_REFRESH "Double refresh"
#define ACTION_ON_LAST_FILE "On last file"
#define DO_NOTHING     "Do nothing"
#define LOOP_DIRECTORY "Loop directory"
#define NEXT_DIRECTORY "Enter next directory"
#define EXIT_TO_FILEMANAGER "Exit to filemanager"
#define ALLOW_PRELOADING "Allow image preloading"
#define SUPPRESS_BATTERY_WARNINGS "Suppress battery warnings"
#define BATTERY_CHARGE_PERCENT "Battery: %s%%"
#define VIEWED_PAGES "Viewed pages: "
#define RESET_VIEWED_PAGES "Reset viewed pages counter?"

#define INFORMATION "Information"
#define FIRST_FILE_REACHED      "First file in\n directory reached"
#define FIRST_FILE_REACHED_LOOP "First file in directory reached,\ngoing to last file!"
#define FIRST_FILE_REACHED_EXIT "First file in directory reached,\nexiting to filemanager"
#define LAST_FILE_REACHED      "Last file in\n directory reached"
#define LAST_FILE_REACHED_LOOP "Last file in directory reached,\ngoing to first file!"
#define LAST_FILE_REACHED_EXIT "Last file in directory reached,\nexiting to filemanager"

#endif