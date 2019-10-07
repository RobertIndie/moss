#include <common/common.h>

Data *handle(Data *const request) {
  Data *response = new Data(1);
  return response;
}

int main(int argc, char **argv) {
 // InitLogger(argv);
  UDPServerChannel channel;
  channel.Bind("127.0.0.1", 2222);
  channel.Serve(handle);
  return 0;
}
