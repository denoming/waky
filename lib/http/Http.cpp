#include "Http.hpp"

#include <array>

static std::array<const char*, 3> Headers = {
    "Expect",
    "User-Agent",
};

namespace Http {

const char*
toString(Field field)
{
    assert(static_cast<int>(field) < Headers.size());
    return Headers[static_cast<int>(field)];
}

} // namespace Http