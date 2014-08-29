WIDTH=800
HEIGHT=200
FPS=60

all:	sample.edj

sample.edj:	sample.edc
	edje_cc -id cc $< $@

sample.y4m:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y mono -o $@

sample.mkv:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y 420mpeg2 | ffmpeg -y -f yuv4mpegpipe -i - -vcodec png $@

sample.pipe:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -y 411 | ffplay -autoexit -

sample.raw.pipe:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -r | \
	  ffmpeg -y -s $(WIDTH)x$(HEIGHT) -r $(FPS) -f rawvideo -vcodec rawvideo -pix_fmt rgba -i - -f yuv4mpegpipe -pix_fmt yuv420p - | \
	  ffplay -autoexit -

sample.raw:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -r -o $@

sample:	sample.edj
	edje2yuv -i $< -w $(WIDTH) -h $(HEIGHT) -f $(FPS) -t

clean:
	rm -f sample.edj sample.mkv sample.y4m sample.raw
