# AsioHookForOsu
## What is this?

This is a programm to enable the osu in-game effect sound output to your ASIO sound device, and it will help to reduce the effect sound lag due to the hardware support.

It is realized by inject some dll into the osu game process, hook the sound library "bass.dll", and notice the other process "Inject.exe" to output to ASIO device by using "fmod.dll".

## Note

1. Make sure your sound device hardware support ASIO,  a virtual ASIO device simulated by software may work with this programm, but it gives no help with the sound output lag. If you are not sure about if your hardware supporting, you can use "ListDev.exe" to list all ASIO devices.

2. This programm works by some "external hanging" skill, so it may be thought as some kind of "cheat for game", and you should consider the ***risk of getting your osu account banned***. Although the author do not think this is some cheat for game,  the only purpose of it is to reduce the sound output lag.

3. Hope that ppy will add in-game support for ASIO sound devices in osu, then this programm will become no use.

## Install

Download the latest released package and upzip it, follow the guide in readme.txt.

You should set the process priority of "Inject.exe" to "High" or "Real Time" (you should already set the process priority of "osu!.exe" to "High" or "Real Time" if you find this programm useful), this will reduce the lag due to interprocess communication. 

## Acknowledgement

Thank rustbell for his original programm "KeyASIO".

Thank EmertxE for his patient test.

Thank ppy for giving such an excellent game. 



