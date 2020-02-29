g++ --std=c++11 main.cpp app.cpp ../rtaudio/RtAudio.cpp ../Ding/src/soundbox/SoundBox.cpp -I../rtaudio -I../Ding/src/soundbox -lasound -lpthread -D__LINUX_ALSA__ -o rtqtm.out
