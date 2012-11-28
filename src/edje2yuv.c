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
Ecore_Timer *ea = NULL;

// command line arguments
int test = 0;
int fps = 25;
int scale = 1;
char *in = NULL;
char *out = "-";
char PATH[256];
int width = 0;
int height = 0;
int chroma = Y4M_CHROMA_420MPEG2;
int raw = 0;

// yuv4mpeg
FILE *stream = NULL;
y4m_stream_info_t si;
y4m_frame_info_t fi;

// swscale stuff
struct SwsContext *dstContext = NULL;
int srcFormat = PIX_FMT_BGRA;
int dstFormat = PIX_FMT_YUV420P;
int srcStride [] = {0, 0, 0, 0};
int dstStride [] = {0, 0, 0, 0};
uint8_t *planar420 [3];

void
init ()
{
   ecore_evas_init ();
   edje_init ();
   y4m_accept_extensions (1);
}

void
shutdown ()
{
   edje_init ();
   ecore_evas_init ();
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

Eina_Bool
dump (void *udata)
{
  //edje_object_calc_force (edj);
  edje_object_play_set (edj, EINA_FALSE);
  ecore_timer_freeze (ea);

  ecore_main_loop_iterate ();

  FILE *f = udata;
  int stream_nr = fileno (f);
  const uint8_t *buf = ecore_evas_buffer_pixels_get (ee);

  if (raw)
    fwrite (buf, sizeof (uint8_t), width*height*4, f);
  else
  {
    sws_scale (dstContext, &buf, srcStride, 0, height, planar420, dstStride);
    y4m_write_frame (stream_nr, &si, &fi, planar420);
  }

  ecore_timer_thaw (ea);
  edje_object_play_set (edj, EINA_TRUE);

  return EINA_TRUE;
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
   while ( (c = getopt (argc, argv, "i:o:w:h:f:y:rt")) != -1)
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
	case 'f':
	   fps = atoi (optarg);
	   break;
	case 'y':
	   chroma = y4m_chroma_parse_keyword (optarg);
	   fprintf (stderr, "chroma: %s\n", y4m_chroma_description (chroma));
	   if (chroma == Y4M_UNKNOWN)
	     chroma = Y4M_CHROMA_420MPEG2;
	   break;
	case 't':
	   test = 1;
	   break;
	case 'r':
	   raw = 1;
	   break;
	case '?':
	   if ( (optopt == 'i') || (optopt == 'o') || (optopt == 'w') || (optopt == 'h') || (optopt == 'f') || (optopt == 'y'))
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
	fprintf (stderr, "usage: %s -i INPUT -w WIDTH -h HEIGHT [-o OUTPUT] [-t] [-f FPS] [-y 420jpeg|420mpeg2|420paldv|444|422|411|mono|444alpha] [-r]\n\n", argv[0]);
	abort ();
     }

   fprintf (stderr, "width: %i\nheight: %i\nfps: %i\nraw: %i\nyuv: %s:\nin: %s\nout: %s\ntest: %i\n\n",
       width, height, fps, raw, y4m_chroma_description (chroma), in, out, test);
   
   init ();
  
   if (!test)
     ee = ecore_evas_buffer_new (width, height);
   else
     ee = ecore_evas_software_x11_new ("", 0, 0, 0, width, height);
   ecore_evas_title_set (ee, "Edje2YUV");
   ecore_evas_show (ee);
   evas = ecore_evas_get (ee);
   edj = edje_object_add (evas);
   
   edje_object_file_set (edj, in, "main");
   evas_object_resize (edj, width, height);
   evas_object_show (edj);

   edje_object_signal_emit (edj, "start", "");
   edje_object_signal_callback_add (edj, "stop", "*", stop, NULL);
   edje_frametime_set (1.0 / (double) fps);
  
   if (!test)
   {
     if (!strcmp (out, "-"))
       stream = stdout;
     else
       stream = fopen (out, "w");
     int stream_nr = fileno (stream);

     // efl
     ea = ecore_timer_add (1.0 / (double) fps, dump, stream);

     if (!raw)
     {
       // yuv4mpeg
       y4m_ratio_t rt = {fps, 1};

       y4m_init_stream_info (&si);
       y4m_si_set_width (&si, width);
       y4m_si_set_height (&si, height);
       y4m_si_set_interlace (&si, Y4M_ILACE_NONE);
       y4m_si_set_framerate (&si, rt);
       y4m_si_set_chroma (&si, chroma);
       y4m_write_stream_header (stream_nr, &si);

       y4m_init_frame_info (&fi);
       y4m_fi_set_presentation (&fi, Y4M_PRESENT_PROG_SINGLE);
       y4m_fi_set_spatial (&fi, Y4M_SAMPLING_PROGRESSIVE);
       y4m_fi_set_temporal (&fi, Y4M_SAMPLING_PROGRESSIVE);
      
       // swscale
       srcStride[0] = 4*width;

       switch (chroma)
       {
	 case Y4M_CHROMA_420JPEG:
	   dstFormat = PIX_FMT_YUVJ420P;
	   dstStride[0] = width;
	   dstStride[1] = width/2;
	   dstStride[2] = width/2;
	   break;
	 case Y4M_CHROMA_420PALDV:
	   //TODO
	 case Y4M_CHROMA_420MPEG2:
	   dstFormat = PIX_FMT_YUV420P;
	   dstStride[0] = width;
	   dstStride[1] = width/2;
	   dstStride[2] = width/2;
	   break;
	 case Y4M_CHROMA_444:
	   dstFormat = PIX_FMT_YUV444P;
	   dstStride[0] = width;
	   dstStride[1] = width;
	   dstStride[2] = width;
	   break;
	 case Y4M_CHROMA_422:
	   dstFormat = PIX_FMT_YUV422P;
	   dstStride[0] = width;
	   dstStride[1] = width/2;
	   dstStride[2] = width/2;
	   break;
	 case Y4M_CHROMA_411:
	   dstFormat = PIX_FMT_YUV411P;
	   dstStride[0] = width;
	   dstStride[1] = width/4;
	   dstStride[2] = width/4;
	   break;
	 case Y4M_CHROMA_MONO:
	   dstFormat = PIX_FMT_GRAY8;
	   dstStride[0] = width;
	   break;
	 case Y4M_CHROMA_444ALPHA:
	   //TODO
	   dstFormat = PIX_FMT_YUV444P;
	   dstStride[0] = width;
	   dstStride[1] = width;
	   dstStride[2] = width;
	   dstStride[3] = width;
	   break;
       }

       dstContext = sws_getCachedContext (dstContext, width, height, srcFormat, width, height, dstFormat, SWS_FAST_BILINEAR, NULL, NULL, NULL);
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
   }
   
   ecore_main_loop_begin ();
  
   if (!test)
   {
     if (strcmp (out, "-"))
       fclose (stream);

     if (!raw)
     {
       free (planar420[0]);

       y4m_fini_stream_info (&si);
       y4m_fini_frame_info (&fi);
     }

     ecore_timer_del (ea);
   }
  
   evas_object_del (edj);
   ecore_evas_free (ee);
   
   shutdown ();
   
   return 0;
}
