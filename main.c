#include <stdlib.h>

#include <SDL2/SDL.h>
#include <aom/aom_decoder.h>
#include <aom/aomdx.h>

struct VideoInfoStruct {
  uint32_t codec_fourcc;
  int frame_width;
  int frame_height;
  int time_base_numerator;
  int time_base_denominator;
};
typedef struct VideoInfoStruct VideoInfo;

struct VideoReaderStruct {
  FILE *file;
  uint8_t *buffer;
  size_t buffer_size;
  size_t frame_size;
};
typedef struct VideoReaderStruct VideoReader;

struct CodecStruct {
  const char *const name;
  aom_codec_iface_t *(*const codec_interface)();
};
typedef struct CodecStruct Codec;

static const char *exec_name;
static const Codec decoders[] = {
  { "av1", &aom_codec_av1_dx }
};

VideoReader *video_reader_open(const char *filename) {
  char header[32];
  VideoReader *reader = NULL;
  FILE *const file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open file\n");
    return NULL;
  }

  if (fread(header, 1, 32, file) != 32) {
    fprintf(stderr, "Failed to read header\n");
    return NULL;
  }

  if (memcmp("DKIF", header, 4) != 0) {
    fprintf(stderr, "Wrong IVF signature\n");
    return NULL;
  }

  reader = (VideoReader *)calloc(1, sizeof(*reader));
  if (!reader) {
    return NULL;
  }

  reader->file = file;

  return reader;
}

void video_reader_close(VideoReader *reader) {
  if (reader) {
    fclose(reader->file);
    free(reader->buffer);
    free(reader);
  }
}

int video_reader_read_frame(VideoReader *reader) {
  // TODO: Read IVF frame and store in reader->buffer
  return 0;
}

const uint8_t *video_reader_get_frame(VideoReader *reader, size_t *size) {
  if (size) *size = reader->frame_size;

  return reader->buffer;
}

void usage_exit(void) {
  fprintf(stderr, "Usage: %s <file>\n", exec_name);
  exit(1);
}

int main(int argc, char **argv) {
  int flags;
  VideoReader *reader = NULL;
  aom_codec_ctx_t codec;
  const Codec *decoder = &decoders[0];
  int frame_cnt = 0;

  printf("AV1 player by Eyevinn Technology\n");
  exec_name = argv[0];

  if (argc != 2) {
    usage_exit();
  }

  if (aom_codec_dec_init(&codec, decoder->codec_interface(), NULL, 0)) {
    fprintf(stderr, "Failed to initiate decoder\n");
    exit(1);
  }

  reader = video_reader_open(argv[1]);
  if (!reader) {
    fprintf(stderr, "Failed to open %s for reading\n", argv[1]);
    exit(1);
  }

  flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

  if (SDL_Init (flags)) {
    fprintf(stderr, "Failed to initiate SDL %s\n", SDL_GetError());
    exit(1);
  }

  while (video_reader_read_frame(reader)) {
    aom_codec_iter_t iter = NULL;
    aom_image_t *img = NULL;
    size_t frame_size = 0;
    const unsigned char *frame =
      video_reader_get_frame(reader, &frame_size);
    if (aom_codec_decode(&codec, frame, (unsigned int)frame_size, NULL, 0)) {
      fprintf(stderr, "Failed to decode frame\n");
      exit(1);
    }
    while ((img = aom_codec_get_frame(&codec, &iter)) != NULL) {
      // TODO: Write output to display
      // Push image on frame queue
      // Video refresh loop (consume frame queue and display image in sync with master clock)
      ++frame_cnt;
    }
  }

  printf("Processed %d frames.\n", frame_cnt);
  if (aom_codec_destroy(&codec)) {
    fprintf(stderr, "Failed to destroy codec\n");
    exit(1);
  }
  
  video_reader_close(reader);
}