//
//  alsa.h
//  fcdsdr
//
//  Created by Albin Stigö on 21/11/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef alsa_h
#define alsa_h

#include <stdio.h>
#include <alsa/asoundlib.h>

snd_pcm_t* sdr_pcm_handle(const char* pcm_name, snd_pcm_uframes_t frames, snd_pcm_stream_t stream);

#endif /* alsa_h */
