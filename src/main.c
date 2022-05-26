#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "server.h"
#include <MagickWand/MagickWand.h>

uv_loop_t *loop;
struct sockaddr_in addr;

void resize_img(char *filename, size_t w, size_t h);
void alloc_buffer(uv_handle_t __attribute__((unused)) *handle, size_t suggested_size, uv_buf_t *buf);
void response(uv_write_t *req, int status);
void request(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
void on_new_connection(uv_stream_t *server, int status);

int main(int __attribute__((unused)) argc, char __attribute__((unused)) *argv[]) {
  int r;
  uv_tcp_t server;

  /* TODO getopt (Port) */

  MagickWandGenesis();

  loop = uv_default_loop();
  uv_tcp_init(loop, &server);

  uv_ip4_addr("localhost", 7000, &addr);

  uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
  r = uv_listen((uv_stream_t*)&server, 128, on_new_connection);
  if (r) {
    fprintf(stderr, "Listen error %s\n", uv_strerror(r));
    exit(EXIT_FAILURE);
  }

  uv_run(loop, UV_RUN_DEFAULT);
  MagickWandTerminus();
  exit(uv_loop_close(loop));
}


void resize_img(char *filename, size_t w, size_t h){
  MagickWand *m_wand = NULL;

  m_wand = NewMagickWand();
  MagickReadImage(m_wand, filename);
  MagickAdaptiveResizeImage(m_wand, w, h);
  /* MagickSetImageCompressionQuality(m_wand,95); */
  MagickWriteImage(m_wand, filename);
  if(m_wand)
    DestroyMagickWand(m_wand);
}

void alloc_buffer(uv_handle_t __attribute__((unused)) *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char*)malloc(suggested_size);
  buf->len = suggested_size;
}

void response(uv_write_t *req, int status) {
  if (status) {
    fprintf(stderr, "Write error %s\n", uv_strerror(status));
  }
  free(req);
}

void request(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      fprintf(stderr, "Read error %s\n", uv_err_name(nread));
      uv_close((uv_handle_t*) client, NULL);
    }
  } else if (nread > 0) {
    /* TODO parse HTTP request */
    /* TODO create file from img (maybe blob) */
    /* TODO resize img */
    uv_write_t *req = (uv_write_t *) malloc(sizeof(uv_write_t));
    uv_buf_t wrbuf = uv_buf_init(buf->base, nread);
    /* TODO send img with HTTP headers */
    uv_write(req, client, &wrbuf, 1, response);
  }

  if (buf->base)
    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
  uv_tcp_t *client;
  if (status < 0) {
    fprintf(stderr, "New connection error %s\n", uv_strerror(status));
    return;
  }

  client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
  uv_tcp_init(loop, client);
  if (uv_accept(server, (uv_stream_t*) client) == 0) {
    uv_read_start((uv_stream_t*)client, alloc_buffer, request);
  } else {
    uv_close((uv_handle_t*) client, NULL);
  }
}
