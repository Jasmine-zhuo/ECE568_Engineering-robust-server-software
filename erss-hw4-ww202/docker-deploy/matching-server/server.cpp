#include <cstdlib>
#include "utils.hpp"

int main(int argc, char * argv[]) {
    StockDatabase initializer_db(true);
    int socket_fd = setup_socket();
    run(socket_fd);
    close(socket_fd);
    return 0;
}

