
#define PACKET_IMAGE_DATA 1

struct Image {
  char* data;
  int size;
  int width;
  int height;
};

struct Protocol_header {
  int type;
  int size;
  int flags;
};

struct Image_header {
  int width;
  int height;
  char tag[32];
};
