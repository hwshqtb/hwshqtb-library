#if __cplusplus >= 202002L
#include <span>
namespace hwshqtb {
    namespace container {
        static constexpr inline std::size_t dynamic_extent = std::dynamic_extent;
        template <typename T, std::size_t Extent>
        using span = std::span<T, Extent>;
    }
}
#else 
#include "stl/span.hpp"
namespace hwshqtb {
    namespace container {
        template <typename T, std::size_t Extent>
        using span = stl::span<T, Extent>;
    }
}
#endif