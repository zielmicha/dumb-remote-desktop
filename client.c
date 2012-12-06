#include "protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define SET_PIXEL(mem, i, r, g, b) ((struct Pixel*)mem)[i]=(struct Pixel){r, g, b}

struct Pixel {
  char r, g, b;
};

void* fbmem;
int sock_fd;
int width = 640, height = 480;

void tick();

int main(int argc, char** argv) {
  int port;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  if (argc < 4) {
    fprintf(stderr, "usage: %s hostname port fbdev\n", argv[0]);
    return 1;
  }
  port = atoi(argv[2]);
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket");
    return 1;
  }
  printf("resolving %s... ", argv[1]);
  fflush(stdout);
  server = gethostbyname(argv[1]);
  printf("ok\n");
  if (server == NULL) {
    perror("gethostbyname");
    return 1;
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr,
        (char *) &serv_addr.sin_addr.s_addr,
        server->h_length);
  serv_addr.sin_port = htons(port);
  printf("connecting... ");
  fflush(stdout);
  if (connect(sock_fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    perror("connect");
    return 1;
  }
  printf("ok\n");

  int fbsize = width * height * 3;
  int fbfd = open(argv[3], O_RDWR);
  if(fbfd == -1) {
    perror("open");
    return 1;
  }
  fbmem = mmap(NULL, fbsize, PROT_WRITE, MAP_SHARED, fbfd, 0);
  if((long)fbmem == -1 || fbmem == NULL) {
    perror("mmap");
    return 1;
  }

  while(1) {
    tick();
  }

  munmap(fbmem, fbsize);
  close(fbfd);
}

int recvall(int fd, void* data, int size, int flags) {
  while(size != 0) {
    int recvd = recv(fd, data, size, flags);
    if(recvd == -1) return -1;
    *((char**)&data) += recvd;
    size -= recvd;
  }
}

struct Image recv_image() {
  struct Protocol_header header;
  struct Image_header imheader;
  if(recvall(sock_fd, &header, sizeof(header), 0) == -1) {
    perror("recv header");
    exit(1);
  }
  if(header.type != PACKET_IMAGE_DATA) {
    fprintf(stderr, "unexpected packet of type %d\n", header.type);
    exit(3);
  }
  if(recvall(sock_fd, &imheader, sizeof(imheader), 0) == -1) {
    perror("recv imheader");
    exit(1);
  }
  struct Image im;
  im.width = imheader.width;
  im.height = imheader.height;
  if(im.width < 0 || im.height < 0 || im.width > 4096 || im.height > 4096) {
    fprintf(stderr, "protocol failure: im.width < 0 || im.height < 0 || im.width > 4096 || im.height > 4096\n");
    exit(3);
  }
  im.size = imheader.width * imheader.height * 3;
  im.data = malloc(im.size);
  if(recvall(sock_fd, im.data, im.size, 0) == -1) {
    perror("recv data");
    exit(1);
  }
  return im;
}


void tick() {
  struct Image im = recv_image();
  // BUG: what if im.width < width?
  for(int x=0; x<width; x++) {
    for(int y=0; y<height; y++) {
      ((struct Pixel*)fbmem)[y * width + x] = ((struct Pixel*)im.data)[y * im.width + x];
    }
  }
  free(im.data);
}
