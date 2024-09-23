#pragma once

#define CHECK_EXPECT_OK(ex)                                                                                            \
    do {                                                                                                               \
        if (!ex.has_value()) {                                                                                         \
            return std::unexpected{ex.error()};                                                                        \
        }                                                                                                              \
    } while (0)

#define CHECK_EXPECT_OK_EXCEPT(ex, msg)                                                                                \
    do {                                                                                                               \
        if (!ex.has_value() && ex.error() != msg) {                                                                    \
            return std::unexpected{ex.error()};                                                                        \
        }                                                                                                              \
    } while (0)
