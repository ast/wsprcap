project('wsprcap', 'c')

cc = meson.get_compiler('c')

alsa 	 = dependency('alsa')
liquid 	 = cc.find_library('liquid', required : true)
m 		 = cc.find_library('m', required : true)
dispatch = cc.find_library('dispatch', required : true)
blocks   = cc.find_library('BlocksRuntime', required: true)

sources = ['main.c', 'alsa.c', 'circular_buffer.c']

executable('wsprcap',
	sources,
	dependencies: [alsa, liquid, dispatch, blocks, m]
)
