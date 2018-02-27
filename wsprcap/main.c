//
//  main.c
//  alsatest
//
//  Created by Albin Stigö on 14/12/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//


#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <complex.h>
#include <liquid/liquid.h>
#include <assert.h>

#include "circular_buffer.h"
#include "alsa.h"

// "Dial" frequencies.
// To get TX freq add 1500
const double dfreqs[] =
{
    1.836600, 3.592600, 5.287200, 7.038600,
    10.138700, 14.095600, 18.104600, 21.094600,
    24.924600, 28.124600, 50.293000, 144.488500,
};

const int N = 2048;
const int FRAME_SIZE = sizeof(float) * 2;

static time_t
next_even_minute()
{
    time_t t0 = time(NULL);
    time_t t1 = t0 - (t0 % 120) + 120;
    // printf("time is %ld will trigger %ld\n", t0, t1 + 120);
    return t1;
}

int main(int argc, const char * argv[]) {
    
    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "udpiq DEVICE\n");
        exit(EXIT_FAILURE);
    }
    
    snd_pcm_t *pcm = sdr_pcm_handle(argv[1], N, SND_PCM_STREAM_CAPTURE);
    
    const snd_pcm_channel_area_t *m_areas;
    snd_pcm_uframes_t m_offset, m_frames;
    
    snd_pcm_sframes_t err = 0;
    snd_pcm_sframes_t frames;
    
    // options
    float r  = 7.0 * 375.0 / 625000.0; // down sampling ratio
    float As = 60.0f; // resampling filter stop-band attenuation [dB]
    
    // create multi-stage arbitrary resampler object
    msresamp_crcf q = msresamp_crcf_create(r,As);
    // msresamp_crcf_print(q);
    
    __block circular_buffer_t cb;
    // 2^19 closest power of two above sizeof(float complex) * 45000
    cb_init(&cb, 524288);
    
    // Construct a timespec with a value of the next even minute.
    struct timespec tspec;
    tspec.tv_sec = next_even_minute();
    tspec.tv_nsec = 0;
    dispatch_time_t when = dispatch_walltime(&tspec, 120 * NSEC_PER_SEC);
    
    // Periodic task that writes the ringbuffer to file
    // No mutex needed since int should be atomic.
    // Single producer / consumer.
    dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    dispatch_source_set_timer(timer, when, 120 * NSEC_PER_SEC, 0);
    
    dispatch_source_set_event_handler(timer, ^{
        
        // time to string
        time_t t;
        struct tm *tm;
        time(&t);
        tm = gmtime (&t);
        
        char c2file[15];
        strftime(c2file, sizeof(c2file), "%y%m%d_%H%M.c2", tm);
        
        float complex *data = cb_readptr_history(&cb, 45000 * sizeof(float complex));
        printf("Writing to file: %s\n", c2file);
        
        //sprintf(c2file, "%d.c2", (int)t);
        
        FILE *file = fopen(c2file, "w");
        
        // write header
        int ntrmin = 2;
        double dfreq = 7.0400980;
        fwrite(c2file, sizeof(c2file) - 1, 1, file);
        fwrite(&ntrmin, sizeof(int), 1, file);
        fwrite(&dfreq, sizeof(double), 1, file);
        fwrite(data, sizeof(float complex), 45000, file);
        fclose(file);
    });
    dispatch_resume(timer);
    
    snd_pcm_start(pcm);
    
    // Park here and collect samples, downsample and put them on the ringbuffer.
    while(true) {
        // snd_pcm_wait will select/poll until audio is available
        if((err = snd_pcm_wait(pcm, -1)) < 0) {
            fprintf(stderr, "sndpcm wait %d\n", (int) err);
            snd_pcm_recover(pcm, (int)err, 0);
            snd_pcm_start(pcm);
        }
        
        frames = snd_pcm_avail_update(pcm);
        if(frames < 0) {
            fprintf(stderr, "snd_pcm_avail_update\n");
            break;
        }
        // m_frames = frames wanted on call and frames available on exit.
        m_frames = frames;
        err = snd_pcm_mmap_begin(pcm, &m_areas, &m_offset, &m_frames);
        if(err < 0) {
            perror("snd_pcm_mmap_begin");
            break;
        }
        
        // printf("wanted: %lu, got: %lu\n", (unsigned long)frames, (unsigned long) m_frames);
        // printf("%lu %lu\n", (unsigned long)m_areas[0].addr, m_offset);
        
        // downsample to 375 samp/s
        float complex *x = &m_areas[0].addr[m_offset * FRAME_SIZE];
        // Decimate and put on circular buffer
        float complex *y = cb_writeptr(&cb);
        uint32_t nwritten = 0;
        msresamp_crcf_execute(q, x, N, y, &nwritten);
        
        assert(nwritten > 0);
        
        cb_produce(&cb, nwritten * sizeof(float complex));
        // printf("%d\n", nwritten);
        
        // we are finished
        err = snd_pcm_mmap_commit(pcm, m_offset, m_frames);
        if(err < 0) {
            perror("snd_pcm_mmap_commit");
            break;
        }
    }
    
    snd_pcm_close(pcm);
    
    return 0;
}
