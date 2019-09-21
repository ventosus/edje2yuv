/*
 * Copyright(c) 2015-2019 Hanspeter Portner(dev@open-music-kontrollers.ch)
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the Artistic License 2.0 as published by
 * The Perl Foundation.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * Artistic License 2.0 for more details.
 *
 * You should have received a copy of the Artistic License 2.0
 * along the source as a COPYING file. If not, obtain it from
 * http://www.perlfoundation.org/artistic_license_2_0.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Edje.h>
#pragma GCC diagnostic pop

#include <yuv4mpeg.h>
#include <libswscale/swscale.h>

#define OVERSAMPLING 10

// efl
static Ecore_Evas *ee = NULL;
static Evas *evas = NULL;
static Evas_Object *edj = NULL;
static Ecore_Animator *ea = NULL;
static volatile Eina_Bool done = EINA_FALSE;
static int counter = 0;

// command line arguments
static int test = 0;
static int fps = 25;
static char *in = NULL;
static char *out = "-";
static int width = 1280;
static int height = 720;
static const char *chroma_str = "420mpeg2";
static int chroma = Y4M_UNKNOWN;
static int raw = 0;

// yuv4mpeg
static FILE *stream = NULL;
static y4m_stream_info_t si;
static y4m_frame_info_t fi;

// swscale stuff
static struct SwsContext *dstContext = NULL;
static int srcFormat = AV_PIX_FMT_BGRA;
static int dstFormat = AV_PIX_FMT_YUV420P;
static int srcStride [] = {0, 0, 0, 0};
static int dstStride [] = {0, 0, 0, 0};
static uint8_t *planar420 [3];

static void
init()
{
	ecore_evas_init();
	edje_init();
	y4m_accept_extensions(1);
}

static void
shutdown()
{
	edje_init();
	ecore_evas_init();
}

static Eina_Bool
_dump(void *udata)
{
	if(++counter == OVERSAMPLING) // over sampling
	{
		counter = 0;

		FILE *f = udata;
		int stream_nr = fileno(f);
		const uint8_t *buf = ecore_evas_buffer_pixels_get(ee);

		if(raw)
			fwrite(buf, sizeof(uint8_t), width*height*4, f);
		else
		{
			sws_scale(dstContext, &buf, srcStride, 0, height, planar420, dstStride);
			y4m_write_frame(stream_nr, &si, &fi, planar420);
		}
	}

	return ECORE_CALLBACK_RENEW;
}

static void
_begin_tick(void *udata __attribute__((unused)))
{
	ecore_animator_custom_tick();
}

static void
_end_tick(void *udata __attribute__((unused)))
{
	// do nothing
}

static void
stop(void *udata __attribute__((unused)),
	Evas_Object *edj __attribute__((unused)),
	const char *emission __attribute__((unused)),
	const char *source __attribute__((unused)))
{
	ecore_main_loop_quit();
	done = EINA_TRUE;
}

static void
_version(void)
{
	fprintf(stderr,
		"--------------------------------------------------------------------\n"
		"This is free software: you can redistribute it and/or modify\n"
		"it under the terms of the Artistic License 2.0 as published by\n"
		"The Perl Foundation.\n"
		"\n"
		"This source is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n"
		"Artistic License 2.0 for more details.\n"
		"\n"
		"You should have received a copy of the Artistic License 2.0\n"
		"along the source as a COPYING file. If not, obtain it from\n"
		"http://www.perlfoundation.org/artistic_license_2_0.\n\n");
}

static void
_usage(char **argv)
{
	fprintf(stderr,
		"--------------------------------------------------------------------\n"
		"USAGE\n"
		"   %s -i INPUT [OPTIONS]\n"
		"\n"
		"OPTIONS\n"
		"   [-v]                     print version information\n"
		"   [-i] INPUT               input *.edj file\n"
		"   [-o] OUTPUT              output file or pipe (%s)\n"
		"   [-w] WIDTH               output width (%u)\n"
		"   [-h] HEIGHT              output height (%u)\n"
		"   [-f] FPS                 frames per second (%u)\n"
		"   [-y] CHROMA              chroma 420jpeg|420mpeg2|420paldv|444|422|411|mono|444alpha (%s)\n"
		"   [-r]                     raw RGBA output\n"
		"   [-t]                     testing mode\n\n"
		, argv[0], out, width, height, fps, chroma_str);
}

int
main(int argc, char **argv)
{
	fprintf(stderr,
		"%s "EDJE2YUV_VERSION"\n"
		"Copyright (c) 2015-2019 Hanspeter Portner (dev@open-music-kontrollers.ch)\n"
		"Released under Artistic License 2.0 by Open Music Kontrollers\n",
		argv[0]);

	int c;
	while((c = getopt(argc, argv, "vi:o:w:h:f:y:rt")) != -1)
	{
		switch(c)
		{
			case 'v':
			{
				_version();
			} return 0;
			case 'i':
			{
				in = optarg;
			}	break;
			case 'o':
			{
				out = optarg;
			}	break;
			case 'w':
			{
				width = atoi(optarg);
			}	break;
			case 'h':
			{
				height = atoi(optarg);
			}	break;
			case 'f':
			{
				fps = atoi(optarg);
			}	break;
			case 'y':
			{
				chroma_str = optarg;
			}	break;
			case 't':
			{
				test = 1;
			} break;
			case 'r':
			{
				raw = 1;
			}	break;
			case '?':
			{
				if((optopt == 'i') ||(optopt == 'o') ||(optopt == 'w')
						||(optopt == 'h') ||(optopt == 'f') ||(optopt == 'y'))
					fprintf(stderr, "Option `-%c' requires an argument.\n", optopt);
				else if(isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			}	return 1;
		}
	}

	if(!in)
	{
		_usage(argv);
		return 1;
	}

	chroma = y4m_chroma_parse_keyword(optarg);
	fprintf(stderr, "chroma: %s\n", y4m_chroma_description(chroma));
	if(chroma == Y4M_UNKNOWN)
	{
		chroma = Y4M_CHROMA_420MPEG2;
	}

	fprintf(stderr, "width: %i\nheight: %i\nfps: %i\nraw: %i\nyuv: %s:\nin: %s\n"
		"out: %s\ntest: %i\n\n",
		width, height, fps, raw, y4m_chroma_description(chroma), in, out, test);  
	init();

	if(!test)
		ee = ecore_evas_buffer_new(width, height);
	else
		ee = ecore_evas_software_x11_new("", 0, 0, 0, width, height);

	ecore_evas_title_set(ee, "Edje2YUV");
	ecore_evas_show(ee);
	evas = ecore_evas_get(ee);
	edj = edje_object_add(evas);

	edje_object_file_set(edj, in, "main");
	evas_object_resize(edj, width, height);
	evas_object_show(edj);

	edje_object_signal_emit(edj, "start", "");
	edje_object_signal_callback_add(edj, "stop", "*", stop, NULL);
	edje_frametime_set(1.0 /(double) fps);

	if(!test)
	{
		if(!strcmp(out, "-"))
			stream = stdout;
		else
			stream = fopen(out, "w");

		int stream_nr = fileno(stream);

		// efl
		if(!raw)
		{
			// yuv4mpeg
			y4m_ratio_t rt = {fps, 1};

			y4m_init_stream_info(&si);
			y4m_si_set_width(&si, width);
			y4m_si_set_height(&si, height);
			y4m_si_set_interlace(&si, Y4M_ILACE_NONE);
			y4m_si_set_framerate(&si, rt);
			y4m_si_set_chroma(&si, chroma);
			y4m_write_stream_header(stream_nr, &si);

			y4m_init_frame_info(&fi);
			y4m_fi_set_presentation(&fi, Y4M_PRESENT_PROG_SINGLE);
			y4m_fi_set_spatial(&fi, Y4M_SAMPLING_PROGRESSIVE);
			y4m_fi_set_temporal(&fi, Y4M_SAMPLING_PROGRESSIVE);

			// swscale
			srcStride[0] = 4*width;

			switch(chroma)
			{
				case Y4M_CHROMA_420JPEG:
					dstFormat = AV_PIX_FMT_YUVJ420P;
					dstStride[0] = width;
					dstStride[1] = width/2;
					dstStride[2] = width/2;
					break;
				case Y4M_CHROMA_420PALDV:
					//TODO
				case Y4M_CHROMA_420MPEG2:
					dstFormat = AV_PIX_FMT_YUV420P;
					dstStride[0] = width;
					dstStride[1] = width/2;
					dstStride[2] = width/2;
					break;
				case Y4M_CHROMA_444:
					dstFormat = AV_PIX_FMT_YUV444P;
					dstStride[0] = width;
					dstStride[1] = width;
					dstStride[2] = width;
					break;
				case Y4M_CHROMA_422:
					dstFormat = AV_PIX_FMT_YUV422P;
					dstStride[0] = width;
					dstStride[1] = width/2;
					dstStride[2] = width/2;
					break;
				case Y4M_CHROMA_411:
					dstFormat = AV_PIX_FMT_YUV411P;
					dstStride[0] = width;
					dstStride[1] = width/4;
					dstStride[2] = width/4;
					break;
				case Y4M_CHROMA_MONO:
					dstFormat = AV_PIX_FMT_GRAY8;
					dstStride[0] = width;
					break;
				case Y4M_CHROMA_444ALPHA:
					//TODO
					dstFormat = AV_PIX_FMT_YUV444P;
					dstStride[0] = width;
					dstStride[1] = width;
					dstStride[2] = width;
					dstStride[3] = width;
					break;
			}

			dstContext = sws_getCachedContext(dstContext, width, height, srcFormat,
				width, height, dstFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);
			if(!dstContext)
			{
				fprintf(stderr, "Failed to get %i ---> %i\n", srcFormat, dstFormat);
				exit(-1);
			}

			uint8_t *data = malloc(3*width*height);
			planar420[0] = data;
			planar420[1] = data + width*height;
			planar420[2] = data + width*height*2;
		}

		ea = ecore_animator_add(_dump, stream);
		ecore_animator_custom_source_tick_begin_callback_set(_begin_tick, stream);
		ecore_animator_custom_source_tick_end_callback_set(_end_tick, stream);
		ecore_animator_source_set(ECORE_ANIMATOR_SOURCE_CUSTOM);

		ecore_main_loop_iterate();
		double lt = ecore_loop_time_get();

		const double step = 1.0 / OVERSAMPLING / fps;

		while(done == EINA_FALSE)
		{
			if(ecore_main_loop_animator_ticked_get() == EINA_TRUE)
			{
				lt += step;
				ecore_loop_time_set(lt);
				ecore_animator_custom_tick();
			}

			ecore_loop_time_set(lt);
			ecore_main_loop_iterate();
		}
	}
	else
		ecore_main_loop_begin();

	if(!test)
	{
		ecore_animator_del(ea);

		if(strcmp(out, "-"))
			fclose(stream);

		if(!raw)
		{
			free(planar420[0]);

			y4m_fini_stream_info(&si);
			y4m_fini_frame_info(&fi);
		}
	}

	evas_object_del(edj);
	ecore_evas_free(ee);

	shutdown();

	return 0;
}
