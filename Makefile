ECORE_OPTS = `pkg-config --cflags --libs ecore`
ECORE_EVAS_OPTS = `pkg-config --cflags --libs ecore-evas`
EVAS_OPTS = `pkg-config --cflags --libs evas`
EDJE_OPTS = `pkg-config --cflags --libs edje`
YUV_OPTS = `pkg-config --cflags --libs mjpegtools`
SW_OPTS = `pkg-config --cflags --libs libswscale`
OPTS = $(ECORE_OPTS) $(ECORE_EVAS_OPTS) $(EVAS_OPTS) $(EDJE_OPTS) $(PNG_OPTS) $(YUV_OPTS) $(SW_OPTS)

WIDTH=800
HEIGHT=200
FPS=60

all:	edje2yuv sample.edj

edje2yuv:	edje2yuv.c
	gcc -g -o $@ $< $(OPTS)

sample.edj:	sample.edc
	edje_cc -id cc $< $@

sample.y4m:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y mono -o sample.y4m

sample.mkv:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y 420mpeg2 | ffmpeg -y -f yuv4mpegpipe -i - -b 5000000 sample.mkv

sample.pipe:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y 411 | ffplay -autoexit -

sample.raw.pipe:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -r | \
	  ffmpeg -y -s $(WIDTH)x$(HEIGHT) -r $(FPS) -f rawvideo -vcodec rawvideo -pix_fmt rgba -i - -f yuv4mpegpipe -pix_fmt yuv420p - | \
	  ffplay -autoexit -

sample.raw:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -o sample.raw -r

sample:	edje2yuv sample.edj
	./edje2yuv -i sample.edj -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -t

clean:
	rm -f edje2yuv sample.edj sample.mkv sample.y4m sample.raw

