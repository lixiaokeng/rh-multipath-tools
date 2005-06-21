#define SOCKET_NAME "/var/cache/multipath/commsock"

/* some prototypes */
int ux_socket_connect(const char *name);
int ux_socket_listen(const char *name);
int send_packet(int fd, const char *buf, size_t len);
int recv_packet(int fd, char **buf, size_t *len);

