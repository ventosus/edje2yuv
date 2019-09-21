project('edje2yuv', 'c', default_options : [
	'buildtype=release',
	'warning_level=3',
	'werror=false',
	'b_lto=false',
	'c_std=gnu11'])

version = run_command('cat', 'VERSION').stdout().strip()

add_project_arguments('-DEDJE2YUV_VERSION="'+version+'"', language : 'c')
add_project_arguments('-D_GNU_SOURCE', language : 'c')

conf_data = configuration_data()
cc = meson.get_compiler('c')

eina_dep = dependency('eina', version : '>=1.11')
ecore_dep = dependency('ecore', version : '>=1.11')
ecore_evas_dep = dependency('ecore-evas', version : '>=1.11')
evas_dep = dependency('evas', version : '>=1.11')
edje_dep = dependency('edje', version : '>=1.11')
mjpeg_dep = dependency('mjpegtools', version : '>=2.1')
swscale_dep = dependency('libswscale', version : '>=3.1')

deps = [
	eina_dep, ecore_dep, ecore_evas_dep, evas_dep, edje_dep, mjpeg_dep, swscale_dep
]

srcs = [
	'edje2yuv.c'
]

executable('edje2yuv', srcs,
	dependencies : deps,
	install : true)

install_man('edje2yuv.1')
