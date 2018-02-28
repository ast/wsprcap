project('wsprcap', 'c')

cc = meson.get_compiler('c')

alsa 	 = dependency('alsa')
fftw3f   = dependency('fftw3f')
liquid 	 = cc.find_library('liquid', required : true)
m 		 = cc.find_library('m', required : true)
dispatch = cc.find_library('dispatch', required : true)
blocks   = cc.find_library('BlocksRuntime', required: true)

#inc = include_directories('wsprd')

subdir('wsprd')

sources = ['main.c', 'alsa.c', 'circular_buffer.c']

executable('wsprcap', sources,
	dependencies: [alsa, liquid, dispatch, blocks, m, fftw3f],
    link_with: wsprd
)
