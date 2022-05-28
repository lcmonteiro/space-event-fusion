# Space Event Fusion
**Do we really need SW managers? :thinking:**

Use Event Fusion, select your resources, give them a workspace and wait :sweat_smile:

![fusion](doc/event-fusion.svg)

# Use Case
TCP client and server resources interacting with each other

``` C++
build<fusion::Space>([](auto self) {
    // server
    build<fusion::stream::remote::Server>(
      self,
      [](auto self, auto space) {
          std::ignore = space;
          wait<fusion::input::Connection>(self, [](auto self, auto space) {
              std::ignore = space;
              wait_loop<fusion::Input>(self, [](auto self, auto space) {
                  std::ignore = space;
                  std::string data(100, '\0');
                  read(self, data);
                  std::cout << "rx= " << data << std::endl;
                  build<fusion::Timer>(
                    self,
                    [data](auto self, auto space) {
                        std::ignore = space;
                        wait<fusion::Input>(self, [data](auto self, auto space) {
                            std::ignore = self;
                            write(space, data + "s");
                        });
                    },
                    std::chrono::system_clock::now() + std::chrono::milliseconds{10});
                  return (data.size() < data.capacity());
              });
          });
      },
      fusion::stream::remote::Address{"localhost", 10000});

    // client
    build<fusion::stream::remote::Client>(self, [](auto self, auto space) {
        std::ignore = space;
        wait<fusion::output::Connection>(
          self,
          [](auto self, auto space) {
              std::ignore = space;
              wait_loop<fusion::Input>(self, [](auto self, auto space) {
                  std::ignore = space;
                  std::string data(100, '\0');
                  read(self, data);
                  std::cout << "rx= " << data << std::endl;
                  build<fusion::Timer>(
                    self,
                    [data](auto self, auto space) {
                        std::ignore = space;
                        wait<fusion::Input>(self, [data](auto self, auto space) {
                            std::ignore = self;
                            write(space, data + "c");
                        });
                    },
                    std::chrono::system_clock::now() + std::chrono::milliseconds{20});
                  return (data.size() < data.capacity());
              });
              write(self, std::string("c"));
          },
          fusion::stream::remote::Address{"127.0.0.1", 10000});
    });
});
```
