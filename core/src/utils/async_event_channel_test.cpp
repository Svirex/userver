#include <utest/utest.hpp>

#include <engine/sleep.hpp>
#include <utils/async_event_channel.hpp>

TEST(AsyncEventChannel, Ctr) {
  utils::AsyncEventChannel<int> channel("channel");
}

class Subscriber final {
 public:
  Subscriber(int& x) : x_(x) {}

  void OnEvent(int x) { x_ = x; }

 private:
  int& x_;
};

TEST(AsyncEventChannel, Publish) {
  RunInCoro([]() {
    utils::AsyncEventChannel<int> channel("channel");

    int value{0};
    Subscriber s(value);
    EXPECT_EQ(value, 0);

    auto sub = channel.AddListener(&s, "", &Subscriber::OnEvent);
    EXPECT_EQ(value, 0);

    channel.SendEvent(1);
    engine::Yield();
    EXPECT_EQ(value, 1);

    sub.Unsubscribe();
  });
}

TEST(AsyncEventChannel, Unsubscribe) {
  RunInCoro([]() {
    utils::AsyncEventChannel<int> channel("channel");

    int value{0};
    Subscriber s(value);
    auto sub = channel.AddListener(&s, "", &Subscriber::OnEvent);

    channel.SendEvent(1);
    engine::Yield();
    EXPECT_EQ(value, 1);

    sub.Unsubscribe();
    channel.SendEvent(2);
    engine::Yield();
    engine::Yield();
    EXPECT_EQ(value, 1);
  });
}

TEST(AsyncEventChannel, PublishTwoSubscribers) {
  RunInCoro([]() {
    utils::AsyncEventChannel<int> channel("channel");

    int value1{0}, value2{0};
    Subscriber s1(value1), s2(value2);

    auto sub1 = channel.AddListener(&s1, "", &Subscriber::OnEvent);
    auto sub2 = channel.AddListener(&s2, "", &Subscriber::OnEvent);
    EXPECT_EQ(value1, 0);
    EXPECT_EQ(value2, 0);

    channel.SendEvent(1);
    engine::Yield();
    EXPECT_EQ(value1, 1);
    EXPECT_EQ(value2, 1);

    sub1.Unsubscribe();
    sub2.Unsubscribe();
  });
}

TEST(AsyncEventChannel, PublishException) {
  RunInCoro([]() {
    utils::AsyncEventChannel<int> channel("channel");

    struct X {
      void OnEvent(int) { throw std::runtime_error("error msg"); }
    };
    X x;

    auto sub1 = channel.AddListener(&x, "subscriber", &X::OnEvent);
    EXPECT_NO_THROW(channel.SendEvent(1));
    sub1.Unsubscribe();
  });
}
