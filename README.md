# edje2yuv

A program to export Edje programs (layout/animation library
of the Enlightenment foundation libraries) to a yuv4mpeg2 or ARGB stream for
further processing with e.g. FFmpeg and integration into films or presentations.

### Build status

[![build status](https://gitlab.com/OpenMusicKontrollers/edje2yuv/badges/master/build.svg)](https://gitlab.com/OpenMusicKontrollers/edje2yuv/commits/master)

### About

### Dependencies

* [Efl](http://docs.enlightenment.org/auto/efl/) (Enlightenment Foundation Libraries)
* [mjpegtools](http://mjpeg.sourceforge.net/) (MJPEG Tools)

### Build Dependencies

#### Debian/Ubuntu

	sudo apt-get install libefl-dev libswscale-dev libmjpegtools-dev

#### ArchLinux

	sudo pacman -S efl ffmpeg mjpegtools

### Build / install

	git clone https://github.com/ventosus/edje2yuv.git
	cd edje2yuv 
	meson build
	cd build
	ninja -j4
	sudo ninja install

### License

Copyright (c) 2015-2019 Hanspeter Portner (dev@open-music-kontrollers.ch)

This is free software: you can redistribute it and/or modify
it under the terms of the Artistic License 2.0 as published by
The Perl Foundation.

This source is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
Artistic License 2.0 for more details.

You should have received a copy of the Artistic License 2.0
along the source as a COPYING file. If not, obtain it from
<http://www.perlfoundation.org/artistic_license_2_0>.
