#include <catch2/catch_test_macros.hpp>

#include <immer/extra/archive/box/archive.hpp>

#include "utils.hpp"

using namespace test;

TEST_CASE("Test saving a box")
{
    using Container = immer::box<std::string>;

    const auto test1 = Container{"hello"};
    const auto test2 = Container{"world"};

    auto ar = immer::archive::box::make_save_archive_for(test1);

    auto id1          = immer::archive::container_id{};
    std::tie(ar, id1) = immer::archive::box::save_to_archive(test1, ar);

    auto id2          = immer::archive::container_id{};
    std::tie(ar, id2) = immer::archive::box::save_to_archive(test2, ar);

    SECTION("Save again, same id")
    {
        auto id3          = immer::archive::container_id{};
        std::tie(ar, id3) = immer::archive::box::save_to_archive(test2, ar);
        REQUIRE(id3 == id2);
    }

    const auto ar_str = to_json(ar);
    // REQUIRE(ar_str == "");

    SECTION("Via JSON")
    {
        const auto archive =
            from_json<immer::archive::box::
                          archive_load<std::string, Container::memory_policy>>(
                ar_str);
        auto loader =
            immer::archive::box::make_loader_for(Container{}, archive);

        const auto loaded1 = loader.load(id1);
        REQUIRE(loaded1 == test1);

        const auto loaded2 = loader.load(id2);
        REQUIRE(loaded2 == test2);

        REQUIRE(loaded2.impl() == loader.load(id2).impl());
    }

    SECTION("Via archive transformation")
    {
        auto loader = immer::archive::box::make_loader_for(Container{},
                                                           to_load_archive(ar));

        const auto loaded1 = loader.load(id1);
        REQUIRE(loaded1 == test1);

        const auto loaded2 = loader.load(id2);
        REQUIRE(loaded2 == test2);
    }
}

TEST_CASE("Test saving and mutating a box")
{
    using Container = immer::box<std::string>;

    auto test1 = Container{"hello"};
    auto ar    = immer::archive::box::make_save_archive_for(test1);

    auto id1          = immer::archive::container_id{};
    std::tie(ar, id1) = immer::archive::box::save_to_archive(test1, ar);

    test1 = std::move(test1).update([](auto str) { return str + " world"; });

    auto id2          = immer::archive::container_id{};
    std::tie(ar, id2) = immer::archive::box::save_to_archive(test1, ar);

    REQUIRE(id1 != id2);

    auto loader =
        immer::archive::box::make_loader_for(Container{}, to_load_archive(ar));

    const auto loaded1 = loader.load(id1);
    REQUIRE(loaded1.get() == "hello");

    const auto loaded2 = loader.load(id2);
    REQUIRE(loaded2.get() == "hello world");
}

namespace {
struct fwd_type;
struct test_type
{
    immer::box<fwd_type> data;
};
struct fwd_type
{
    int data = 123;
};
} // namespace

TEST_CASE("Test box with a fwd declared type")
{
    auto val = test_type{};
    REQUIRE(val.data.get().data == 123);
}
