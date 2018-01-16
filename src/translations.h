#ifdef language_russian
#define EVIEW_IS_STARTING "eView запускается"
#define PLEASE_WAIT " Пожалуйста, подождите! "

#define ERROR "Ошибка"
#define UNKNOWN_OR_DAMAGED_ARCHIVE "Неизвестный тип архива\nлибо архив повреждён"
#define UNABLE_TO_ENTER_NEXT_DIRECTORY "Переход в следующий\nкаталог неудался!"
#define UNABLE_TO_ENTER_PREVIOUS_DIRECTORY "Переход в предыдущий\nкаталог неудался!"
#define UNABLE_TO_SHOW "Невозможно отобразить: \n"
#define UNABLE_TO_CHANGE_DIRECTORY_TO "Невозможно войти в каталог '%s' - %s"
#define UNABLE_TO_ACCESS_FILE "Невозможно получить доступ к '%s' - %s"
#define NO_IMAGES_IN_CURRENT_DIRECTORY "Нет изображений в текущем каталоге - нажмите назад или далее чтобы продолжить"
#define GTK_PARTS_IS_OUTDATED "Ошибка eView: \nОбнаружены компоненты GTK версии %s, требуемая версия не ниже %d. \nПожалуйста, скачайте новейшую версию с http://raw.github.com/S-trace/eView/master/GTK_parts/GTK_parts.sh и обновите компоненты"
#define FAILED_TO_START_XFBDEV "Ошибка eView: \nНе удалось запустить Xfbdev. Пожалуйста, свяжитесь с разработком (S-trace@list.ru) чтобы попытаться исправить это"
#define FAILED_TO_OPEN_DEV_FB0 "Ошибка eView: \nНевозможно открыть /dev/fb0 - экран не будет обновляться! Пожалуйста, свяжитесь с разработком (S-trace@list.ru) чтобы попытаться исправить это"
#define XFBDEV_STARTUP_TIMEOUT "Ошибка eView: \nНе удалось запустить Xfbdev - превышено время ожидания запуска. Пожалуйста, свяжитесь с разработком (S-trace@list.ru) чтобы попытаться исправить это"
#define PIXBUF_LOADING_FROM_FILE_FAILED "Загрузка изображения из файла не удалась: %s"
#define PIXBUF_CROPPING_FAILED "Обрезка изображения не удалась!"
#define PIXBUF_SCALING_FAILED  "Масштабирование изображения не удалось!"
#define PIXBUF_ROTATING_FAILED "Поворот изображения не удался!"
#define SUBPIXBUF_CREATING_FAILED "Невозможно создать подизображения!"

#define DELETE_CONFIRM "Подтвердите удаление"
#define MOVE_CONFIRM "Подтвердите перемещение"
#define MOVE "Переместить"

#define POWER_STATUS "Энергопитание"
#define POWER_SOURCE_IS_BATTERY "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до разрядки: %s"
#define POWER_SOURCE_IS_USB "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до зарядки: %s\nТок USB (мА): %s\nНапряжение USB (мВ): %s"
#define POWER_SOURCE_IS_AC "Заряд: %s%%\nТемпература чипа (C): %s\nНапряжение чипа (мВ): %s\nТок батареи (мА): %s\nИсточник питания: %s\nХод зарядки: %s\nТемпература батареи (С): %s\nНапряжение батареи (мВ): %s\nВремя до зарядки: %s\nТок AC (мА): %s\nНапряжение AC (мВ): %s"

#define POWER_SOURCE_IS_BATTERY_RK2818 "Заряд: %s%%\nТок батареи (мА): %s\nИсточник питания: батарея\nХод зарядки: %s\nНапряжение батареи (мВ): %s"
#define POWER_SOURCE_IS_USB_RK2818 "Заряд: %s%%\nТок батареи (мА): %s\nИсточник питания: USB\nХод зарядки: %s\nНапряжение батареи (мВ): %s\nНапряжение USB (мВ): %s"
#define POWER_SOURCE_IS_AC_RK2818 "Заряд: %s%%\nТок батареи (мА): %s\nИсточник питания: AC адаптер\nХод зарядки: %s\nНапряжение батареи (мВ): %s\nНапряжение AC (мВ): %s"

#define MAIN_MENU "Главное меню"
#define CLOSE_MENU "Закрыть меню"
#define CREATE_TEMPORARY_DIRECTORY "Cоздать каталог temp000"
#define COPY "Копировать"
#define MOVE_FILE "Переместить"
#define DELETE "Удалить"
#define EXIT "Выход из eView"

#define OPTIONS "Опции"
#define FILEMANAGER_MODE "Файлменеджер (две панели)"
#define PARTIAL_UPDATE "Быстрое обновление"
#define CONFIRM_MOVE "Подтверждать перемещение"
#define SHOW_HIDDEN_FILES "Показывать скрытые файлы"
#define SHOW_PANEL "Показывать время и заряд"
#define LED_NOTIFY "Уведомления светодиодом"
#define RESET_CONFIGURATION "    Сбросить конфигурацию"
#define BACKLIGHT "Подсветка"
#define SLEEP_TIMEOUT "Таймаут сна"
#define ABOUT_PROGRAM "    О программе"
#define ABOUT_PROGRAM_TEXT VERSION"\n\nОригинальная разработка Mini Gtk-file-manager:\nTito Ragusa (2002-2006) <tito-wolit@tiscali.it>\n\nНачальная разработка eView (до версии 0.44):\nNorin Maxim (2011)\n\nДоработка и усовершенствование eView (0.45++):\nSoul Trace (2013) <s-trace@list.ru>\nТестирование (0.63) и иконка - Userok(aka QbiX) (ddixlab.ru) "

#define SETTINGS      "Настройки"
#define CROP_IMAGE    "Обрезать поля"
#define SPLIT_DOUBLE_PAGES "Делить развороты пополам"
#define ROTATE_IMAGE  "Повернуть изображение"
#define WEB_MANGA_MODE "Режим веб-манги"
#define FRAME_IMAGE   "Умное листание"
#define OVERLAP_VALUE "Наложение страниц (%)"
#define MANGA_MODE    "Листать как мангу"
#define KEEP_ASPECT   "Сохранять пропорции"
#define DOUBLE_REFRESH "Двойное обновление"
#define ACTION_ON_LAST_FILE "При последнем файле"
#define DO_NOTHING     "Ничего не делать"
#define LOOP_DIRECTORY "Зациклить каталог"
#define NEXT_DIRECTORY "Перейти в следующий каталог"
#define EXIT_TO_FILEMANAGER "Перейти в файлменеджер"
#define ALLOW_PRELOADING "Разрешить предзагрузку"
#define ALLOW_CACHING "Разрешить кэширование"
#define SUPPRESS_BATTERY_WARNINGS "Подавлять жалобы на батарейку"
#define HD_SCALING "HD масштабирование"
#define BOOST_CONTRAST "Усиление контраста"
#define BATTERY_CHARGE_PERCENT "Заряд батареи: %s%%"
#define VIEWED_PAGES "Просмотрено страниц: "
#define RESET_VIEWED_PAGES "Сбросить счётчик страниц?"

#define INFORMATION "Информация"
#define FIRST_FILE_REACHED      "Достигнут первый файл\nв каталоге"
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
#define UNABLE_TO_CHANGE_DIRECTORY_TO "Unable to enter directory '%s' - %s"
#define UNABLE_TO_ACCESS_FILE "Unable to access '%s' - %s"
#define NO_IMAGES_IN_CURRENT_DIRECTORY "No images in current directory - press back or forward to continue"
#define GTK_PARTS_IS_OUTDATED "eView error: \nFound GTK_parts version %s, needed version is %d or upper. \nPlease, download from http://raw.github.com/S-trace/eView/master/GTK_parts/GTK_parts.sh and upgrade Your GTK_parts."
#define FAILED_TO_START_XFBDEV "eView error: \nXfbdev startup failed. Please contact eView developer (S-trace@list.ru) to try to fix it"
#define FAILED_TO_OPEN_DEV_FB0 "eView error: \nUnable to open /dev/fb0 - display won't refresh! Please contact eView developer (S-trace@list.ru) to try to fix it"
#define XFBDEV_STARTUP_TIMEOUT "eView error: \nXfbdev startup failed - wait for server timeout exceeded. Please contact eView developer (S-trace@list.ru) to try to fix it"
#define PIXBUF_LOADING_FROM_FILE_FAILED "Image loading failed: %s"
#define PIXBUF_CROPPING_FAILED "Image margins cropping failed!"
#define PIXBUF_SCALING_FAILED  "Image scaling failed!"
#define PIXBUF_ROTATING_FAILED "Image rotating failed!"
#define SUBPIXBUF_CREATING_FAILED "Unable to create subimages!"

#define DELETE_CONFIRM "Please confirm deletion"
#define MOVE_CONFIRM "Please confirm moving"
#define MOVE "Move"

#define POWER_STATUS "Power information"
#define POWER_SOURCE_IS_BATTERY "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to discharge: %s"
#define POWER_SOURCE_IS_USB "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nUSB current (mA): %s\nUSB voltage (mV): %s"
#define POWER_SOURCE_IS_AC "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nAC current (mA): %s\nAC voltage (mV): %s"

#define POWER_SOURCE_IS_BATTERY_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: battery\nCharging status: %s\nBattery voltage (mV): %s"
#define POWER_SOURCE_IS_USB_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: USB\nCharging status: %s\nBattery voltage (mV): %s\nUSB voltage (mV): %s"
#define POWER_SOURCE_IS_AC_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: AС adapter\nCharging status: %s\nBattery voltage (mV): %s\nAC voltage (mV): %s"

#define MAIN_MENU "Main menu"
#define CLOSE_MENU "Close menu"
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
#define SLEEP_TIMEOUT "Sleep timeout"
#define RESET_CONFIGURATION "    Reset configuration"
#define ABOUT_PROGRAM "    About program"
#define ABOUT_PROGRAM_TEXT VERSION"\n\nOriginal Mini Gtk-file-manager developement:\nTito Ragusa (2002-2006) <tito-wolit@tiscali.it>\n\nOriginal eView developement (up to 0.44):\nNorin Maxim (2011)\n\neView updates and enhacements:\nSoul Trace (2013) <s-trace@list.ru>\nTesting (0.63) and icon - Userok(aka QbiX (ddixlab.ru)"

#define SETTINGS "Settings"
#define CROP_IMAGE    "Crop margins"
#define SPLIT_DOUBLE_PAGES "Split double-page spreads"
#define ROTATE_IMAGE  "Rotate image"
#define WEB_MANGA_MODE "Web-manga mode"
#define FRAME_IMAGE   "Smart scrolling"
#define OVERLAP_VALUE "Pages overlap (%)"
#define MANGA_MODE    "Manga pages order"
#define KEEP_ASPECT   "Keep aspect rate"
#define DOUBLE_REFRESH "Double refresh"
#define ACTION_ON_LAST_FILE "On last file"
#define DO_NOTHING     "Do nothing"
#define LOOP_DIRECTORY "Loop directory"
#define NEXT_DIRECTORY "Enter next directory"
#define EXIT_TO_FILEMANAGER "Exit to filemanager"
#define ALLOW_PRELOADING "Allow image preloading"
#define ALLOW_CACHING "Allow image caching"
#define SUPPRESS_BATTERY_WARNINGS "Suppress battery warnings"
#define HD_SCALING "HD scaling"
#define BOOST_CONTRAST "Boost contrast"
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

#ifdef language_chinese
#define EVIEW_IS_STARTING "启动eView"
#define PLEASE_WAIT       "请等待! "

#define ERROR "异常"
#define UNKNOWN_OR_DAMAGED_ARCHIVE "未知压缩包类型\n或者压缩包已损坏"
#define UNABLE_TO_ENTER_NEXT_DIRECTORY "无法点击\n下一级文件夹!"
#define UNABLE_TO_ENTER_PREVIOUS_DIRECTORY "无法点击\n上一级文件夹!"
#define UNABLE_TO_SHOW "无法显示: \n"
#define UNABLE_TO_CHANGE_DIRECTORY_TO "无法进入文件夹 '%s' - %s"
#define UNABLE_TO_ACCESS_FILE "无法访问 '%s' - %s"
#define NO_IMAGES_IN_CURRENT_DIRECTORY "当前目录中没有图像-按 \"后退\" 或 \"前进\" 继续"
#define GTK_PARTS_IS_OUTDATED "eView 异常: \n您的gtk版本 %s, 需要的版本 %d 无法达到要求. \n请到http://raw.github.com/S-trace/eView/master/GTK_parts/GTK_parts.sh 下载并且升级你的gtk."
#define FAILED_TO_START_XFBDEV "eView 异常: \nXfbdev 启动 失败. 请联系开发者 (S-trace@list.ru) 修复bug"
#define FAILED_TO_OPEN_DEV_FB0 "eView 异常: \n无法打开 /dev/fb0 请联系开发者 (S-trace@list.ru) 修复bug"
#define XFBDEV_STARTUP_TIMEOUT "eView error: \nXfbdev 启动失败 - 服务超时. 请联系开发者 (S-trace@list.ru) 修复bug"
#define PIXBUF_LOADING_FROM_FILE_FAILED "图片载入失败: %s"
#define PIXBUF_CROPPING_FAILED "图像边距裁剪失败!"
#define PIXBUF_SCALING_FAILED  "图像缩放失败!"
#define PIXBUF_ROTATING_FAILED "图像旋转失败!"
#define SUBPIXBUF_CREATING_FAILED "无法创建图像!"

#define DELETE_CONFIRM "请确认删除"
#define MOVE_CONFIRM "请确认移动"
#define MOVE "移动"

#define POWER_STATUS "电源信息"
#define POWER_SOURCE_IS_BATTERY "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to discharge: %s"
#define POWER_SOURCE_IS_USB "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nUSB current (mA): %s\nUSB voltage (mV): %s"
#define POWER_SOURCE_IS_AC "Charge: %s%%\nChip temperature (C): %s\nChip voltage (mV): %s\nBattery current (mA): %s\nPower source: %s\nCharging status: %s\nBattery temperature (С): %s\nBattery voltage (mV): %s\nTime to full charge: %s\nAC current (mA): %s\nAC voltage (mV): %s"

#define POWER_SOURCE_IS_BATTERY_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: battery\nCharging status: %s\nBattery voltage (mV): %s"
#define POWER_SOURCE_IS_USB_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: USB\nCharging status: %s\nBattery voltage (mV): %s\nUSB voltage (mV): %s"
#define POWER_SOURCE_IS_AC_RK2818 "Charge: %s%%\nBattery current (mA): %s\nPower source: AС adapter\nCharging status: %s\nBattery voltage (mV): %s\nAC voltage (mV): %s"

#define MAIN_MENU "主菜单"
#define CLOSE_MENU "关闭菜单"
#define CREATE_TEMPORARY_DIRECTORY "创建 temp000 文件夹"
#define COPY "复制"
#define MOVE_FILE "移动"
#define DELETE "删除"
#define EXIT "退出 eView"

#define OPTIONS "选项"
#define FILEMANAGER_MODE "文件管理器"
#define PARTIAL_UPDATE "使用部分更新"
#define CONFIRM_MOVE "确认移动"
#define SHOW_HIDDEN_FILES "显示隐藏文件"
#define SHOW_PANEL "显示时钟和电池"
#define LED_NOTIFY "指示灯通知"
#define BACKLIGHT "背光"
#define SLEEP_TIMEOUT "休眠超时"
#define RESET_CONFIGURATION "重置"
#define ABOUT_PROGRAM "    关于本程序"
#define ABOUT_PROGRAM_TEXT VERSION"\n\nOriginal Mini Gtk-file-manager developement:\nTito Ragusa (2002-2006) <tito-wolit@tiscali.it>\n\nOriginal eView developement (up to 0.44):\nNorin Maxim (2011)\n\neView updates and enhacements:\nSoul Trace (2013) <s-trace@list.ru>\nTesting (0.63) and icon - Userok(aka QbiX (ddixlab.ru)"

#define SETTINGS "设置"
#define CROP_IMAGE    "裁剪边距"
#define SPLIT_DOUBLE_PAGES "双页"
#define ROTATE_IMAGE  "旋转图像"
#define WEB_MANGA_MODE "网络-漫画模式"
#define FRAME_IMAGE   "智能滚动"
#define OVERLAP_VALUE "页面重叠 (%)"
#define MANGA_MODE    "漫画页订购"
#define KEEP_ASPECT   "保持宽高"
#define DOUBLE_REFRESH "双刷新"
#define ACTION_ON_LAST_FILE "上一文件"
#define DO_NOTHING     "什么都没有"
#define LOOP_DIRECTORY "循环目录"
#define NEXT_DIRECTORY "进入下一个目录"
#define EXIT_TO_FILEMANAGER "退出文件管理器"
#define ALLOW_PRELOADING "允许图像预加载"
#define ALLOW_CACHING "允许图像缓存"
#define SUPPRESS_BATTERY_WARNINGS "禁止电池警告"
#define HD_SCALING "高清缩放"
#define BOOST_CONTRAST "增强对比度"
#define BATTERY_CHARGE_PERCENT "电池: %s%%"
#define VIEWED_PAGES "已阅:"
#define RESET_VIEWED_PAGES "重置查看的页面计数器？"

#define INFORMATION "信息"
#define FIRST_FILE_REACHED      "已到达\n目录中的第一个文件"
#define FIRST_FILE_REACHED_LOOP "已到达目录中的第一个文件,\n到达最后一个文件!"
#define FIRST_FILE_REACHED_EXIT "已到达目录中的第一个文件\n退出文件管理器!"
#define LAST_FILE_REACHED      "已到达\n目录中的最后一个文件"
#define LAST_FILE_REACHED_LOOP "已到达目录中的最后一个文件,\n 将第一个文件!"
#define LAST_FILE_REACHED_EXIT "最后一个文件了,\n是否退出文件管理"

#endif
