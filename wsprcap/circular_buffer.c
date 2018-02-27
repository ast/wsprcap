//
//  circularbuffer.c
//  libdtmf
//
//  Created by Albin Stigö on 14/10/15.
//  Copyright © 2015 Albin Stigo. All rights reserved.
//

#include "circular_buffer.h"

#include <unistd.h>
#include <string.h>
#include <assert.h>

// Create
int cb_init(circular_buffer_t *cb, uint32_t size) {
    // Virtual memory magic starts here.
    char  path[] = "/tmp/cb-XXXXXX";
    int   fd;
    int   ret;
    void *buf;
    void *adr;
    
    // Create temp file
    if((fd = mkstemp(path)) < 0) {
        perror("mkstemp");
        return fd;
    }
    // Remove link from filesystem
    if((ret = unlink(path)) < 0) {
        perror("unlink");
        return ret;
    }
    // Set size
    if((ret = ftruncate(fd, size)) < 0) {
        perror("ftruncate");
        return ret;
    }
    
    // Try to map an area of memory of 2 * size.
    buf = mmap(NULL, size << 1, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if(buf == MAP_FAILED) {
        perror("mmap");
        return -1;
    }
    
    // Then map size bytes twice.
    adr = mmap(buf, size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (adr != buf) {
        fprintf(stderr, "adr != buf\n");
        return -1;
    }
    
    adr = mmap(buf + size , size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);
    if (adr != buf + size) {
        fprintf(stderr, "adr != buf + size\n");
        return -1;
    }
    
    ret = close(fd);
    if(ret < 0) {
        perror("close");
        return ret;
    }
    
    // Zero buffer
    memset(buf, 0, size);
    
    cb->write = 0;
    cb->size = size;
    cb->buffer = buf;
    
    return 1;
}

void cb_destroy(circular_buffer_t *cb) {
    munlock((const void *) cb->buffer, cb->size << 1);
    // clear struct
    memset(cb, 0, sizeof(circular_buffer_t));
}
