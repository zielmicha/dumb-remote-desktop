#include "x11_support.h"
#include "protocol.h"
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

void run_child(int fd);

int main(int argc, char** argv) {
  if(argc != 2) {
    fprintf(stderr, "expected one argument\n");
    return 2;
  }
  int port = atoi(argv[1]);
  int main_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(main_fd == -1) {
    perror("socket");
    return 1;
  }
  int yes = 1;
  if (setsockopt(main_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
  }
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if(bind(main_fd, (struct sockaddr *)&address, sizeof(address)) == -1) {
    perror("bind");
    return 1;
  }
  if(listen(main_fd, 1) == -1) {
    perror("listen");
    return 1;
  }
  while(1) {
    int client_fd = accept(main_fd, NULL, NULL);
    if(client_fd == -1) {
      perror("accept");
      return 1;
    }
    int pid = fork();
    if(pid == -1) {
      perror("fork");
    }
    if(pid == 0) {
      run_child(client_fd);
      return 0;
    } else {
      close(client_fd);
      fprintf(stderr, "forked child with PID %d\n", pid);
    }
  }
}

int my_send(int fd, void* data, int size, int flags) {
  int all_sent = 0;
  while(all_sent != size) {
    int sent = send(fd, ((char*)data) + all_sent, size - all_sent, flags);
    if(sent == -1) return -1;
    all_sent += sent;
  }
  return 0;
}

void run_child(int fd) {
  while(true) {
     struct Image im = get_root_window_data();
     struct Protocol_header header;
     header.size = im.size + sizeof(struct Image_header);
     header.type = PACKET_IMAGE_DATA;
     header.flags = 0;
     struct Image_header imheader;
     imheader.width = im.width;
     imheader.height = im.height;
     memset(imheader.tag, 0xAA, sizeof(imheader.tag));
     if(my_send(fd, &header, sizeof(header), 0) == -1) goto error;
     if(my_send(fd, &imheader, sizeof(imheader), 0) == -1) goto error;
     if(my_send(fd, im.data, im.size, 0) == -1) goto error;
     free(im.data);
  }
  goto end;
 error:
  perror("run_child");
 end:
  close(fd);
}
