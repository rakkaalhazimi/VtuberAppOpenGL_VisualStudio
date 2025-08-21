#ifndef USED_BONES_HEADER_FILE
#define USED_BONES_HEADER_FILE

#include <string_view>
#include <unordered_set>


class UsedBones {
  public:
    static constexpr std::string_view CENTER = "センター";
    static constexpr std::string_view UPPER_BODY = "上半身";
    static constexpr std::string_view NECK = "首";
    static constexpr std::string_view HEAD = "頭";
    static constexpr std::string_view LEFT_ARM = "左腕";
    static constexpr std::string_view RIGHT_ARM = "右腕";
};
#endif
