# Сборка и запуск в Windows

## Требования
- Windows 10/11 или Windows Server
- Visual Studio 2022 или MS Build Tools с поддержкой C++
- Windows SDK
- Boost

## Настройка Boost
Необходимо скачать boost libraries с официального сайта или с гитхаб https://github.com/boostorg/boost/releases и распаковать его

## Конфигурация
Из Developer Command Prompt VS 2022 запустить команду

`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRDP_EXT_BOOST_ROOT="path_to_unpacked_boost"`

Например, если boost установлен в `C:\boost\boost_1_90_0`, то строка запуска будет

`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRDP_EXT_BOOST_ROOT="C:\boost\boost_1_90_0"`

## Сборка
Из Developer Command Prompt VS 2022 запустить команду

`cmake --build build --config Release --target rdp_ext_all`

## Артефакты
В папке `/bin/Release` build директории должны появиться следующие файлы:
- Плагин для mstsc: `mstsc_plugin.dll`
- Исполняемый файл для сервера: `wts_server.exe`

## Запуск

### MSTSC плагин
Для загрузки плагина в mstsc необходимо добавить запись о плагине в реестр Windows. Пример команды:

`reg add "HKLM\Software\Microsoft\Terminal Server Client\Default\Addins\RDPEXT" /v Name /t REG_SZ /d "с:\rdp-ext\build\bin\mstsc_plugin.dll" /f`

Т.к. mstsc не консольное приложение, логи клиента будут доступны в Debug Log. Посмотреть его можно при помощи программы DebugView https://learn.microsoft.com/en-us/sysinternals/downloads/debugview

### Серверная часть
Серверная часть поставляется в виде консольного exe файла. Его достаточно скопировать через RDP на серверную машину и просто запустить
