# AsioHookForOsu
## What is this?

This is a programm to enable the osu in-game effect sound output to your ASIO sound device, and it will help to reduce the effect sound lag.

It is realized by inject some dll into the osu game process, hook the sound library "bass.dll", and notice the other process "Inject.exe" to output to ASIO device by using "fmod.dll".

## Note

1. Make sure your sound device hardware support ASIO,  or use software like ASIO4ALL. If you are not sure about if your hardware supporting or having the right environment, you can use "ListDev.exe" to list all ASIO devices.

2. This programm works by some "external hanging" skill, so it may be thought as some kind of "cheat for game", and you should consider the ***risk of getting your osu account banned***. Although the author do not think this is some cheat for game,  the only purpose of it is to reduce the sound output lag.

3. Hope that ppy will add in-game support for ASIO sound devices in osu, then this programm will become no use.

## Install

Download the latest released package and upzip it, follow the guide in readme.txt.

If you use ASIO4ALL, make sure that you have at least 2 different audio output devices. For example, one for the computer's motherboard, another for the monitor. You must choose the osu in-game audio output to another device rather than the ASIO4ALL using. Then the effect sounds and game music will output to different devices (***if you choose the same device for both ASIO4ALL and osu, then it will have no hit in game***).

## Acknowledgement

Thank rustbell for his original programm "KeyASIO".

Thank EmertxE for his patient test.

Tnank https://github.com/XTXTMTXTX/osu-External-ASIO-Sound for some good ideas.

Thank ppy for giving such an excellent game. 



