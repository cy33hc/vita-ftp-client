# vita-ftp-client
Simple FTP Client App for Vita

![Preview](/screenshot/preview.jpg)

# Requirements
The app is built with imgui-vita which needs the **libshacccg.suprx** extracted. If you are experiencing a crash after upgrading to 1.21. Please follow the following guide to get that extracted.

As of v1.30 the app will no longer crash, but display a message that you are missing the libshacccg.suprx files.


https://samilops2.gitbook.io/vita-troubleshooting-guide/shader-compiler/extract-libshacccg.suprx

# Know Issues
Opening the Input Editor multiple times crashes the application. I can't figure out what's wrong. If somebody knows what's the problem. Help me out.

# Multi Language support
The appplication can auto-detect the following standard PSVITA languages
```
Japanese
English
French
Spanish
German
Italian
Dutch
Portuguese_BR
Protugese_PT
Russian
Korean
Chinese
Polish
```

For **Non-Standard** Languages like **Croatian, Catalan, Euskera, Galego, Hungarian, Indonesian**

User must modify the file **ux0:data/FTPCLI0001/config.ini** and update the **language** setting to Croatian, Catalan, Euskera, Galego or Indonesian

# Credits
Thx to the following people for translating the string used in the app.
@jojahn @dampestwriter @dinckelman @TheFrutz @hiroSzymon @Fiodorwellfme @AndreDK7 @IlDucci @kuragehimekurara1 @Qingyu510 

Borrowed the Theme colors and Folder/File icon from NXShell. https://github.com/joel16/NX-Shell/releases

Copied some of the language code from yoyoloader https://github.com/Rinnegatamante/yoyoloader_vita

Also thx to @Rinnegatamante for imgui-vita library
