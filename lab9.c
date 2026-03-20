#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8000
#define BUF_SIZE 64
#define ADDR "127.0.0.1"

/* 
 * Questions to answer at top of client.c
 * (You should not need to change the code in client.c)
 * 
 * 1. What is the address of the server it is trying to connect to (IP address and port number)?
 *    From above, the IP address is defined as "127.0.0.1" and the port is defined as "8000."
 *
 * 2. Is it UDP or TCP? How do you know?
 *    It is a TCP server, because the code uses functions "read()" and "write()", which only TCP
 *    uses.
 *
 * 3. The client going to send some data to the server. Where does it get this data from?
 *    How can you tell in the code?
 *    From the keyboard of the client, because "read(STDIN_FILENO, buf, BUF_SIZE)" reads from the
 *    keyboard and sends the bytes to the server.
 *
 * 4. How does the client program end? How can you tell that in the code?
 *    Whenever fewer than 1 bytes are read (since the while loop for client.c has the termination
 *    condition num_bytes < 1), so basically when you enter nothing but still send (<enter>) or if
 *    you send an EOF signal. The while loop stops if 0 OR 1 bytes are read, so <enter> sends a \n
 *    and terminates the loop. 
 */





#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

int main() {
  struct sockaddr_in addr;
  int sfd;
  ssize_t num_read;
  char buf[BUF_SIZE];

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  if (inet_pton(AF_INET, ADDR, &addr.sin_addr) <= 0) {
    handle_error("inet_pton");
  }

  int res = connect(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (res == -1) {
    handle_error("connect");
  }

  while ((num_read = read(STDIN_FILENO, buf, BUF_SIZE)) > 1) {
    if (write(sfd, buf, num_read) != num_read) {
      handle_error("write");
    }
    printf("Just sent %zd bytes.\n", num_read);
  }

  if (num_read == -1) {
    handle_error("read");
  }

  close(sfd);
  exit(EXIT_SUCCESS);
}
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUF_SIZE 64
#define PORT 8000
#define LISTEN_BACKLOG 32

#define handle_error(msg)                                                      \
  do {                                                                         \
    perror(msg);                                                               \
    exit(EXIT_FAILURE);                                                        \
  } while (0)

// Shared counters for: total # messages, and counter of clients (used for
// assigning client IDs)
int total_message_count = 0;
int client_id_counter = 1;

// Mutexs to protect above global state.
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_id_mutex = PTHREAD_MUTEX_INITIALIZER;

struct client_info {
  int cfd;
  int client_id;
};

void *handle_client(void *arg) { // PTHREAD requires functions with void *arg
                                 // arguments, so the first line of this
                                 // function resets the Type information
  struct client_info *client = (struct client_info *)arg;
  // EXTRACT ARGS
  int cfd = client->cfd;
  int client_ID = client->client_id;
  free(client); // must be heap-allocated in main so free it here.
  // debug: printf("cfd = %d, client_id=%d\n", cfd, client_id);
  //  TODO: print the message received from client
  //  create a storage buffer
  char buf[BUF_SIZE];
  ssize_t bytes_read;

  while ((bytes_read = read(cfd, buf, BUF_SIZE)) > 0) {
    buf[bytes_read] = '\0';
    // HAVE TO USE PTHREAD
    pthread_mutex_lock(&count_mutex);
    total_message_count++;
    int temp_count = total_message_count; // temp storage
    pthread_mutex_unlock(&count_mutex);

    printf("Msg #%3d; Client ID %d: %s", temp_count, client_ID, buf);
  }
  // MUST CLOSE CONNECTION
  close(cfd);
  // TODO: increase total_message_count per message
  // MUST USE PTHREAD
  printf("Ending thread for client %d\n", client_ID);
  return NULL;
}

int main() {
  struct sockaddr_in addr;
  int sfd;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1) {
    handle_error("socket");
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1) {
    handle_error("bind");
  }

  if (listen(sfd, LISTEN_BACKLOG) == -1) {
    handle_error("listen");
  }

  for (;;) {
    // TODO: create a new thread when a new connection is encountered
    // PSA: cfd = client file descriptor, sfd = server file descriptor
    int cfd = accept(sfd, NULL, NULL);
    if (cfd == -1) {
      handle_error("accept: client");
    }
    pthread_mutex_lock(&client_id_mutex);
    int connection_id = client_id_counter++;
    pthread_mutex_unlock(&client_id_mutex);

    // struct
    struct client_info *inf = malloc(sizeof(struct client_info));
    inf->cfd = cfd;
    inf->client_id = connection_id;

    printf("New client created! ID %d on socket %d\n", connection_id, cfd);

    // TODO: call handle_client() when launching a new thread, and provide
    pthread_t th;
    pthread_create(&th, NULL, handle_client, inf);
    pthread_detach(th); // because we don't want the server to wait.
  }

  if (close(sfd) == -1) {
    handle_error("close");
  }

  return 0;
}
