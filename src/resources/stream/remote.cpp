
#include "resources/stream/remote.hpp"

#include <string.h>
#include <unistd.h>

namespace fusion {
namespace stream {
    namespace remote {

        Client::Client(const std::string& local) : Element(0) {}

        Process wait(Connection, Client&, Process proc, const std::string& remote) {
            std::cout << __FILE__ << __LINE__ << remote << std::endl;
            return proc;
        }
    } // namespace remote
} // namespace stream
} // namespace fusion