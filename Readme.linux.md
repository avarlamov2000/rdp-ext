# Сборка и запуск в Linux

## Требования
- Ubuntu 24.04 Client (или Server редакция с установленным пакетом `ubuntu-desktop`)
- Visual Studio 2022 или MS Build Tools с поддержкой C++
- Windows SDK
- Boost

## Установка зависимостей
Команда для "чистой" Ubuntu:

`sudo apt install -y build-essential git cmake clang clang-tools zlib1g-dev ninja-build pkg-config freerdp3-dev libwinpr3-dev freerdp3-x11 xrdp xorgxrdp dbus-x11 libboost-dev`

## Настройка xrdp
Опционально, пропустить если уже всё настроено
```bash
sudo systemctl enable --now xrdp
sudo adduser xrdp ssl-cert
sudo systemctl restart xrdp
sudo systemctl restart xrdp-sesman
```

## Установка и настройка xfce
Опционально, пропустить если уже всё настроено

Как известно, системный GNOME очень тяжеловесный. Рекомендуется воспользоваться легким xfce4 для работы через xrdp
```bash
sudo apt install -y xfce4
echo xfce4-session > ~/.xsession
chmod 644 ~/.xsession
sudo systemctl restart xrdp xrdp-sesman
```

## Конфигурация
`cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`

## Сборка
`cmake --build build --config Release --target rdp_ext_all`

## Артефакты
В папке `/bin/Release` build директории должны появиться следующие файлы:
- Плагин для freerdp: `librdpext-client.so`
- Исполняемый файл для сервера: `xrdp_server`

## Запуск

### FreeDRP плагин
Для загрузки плагина в FreeRDP необходимо скопировать плагин в папку плагинов FreeRDP или сделать symlink (что проще для отладки).  Пример команды:
```bash
sudo mkdir /usr/lib/x86_64-linux-gnu/freerdp3
sudo ln -sf /home/user/rdp-ext/build/bin/librdpext-client.so /usr/lib/x86_64-linux-gnu/freerdp3/librdpext-client.so
```

При запуске FreeRDP обязательно нужно указать параметр `/vc:rdpext`, только в этом случае он загрузит плагин librdpext-client.so. Пример:

`xfreerdp3 /v:192.168.1.100 /u:user /p:pass /cert:ignore /log-level:INFO /size:1920x1080 /bpp:24 /vc:rdpext`

### Серверная часть
Серверная часть поставляется в виде консольного файла. Его достаточно скопировать через RDP на серверную машину и просто запустить
