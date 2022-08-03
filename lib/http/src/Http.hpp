#pragma once

namespace Http {

enum class Field {
    Expect,
    UserAgent,
};

const char*
toString(Field field);

} // namespace Http