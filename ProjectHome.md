# A tiny server for a [squeezebox](http://slimdevices.com) #

This is a tiny replacement for [squeezeCenter](http://www.slimdevices.com/pi_features.html), the default server for squeezeboxes. This server will never match squeezeCenter in terms of features, but since it is much smaller and doesn't requires perl, it can be run on NAS devices and routers.

Squeezed currently provides basic playlist control (add,play,pause,forward,rewind) and limited searching options for your music collection.

## News ##

11/08/2009:  The summer is over, time for an [update](http://squeezed.googlecode.com/files/squeezed.zip). SqueezeD can now be controlled over a web-browser.

The HTML-code is very primitive, so if anyone is good in that, let me know.
The linux version has not been tested, so that might not work.

## Features and limitations ##

  * It can browse for files, or search for artist/album using the remote.
  * It can control the playlist from the remote and (a bit) from the web-gui
  * Supports mp3 and flac music files and also shoutcast mp3-streams.
  * The menu is animated, visualization (spectrum analyzer) is supported
  * There is (basic) control over a web-browser [http://127.0.0.1:9000/html/index.html](http://127.0.0.1:9000/html/index.html)
  * It has it bugs, I just haven't found them yet
  * The search functionality is limited

## Compilation ##

### Posix / Linux ###

This program currently requires the pthread and taglib libraries. By commenting
out `CONFIG=-DUSE_TAGLIB` in `Makefile`, it can compile without taglib. It then only
supports audio files which have a ID3v2 tag.

A Makefile is provided. just type 'make' and it should compile without any problems.

### Windows ###

For windows, there's a visual studio 2008 project-file in the `msdev/` subdirectory. By default, it links to a static 'taglib' library, for which another project-file is provided
in the `taglib` subdirectory. That taglib subdirectory only contains modifications required to link it statically.

In order to compile, You'll have to put all the files of [taglib](http://developer.kde.org/~wheeler/files/src/taglib-1.5.tar.gz) there, and then make sure the modifications I checked in are still there. (svn revert on the taglib directory should be able to do that).


## Supported platforms ##

The source has been compiled and tested for:

  * Windows xp, x86 cpu, 32-bit
  * Linux, x86 cpu, 32-bit
  * Linux, x86 cpu, 64-bit
  * Linux, powerpc, 64-bit

I have uploaded [binaries](http://squeezed.googlecode.com/files/squeezed.zip) for some of these platforms, but they are usually a bit out of date. See the [updates](http://code.google.com/p/squeezed/updates/list) for more information.

It has been compiled for [openWRT](http://www.openwrt.org) also, for the brcm47xx mips platform. The current [binary](http://squeezed.googlecode.com/files/squeezed.zip) does not work, You'll have to build it from source.


## Installation and Usage ##

Download a [executable](http://squeezed.googlecode.com/files/squeezed.zip). Run it once to generate a configuration file named `squeezed.ini`.
The most important option to change is:

```
[musicDB]
path = .
```

This defines the root directory of you music collection. Whenever your collection has changed, you should delete `SqueezeD.db` and restart the server.

When the server is running, a (very) basic webserver can be found at [http://127.0.0.1:9000/html/index.html](http://127.0.0.1:9000/html/index.html).

## The big whish list and other todo's ##

  * The code runs in 2 threads, multi-threading-safety has not been tested that good.

  * Improved browser-frontend. Much can be done by just improving the html code.

  * Better searching. The whole sorting/searching-code can be much more compact.

  * Fancy display features: scrolling text, song position indicator and much more

  * Support of streaming from [last.fm](http://last.fm)

  * In general: more features that [squeezeCenter](http://www.slimdevices.com/pi_features.html) has.

  * ~~A larger font, currently it only has an 8x8 font~~ (done)

  * ~~Use [taglib](http://developer.kde.org/~wheeler/taglib.html) for audiofile detection instead of the built-in code. That would allow other file-formats besides mp3~~ (done)


