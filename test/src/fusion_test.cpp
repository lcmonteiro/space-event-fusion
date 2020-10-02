#include <chrono>

#include <gtest/gtest.h>


// class TimeEvent;

// struct FusionSpace {
//     void add(std::shared_ptr<TimeEvent> t);
// };

// template <typename Context>
// struct WorkSpace : public Context {
//     /* data */
// };

// struct TimeEvent {
//     TimeEvent(std::function<void(FusionSpace)> f);
// };

// template <typename Scope>
// void EntryPoint(Scope self) {
//     self->wait([self = self]() { EntryPoint(self); });
// }

// struct Fusion : std::enable_shared_from_this<Resource> {
//     auto self() { return shared_from_this(); }
// };
// struct Resource : std::enable_shared_from_this<Resource> {
//     auto self() { return shared_from_this(); }
// };

// struct Timer : Resource {};

// template <typename Type, typename Main>
// void Space(Main func) {
//     auto spc = std::make_shared<Type>();
//     func(spc);
//     spc();
// }
// template <typename Type, typename Main>
// void Scope(Main func) {
//     auto spc = std::make_shared<Type>();
//     func(spc);
// }


#include "fusion.hpp"
#include "resources/timer.hpp"

TEST(FusionSpace, positive_test) {
    //
    // std::make_shared<FusionSpace>()->main([](auto space){
    //     std::make_shared<TimeEvent>(space)->main([](auto timer) {
    //         timer.set();
    //         time.await([])
    //     });
    // };

    Fusion::Space<Fusion::Kernel>([](auto self) {
        Fusion::Scope<Fusion::Resource::Timer>(self, [](auto self, auto home) {
            auto entry_point = [](auto self, auto entry_point) -> void {
                //entry_point(self, entry_point); 
                self->wait([entry_point](auto self, auto home) { 
                    self->clear();
                    entry_point(self, entry_point); 
                });
            };
            self->build(std::chrono::system_clock::now(), std::chrono::seconds{1});
            entry_point(self, entry_point);
        });
    });

    // {
    //     Share::In::Message::Local(space, "/sdhds");
    // }

    // __________
    // __________\ ___
    // __________/

    // Check
    EXPECT_EQ(1, 1);
}