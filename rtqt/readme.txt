I'm having trouble getting playback through ALSA with Qt. I don't know if it's Qt tying up the resources or simply the way I'm handling output. I need to find out if Qt is blocking the threading, which is very possible.

My idea for this is to do a basic demo using an Qt app that turns on and off the saw wave. If I don't need a new thread, then the real issue in my application is probably my work (which is very likely). If, however, the Qt app DOES prevent the audio, then I have to put RtAudio on another thread.

RtAudio uses pthread_create to pass in an audio callback, and it does uses mutexes for all sorts of things. So it's very possible that the problem is mine, which would be nice.

--------------
After first test, with Qt, the audio did not work.
After the second test, without Qt, the audio still did not work.
Turns out, it was a bug in SoundBox. Audio does play with Qt. Hurray!
