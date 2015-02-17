#include "zlib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHUNK 1024
#define NGX_OK 0
#define NGX_ERROR 1

typedef int ngx_int_t;
typedef unsigned int u_int;
typedef unsigned char u_char;
typedef unsigned long u_long;
typedef struct ngx_chain_s ngx_chain_t;
typedef struct ngx_buf_s ngx_buf_t;

struct ngx_chain_s {
    ngx_buf_t *buf;
    ngx_chain_t *next;
};

struct ngx_buf_s {
    u_char *pos;
    u_char *last;
    u_char *start;
    u_char *end;
};


ngx_chain_t*
create_chain_with_buf(size_t buf_size) {
    ngx_chain_t *cl = NULL;
    ngx_buf_t *b = NULL;
    cl = malloc(sizeof(ngx_chain_t));
    if(cl == NULL) {
        goto Error;
    }
    b = malloc(sizeof(ngx_buf_t));
    if(b == NULL) {
        goto Error;
    }
    b->start = malloc(buf_size);
    if(b->start == NULL) {
        goto Error;
    }
    b->pos = b->start;
    b->last = b->start;
    b->end = b->last + buf_size;

    cl->buf = b;
    cl->next = NULL;
    return cl;

Error:
    if(cl != NULL) {
        free(cl);
    }
    if(b != NULL) {
        if(b->start != NULL) {
            free(b->start);
        }
        free(b);
    }
    return NULL;
}


ngx_int_t
gunzip(ngx_chain_t *in, ngx_chain_t **out) {
    int err = 0;
    ngx_chain_t *cl;
    ngx_buf_t *bin, *bout;
    z_stream stream = {0};

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;

    err = inflateInit2(&stream, MAX_WBITS + 16);
    if(err != Z_OK) {
        return NGX_ERROR;
    }
    *out = create_chain_with_buf(CHUNK);
    if(*out == NULL) {
        goto Error;
    }
    cl = *out;
    bin = in->buf;
    bout = (*out)->buf;

    stream.next_in = bin->pos;
    stream.avail_in = bin->last - bin->pos;

    stream.next_out = bout->pos;
    stream.avail_out = bout->end - bout->pos;

    while(1) {
        if(stream.avail_in == 0) {
            in = in->next;
            if(in == NULL) {
                bout->last = bout->end - stream.avail_out;
                goto OK;
            }
            bin = in->buf;
            stream.next_in = bin->pos;
            stream.avail_in = bin->last - bin->pos;
        }
        if(stream.avail_out == 0) {
            bout->last = bout->end;
            cl->next = create_chain_with_buf(CHUNK);
            if(cl->next == NULL) {
                goto Error;
            }
            cl = cl->next;
            bout = cl->buf;
            stream.next_out = bout->pos;
            stream.avail_out = bout->end - bout->pos;
        }
        err = inflate(&stream, Z_NO_FLUSH);
        switch(err) {
            case Z_STREAM_END:
                bout->last = bout->end - stream.avail_out;
                goto OK;
            case Z_OK:
                break;
            default:
                goto Error;
        }
    }

OK:
    err = inflateEnd(&stream);
    if(err != Z_OK) {
        return NGX_ERROR;
    }
    return NGX_OK;
Error:
    inflateEnd(&stream);
    return NGX_ERROR;
}



ngx_int_t
zip(ngx_chain_t *in, ngx_chain_t **out) {
    int err = 0, flush = Z_NO_FLUSH;
    ngx_chain_t *cl;
    ngx_buf_t *bin, *bout;
    z_stream stream = {0};

    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;

    err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
    if(err != Z_OK) {
        return NGX_ERROR;
    }
    *out = create_chain_with_buf(CHUNK);
    if(*out == NULL) {
        goto Error;
    }
    cl = *out;
    bin = in->buf;
    bout = cl->buf;


    stream.next_in = bin->pos;
    stream.avail_in = bin->last - bin->pos;

    stream.next_out = bout->pos;
    stream.avail_out = bout->end - bout->pos;


    while(1) {
        if(stream.avail_in == 0) {
            printf("avail_in == 0\n");
            if(in != NULL) {
                in = in->next;
                if(in != NULL) {
                    bin = in->buf;
                    stream.next_in = bin->pos;
                    stream.avail_in = bin->last - bin->pos;
                }
            }
        }
        if(stream.avail_out == 0) {
            printf("avail_out == 0\n");
            bout->last = bout->end;
            cl->next = create_chain_with_buf(CHUNK);
            if(cl->next == NULL) {
                goto Error;
            }
            cl = cl->next;
            bout = cl->buf;
            stream.next_out = bout->pos;
            stream.avail_out = bout->end - bout->pos;
        }
        flush = ( in == NULL ? Z_FINISH : Z_NO_FLUSH );

        err = deflate(&stream, flush);
        switch(err) {
            case Z_STREAM_END:
                bout->last = bout->end - stream.avail_out;
                goto OK;
            case Z_OK:
                break;
            default:
                goto Error;
        }
    }

OK:
    err = deflateEnd(&stream);
    if(err != Z_OK) {
        return NGX_ERROR;
    }
    return NGX_OK;
Error:
    deflateEnd(&stream);
    return NGX_ERROR;
}


ngx_chain_t*
read_file(const char *filename) {
    ngx_chain_t *out, *cl;
    ngx_buf_t *b;
    size_t read_len, b_left_len;
    FILE *f = fopen(filename, "rb");
    if(f == NULL) {
        return NULL;
    }
    out = create_chain_with_buf(CHUNK);
    if(out == NULL) {
        goto Error;
    }
    cl = out;
    b = cl->buf;
    b_left_len = CHUNK;

    while(read_len = fread(b->last, 1, b_left_len, f)) {
        b_left_len -= read_len;
        b->last += read_len;
        if(b_left_len == 0) {
            cl->next = create_chain_with_buf(CHUNK);
            if(cl->next == NULL) {
                goto Error;
            }
            cl = cl->next;
            b = cl->buf;
            b_left_len = CHUNK;
        }
    }

OK:
    fclose(f);
    return out;
Error:
    fclose(f);
    return NULL;
}

ngx_int_t
write_file(const char *filename, ngx_chain_t *in) {
    ngx_buf_t *b;
    size_t write_len, b_left_len;
    u_char *pos;
    FILE *f = fopen(filename, "wb");
    if(f == NULL) {
        return NGX_ERROR;
    }
    b = in->buf;
    b_left_len = b->last - b->pos;
    pos = b->pos;

    while(write_len = fwrite(pos, 1, b_left_len, f)) {
        b_left_len -= write_len;
        pos += write_len;
        if(b_left_len == 0) {
            in = in->next;
            if(in == NULL) {
                break;
            }
            b = in->buf;
            b_left_len = b->last - b->pos;
            pos = b->pos;
        }
    }

    fclose(f);
    return NGX_OK;
}


int main(void) {
    /*
    ngx_chain_t *gzip_data, *gunzip_data;
    ngx_int_t rc;
    gzip_data = read_file("gzip_data.gz");
    if(gzip_data == NULL) {
        return -1;
    }
    rc = gunzip(gzip_data, &gunzip_data);
    if(rc != NGX_OK) {
        return -1;
    }
    rc = write_file("html_data", gunzip_data);
    if(rc != NGX_OK) {
        return -1;
    }
    */

    ngx_chain_t *html_data, *zlib_data;
    ngx_int_t rc;
    html_data = read_file("html_data");
    if(html_data == NULL) {
        return -1;
    }

    rc = zip(html_data, &zlib_data);
    if(rc != NGX_OK) {
        return -1;
    }

    rc = write_file("zlib_data", zlib_data);
    if(rc != NGX_OK) {
        return -1;
    }

    return 0;
}
