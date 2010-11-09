ECORE_OPTS = `pkg-config --cflags --libs ecore`
ECORE_EVAS_OPTS = `pkg-config --cflags --libs ecore-evas`
EVAS_OPTS = `pkg-config --cflags --libs evas`
EDJE_OPTS = `pkg-config --cflags --libs edje`
PNG_OPTS = `pkg-config --cflags --libs libpng`
YUV_OPTS = `pkg-config --cflags --libs mjpegtools`
SW_OPTS = `pkg-config --cflags --libs libswscale`
OPTS = $(ECORE_OPTS) $(ECORE_EVAS_OPTS) $(EVAS_OPTS) $(EDJE_OPTS) $(PNG_OPTS) $(YUV_OPTS) $(SW_OPTS)

all:	edje2yuv

edje2yuv:	edje2yuv.c
	gcc -g -o $@ $< $(OPTS)

sample.edj:	sample.edc
	edje_cc -id cc $< $@

sample.mkv:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w 800 -h 200 -f 25 | ffmpeg -y -f yuv4mpegpipe -i - -b 5000000 sample.mkv

test:	sample.mkv

clean:
	rm -f edje2yuv sample.edj sample.mkv

