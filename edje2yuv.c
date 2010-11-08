/*
 *  edje2yuv.c: A program to export Edje programs (layout/animation library
 *  of the Enlightenment foundation libraries) to a yuv4mpeg stream for
 *  further integration into films or presentations.
 *
 *
 *  Copyright (C) 2010 Hanspeter Portner <ventosus@airpost.net>
 *
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Edje.h>

#include <yuv4mpeg.h>
#include <libswscale/swscale.h>

// efl
Ecore_Evas *ee = NULL;
Evas *evas = NULL;
Evas_Object *edj = NULL;
Ecore_Animator *ea = NULL;

// command line arguments
int test = 0;
int fps = 25;
int scale = 1;
char *in = NULL;
char *out = "-";
char PATH[256];
int width = 0;
int height = 0;

// yuv4mpeg
FILE *stream = NULL;
y4m_stream_info_t si;
y4m_frame_info_t fi;

// swscale stuff
struct SwsContext *srcContext, *dstContext = NULL;
int srcFormat = PIX_FMT_BGRA;
int dstFormat = PIX_FMT_YUV420P;
int srcStride [] = {0, 0, 0};
int dstStride [] = {0, 0, 0};
uint8_t *planar420 [3];

void
init ()
{
   ecore_init ();
   ecore_evas_init ();
   evas_init ();
   edje_init ();
}

void
shutdown ()
{
   edje_shutdown ();
   evas_shutdown ();
   ecore_evas_shutdown ();
   ecore_shutdown ();
}

void
abort_ (const char *s, ...)
{
   va_list args;
   va_start (args, s);
   vfprintf (stderr, s, args);
   fprintf (stderr, "\n");
   va_end (args);
   abort ();
}

int
dump (void *udata)
{
  int stream_nr = fileno (udata);
  const uint8_t * buf = ecore_evas_buffer_pixels_get (ee);
  const uint8_t *src [3] = {buf, NULL, NULL};

  sws_scale(dstContext, src, srcStride, 0, height, planar420, dstStride);
  y4m_write_frame (stream_nr, &si, &fi, planar420);

  return 1;
}

void
stop (void *udata, Evas_Object * edj, const char *emission,
      const char *source)
{
   ecore_main_loop_quit ();
}

int
main (int argc, char **argv)
{
   int c;
   while ( (c = getopt (argc, argv, "i:o:w:h:f:s:t")) != -1)
     switch (c)
       {
	case 'i':
	   in = optarg;
	   break;
	case 'o':
	   out = optarg;
	   break;
	case 'w':
	   width = atoi (optarg);
	   break;
	case 'h':
	   height = atoi (optarg);
	   break;
	case 't':
	   test = 1;
	   break;
	case 'f':
	   fps = atoi (optarg);
	   break;
	case 's':
	   scale = atoi (optarg);
	   break;
	case '?':
	   if ( (optopt == 'i') || (optopt == 'o') || (optopt == 'w') || (optopt == 'h') || (optopt == 'f'))
	     fprintf (stderr, "Option `-%c' requires an argument.\n", optopt);
	   else if (isprint (optopt))
	     fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	   else
	     fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
	   return 1;
	default:
	   abort ();
       }

   if (!in || !width || !height)
     {
	fprintf (stderr, "usage: %s -i INPUT -w WIDTH -h HEIGHT [-o OUTPUT] [-t] [-f FPS] [-s SCALE]\n\n", argv[0]);
	abort ();
     }
   
   init ();
  
   if (!test)
     ee = ecore_evas_buffer_new (width, height);
   else
     ee = ecore_evas_software_x11_new ("", 0, 0, 0, width, height);
   ecore_evas_show (ee);
   evas = ecore_evas_get (ee);
   edj = edje_object_add (evas);
   
   edje_object_file_set (edj, in, "main");
   evas_object_resize (edj, width, height);
   evas_object_show (edj);
   
   edje_object_signal_emit (edj, "start", "");
   edje_object_signal_callback_add (edj, "stop", "*", stop, NULL);
  
   if (!test)
   {
     if (!strcmp (out, "-"))
       stream = stdout;
     else
       stream = fopen (out, "w");
     int stream_nr = fileno (stream);

     // efl
     ea = ecore_animator_add (dump, stream);
     edje_frametime_set (1.0 / (double) fps);
     ecore_animator_frametime_set (1.0 / (double) fps);

     // yuv4mpeg
     y4m_ratio_t rt = {fps, 1};

     y4m_init_stream_info (&si);
     y4m_si_set_width (&si, width);
     y4m_si_set_height (&si, height);
     y4m_si_set_interlace (&si, Y4M_ILACE_NONE);
     y4m_si_set_framerate (&si, rt);
     y4m_si_set_chroma (&si, Y4M_CHROMA_420MPEG2);
     y4m_write_stream_header (stream_nr, &si);

     y4m_init_frame_info (&fi);
     y4m_fi_set_presentation (&fi, Y4M_PRESENT_PROG_SINGLE);
     y4m_fi_set_spatial (&fi, Y4M_SAMPLING_PROGRESSIVE);
     y4m_fi_set_temporal (&fi, Y4M_SAMPLING_PROGRESSIVE);
    
     // swscale
     srcStride[0] = 4*width;
     dstStride[0] = width;
     dstStride[1] = width/2;
     dstStride[2] = width/2;
     dstContext= sws_getContext (width, height, srcFormat, width, height, dstFormat, SWS_GAUSS, NULL, NULL, NULL);
     if (!dstContext)
     {
       fprintf(stderr, "Failed to get %s ---> %s\n",
	   sws_format_name(srcFormat),
	   sws_format_name(dstFormat));
       exit (-1);
     }
     
     uint8_t *data = malloc (3*width*height);
     planar420[0] = data;
     planar420[1] = data + width*height;
     planar420[2] = data + width*height*2;
   }
   else
   {
     edje_frametime_set (1.0 / (double) fps);
     ecore_animator_frametime_set ((double) scale / (double) fps);
   }
   
   ecore_main_loop_begin ();
  
   if (!test)
   {
     free (planar420[0]);

     y4m_fini_stream_info (&si);
     y4m_fini_frame_info (&fi);

     if (strcmp (out, "-"))
       fclose (stream);
   }
  
   if (ea)
     ecore_animator_del (ea);
   evas_object_del (edj);
   evas_free (evas);
   ecore_evas_free (ee);
   
   shutdown ();
   
   return 0;
}
