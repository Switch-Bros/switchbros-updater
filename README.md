# SwitchBros-Updater
<img alt="GitHub" src="https://img.shields.io/github/license/Switch-Bros/switchbros-updater?label=Lizenz&style=plastic"> [![Github Release](https://img.shields.io/github/v/release/Switch-Bros/switchbros-updater?label=Aktuelle%20Version&style=plastic)](https://github.com/Switch-Bros/switchbros-updater) [![Downloads latest](https://img.shields.io/github/downloads/Switch-Bros/switchbros-updater/total?label=Alle%20Downloads&style=plastic)](https://github.com/Switch-Bros/switchbros-updater/releases/tag/2.21.4-SB) [![Chat on Discord](https://img.shields.io/discord/322458533880659969?label=SB%20Discord&style=plastic)](https://discord.com/invite/switchbros)

<p align="center">
<img src = "https://i.imgur.com/R2NRQ1g.jpg"\><br>
</p>

Eine Nintendo Switch Homebrew App um das Switch Bros. Paket, Firmwares und Cheats herunterzuladen und zu aktualisieren.

Basierend auf [kefir-updater](https://github.com/rashevskyv/kefir-updater) von [rashevskyv](https://github.com/rashevskyv) 
Original = [AiO-switch-updater](https://github.com/HamletDuFromage/aio-switch-updater) von [HamletDuFromage](https://github.com/HamletDuFromage)

## Installation
Kopiere den Inhalt der `switchbros-updater.zip`in das 'root' deiner SD-Karte!
Jetzt kannst du im hbmenu (nicht über das Album bitte) die switchbros-updater app starten! 

## Beschreibung der Funktionen
### ⬦ Switch Bros. aktualisieren
- Lade unser [Switch Bros. Basis Paket](https://github.com/Switch-Bros/Switch-Bros.-Pack/releases/tag/1.1.0) herunter. Nach dem herunterladen unseres Pakets, wird es entpackt und deine Switch in die Payload [TegraExplorer](https://github.com/suchmememanyskill/TegraExplorer/releases/) gestartet um das Paket korrekt zu installieren/aktualisieren.
Sollte deine Konsole in der app bleiben und nicht automatisch neu starten, dann halte einfach die POWER-Taste gedrückt (cav5 sek.) und wähle 'Neustart'!

### ⬦ Firmware Downloads
- Lade Firmwares in den `/firmware` Ordner runter und installiere sie mit der App DayBreak.

### ⬦ Cheat Downloads
- Herunterladen und extrahieren täglich aktualisierter Cheats. Die App extrahiert nur die Cheats von Spielen die installiert sind. Standardmäßig, überschreibt diese App vorhandene Cheats. Wenn du deine eigene Cheats hast die unverändert bleiben sollen, dann kannst du Cheat updates für bestimmte Titel in `Tools->Cheat Menü` ausstellen.
- Seit Version 2.0.0, kann aio-switch-updater Cheat Seiten von  [`CheatSlips.com`](https://www.cheatslips.com/) herunterladen. 

## Extras (im Reiter `Tools`)
- Schau nach welche Spiele Cheats besitzen.
- Starte den Internet Browser der Switch.

## Screenshots
<details><summary>Ausklappen um die Screenshots zu sehen</summary>
  
![1](https://user-images.githubusercontent.com/13203024/209908913-be40876e-dbfd-4719-9341-880128932374.jpg)
![2](https://user-images.githubusercontent.com/13203024/209908959-937e87c9-7f2f-40ad-b7c6-78e4ef7b9afc.jpg)
![3](https://user-images.githubusercontent.com/13203024/209908962-de5b431f-6cc8-46da-a35a-d1b1c4f52b35.jpg)
![4](https://user-images.githubusercontent.com/13203024/209908967-599ef915-6f67-4895-a418-d4240ae1a3e7.jpg)
![5](https://user-images.githubusercontent.com/13203024/209908975-091ef5b5-43f3-403d-afc4-5ad0090b2e00.jpg)
![6](https://user-images.githubusercontent.com/13203024/209908978-41e2c7ad-53ba-4449-8d40-c8e143d8d23f.jpg)

</details>

## Kompilieren

<details><summary>Ausklappen um Instruktionen anzuzeigen</summary>

Du musst devkitPro und devkitARM installiert haben um dieses Projekt zu kompilieren.

Installiere die benötigten Zusatzmodule:
```bash
$ sudo (dkp-)pacman -Sy
```
```bash
$ sudo (dkp-)pacman -S  switch-glfw \
                        switch-curl \
                        switch-glad \
                        switch-glm \
                        switch-mbedtls \
                        switch-zlib
```
Use [`switch-ex-curl`](https://github.com/eXhumer/switch-ex-curl) instead of `switch-curl` to use this app with an invalid SSL certificate.

Use portlibs and build with libnx provided in this repository

Repository klonen
```bash
$ git clone --recursive https://github.com/Switch-Bros/TegraExplorer
$ git clone --recursive https://github.com/Switch-Bros/switchbros-updater
$ cd switchbros-updater
```

Kompilieren 
```bash
$ export DEVKITPRO=/opt/devkitpro
$ export DEVKITARM=/opt/devkitpro/devkitARM
$ export DEVKITPPC=/opt/devkitpro/devkitPPC

$ cd sbu-forwarder
$ make -j
$ cd ..
$ make -j
```

</details>

## Haftungsausschluss
Ich besitze, hoste oder vertreibe keine der Dateien, die mit diesem Homebrew-Tool heruntergeladen werden können. Auf Anfrage des Eigentümers werde ich die Möglichkeit, problematische Dateien herunterzuladen, sofort entfernen.

## Besonderer Dank an
- [natinusala](https://github.com/natinusala) für die Borealis Bibliothek.
- [tiansongyu](https://github.com/tiansongyu) für die Unterstützung von mehreren Sprachen und der Chinesischen Übersetzung.
- [yyoossk](https://github.com/yyoossk) für die japanische Übersetzung.
- [sergiou87](https://github.com/sergiou87) für die spanische Übersetzung.
- [pedruhb](https://github.com/pedruhb) für die brasilianische Übersetzung.
- [AD2076](https://github.com/AD2076) für die italienische Übersetzung.
- [qazrfv1234](https://github.com/qazrfv1234) für die Übersetzung in traditionellen chinesisch.
- [Team Neptune](https://github.com/Team-Neptune) dessen rcm code ich nutze.
- [fennectech](https://github.com/fennectech) dabei zu helfen die App zu testen und für seine Vorschläge.
- Iliak für cheatslips.com.

### Gefällt dir die App? Unterstütze den Original Entwickler!
- Liberapay : <a href="https://liberapay.com/HamletDuFromage/donate"><img alt="Spenden mit Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>
- BTC: 1CoFc1bY5AHLP6Noe1zmqnJnp7ZWBxyo79
- ETH: 0xf68f568e21a15934e0e9a6949288c3ca009140ba
- CHAINLINK: 0xf68f568e21a15934e0e9a6949288c3ca009140ba
