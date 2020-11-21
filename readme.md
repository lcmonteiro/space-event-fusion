# Space Event Fusion

![fusion](doc/event-fusion.svg)



# Use Case
TCP client and server resources interacting wit each other

``` C++
build<fusion::Space>([](auto self) {
        // server
        build<fusion::stream::remote::Server>(
          self,
          [](auto self, auto space) {          
              wait<fusion::input::Connection>(self, [](auto self, auto space) {              
                  function(self, [](auto self, auto process) {
                      wait<fusion::Input>(self, [process](auto self, auto space) {
                          std::string data(100, '\0');
                          read(self, data);
                          build<fusion::Timer>(
                            self,
                            [data](auto self, auto space) {
                                wait<fusion::Input>(self, [data](auto self, auto space) {
                                    write(space, data + "s");
                                });
                            },
                            std::chrono::system_clock::now() + std::chrono::seconds{1});
                          function(self, process);
                      });
                  });
              });
          },
          fusion::stream::remote::Address{"localhost", 10000});

        // client
        build<fusion::stream::remote::Client>(
          self,
          [](auto self, auto space) {              
              wait<fusion::output::Connection>(
                self,
                [](auto self, auto space) {                    
                    function(self, [](auto self, auto process) {
                        wait<fusion::Input>(self, [process](auto self, auto space) {
                            std::string data(100, '\0');
                            read(self, data);
                            build<fusion::Timer>(
                              self,
                              [data](auto self, auto space) {
                                  wait<fusion::Input>(self, [data](auto self, auto space) {
                                      write(space, data + "c");
                                  });
                              },
                              std::chrono::system_clock::now() + std::chrono::seconds{1});
                            function(self, process);
                        });
                    });
                    write(self, std::string("c"));
                },
                fusion::stream::remote::Address{"127.0.0.1", 10000});
          },
          fusion::stream::remote::IPv4());
    });
```