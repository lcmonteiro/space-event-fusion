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


// #include "fusion.hpp"
// #include "resources/timer.hpp"

#include <functional>
#include <variant>

#include <sys/eventfd.h>
#include <unistd.h>

#include "fusion.hpp"
#include "resources/timer.hpp"

// namespace fusion {
// typedef std::function<void()> wait;
// typedef std::function<void()> next;
// template <class... Ts>
// struct overloaded : Ts... {
//     using Ts::operator()...;
// };
// template <class... Ts>
// overloaded(Ts...) -> overloaded<Ts...>;

// typedef std::function<void(const std::variant<wait>&)> engine;

// auto build(std::shared_ptr<engine> en) {
//     struct event final {
//         event()
//           : h([]() { return eventfd(0, 0); }()) {}
//         ~event() { close(h); }
//         const int h;
//     };
//     return std::make_shared<engine>(
//         [en, ev = std::make_shared<event>()](const std::variant<wait>& opt) {
//             std::visit(overloaded{[](wait w) { std::cout << "wait" << std::endl; }}, opt);
//         });
// }
// } // namespace fusion



TEST(FusionSpace, positive_test) {

    // auto s = std::make_shared<fusion::Space>();
    // auto p = std::make_shared<fusion::Scope<std::string, std::shared_ptr<fusion::Space>>>(s);
    // fusion::wait(p, [](auto, auto){});
    //
    // std::make_shared<FusionSpace>()->main([](auto space){
    //     std::make_shared<TimeEvent>(space)->main([](auto timer) {
    //         timer.set();
    //         time.await([])
    //     });
    // };

    // using Variant = std::variant<std::shared_ptr<Base>, std::shared_ptr<Multi>>;

    // //std::shared_ptr<Multi> b1 = std::make_shared<B>();
    // Variant b = std::make_shared<B>();
    // auto v = [](auto opt) {
    //     std::visit(
    //         fusion::overloaded{
    //             [](std::shared_ptr<Multi> m) { std::cout << "multi" << std::endl; },
    //             [](std::shared_ptr<Base> m) { std::cout << "base" << std::endl; }
    //         },
    //         opt);
    // };

    // v(b);
    // auto r = [s](auto opt) {
    //     int i = 0;
    //     std::visit(
    //         fusion::overloaded{[](fusion::wait w) {
    //             s([i](auto))

    //             std::cout << "wait" << std::endl; }}, opt);
    // };

    // auto space = std::make_shared<fusion::engine>([](const std::variant<fusion::wait>& opt) {
    //     std::visit(
    //         fusion::overloaded{[](fusion::wait w) { std::cout << "wait" << std::endl; }}, opt);
    // });


    // auto timer = fusion::build(space);

    // timer(fusion::wait([](auto self, auto home) {

    // }));

    // Fusion::Space<Fusion::Kernel>([](auto self) {
    //     Fusion::Scope<Fusion::Resource::Timer>(self, [](auto self) {
    //         auto entry_point = [](auto self, auto entry_point) -> void {
    //             self->wait([entry_point](auto self) {
    //                 self->clear(self);
    //                 entry_point(self, entry_point);
    //             });
    //         };
    //         self->build(std::chrono::system_clock::now(), std::chrono::seconds{1});
    //         entry_point(self, entry_point);
    //     });
    // });

    // fusion::build<fusion::Space>([](auto self) {
    //     fusion::build<fusion::Timer>(
    //         self,
    //         [](auto self, auto space) {
    //             fusion::build<fusion::Timer>(
    //                 self,
    //                 [](auto self, auto space) {
    //                     auto entry_point = [=](auto entry_point) -> void {
    //                         wait(self, [entry_point](auto self, auto space) {
    //                             fusion::clear(self);
    //                             entry_point(entry_point);
    //                         });
    //                     };
    //                     entry_point(entry_point);
    //                 },
    //                 std::chrono::system_clock::now(),
    //                 std::chrono::seconds{1});
    //         },
    //         std::chrono::system_clock::now(),
    //         std::chrono::seconds{1});
    // });

    build<fusion::Space>([](auto self) {
        std::cout << __func__ << ":" << __LINE__ <<std::endl;
        build<fusion::Timer>(
            self,
            [](auto self, auto space) {
                // auto entry_point = [self](auto entry_point) -> void {
                //     fusion::wait(self, [entry_point](auto self, auto space) {
                //         fusion::clear(self);
                //         entry_point(entry_point);
                //     });
                // };
                // entry_point(entry_point);
                std::cout << __func__ << ":" << __LINE__ <<std::endl;
                wait(self, [](auto self, auto space) {
                    clear(self);
                    // entry_point(entry_point);
                });
                std::cout << __func__ << ":" << __LINE__ <<std::endl;
                clear(self);
            },
            std::chrono::system_clock::now(),
            std::chrono::seconds{1});
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