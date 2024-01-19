#pragma once

#define COLOR_Gum1    255, 162, 143
#define COLOR_L11     150, 117, 148
#define COLOR_L21     170, 255, 255
#define COLOR_L31     255, 0, 127
#define COLOR_L41     170, 255, 127
#define COLOR_L51     0, 0, 127
#define COLOR_L61     255, 255, 127
#define COLOR_L71     255, 170, 255
#define COLOR_L81     125, 255, 111
#define COLOR_R11     255, 0, 0
#define COLOR_R21     255, 125, 0
#define COLOR_R31     255, 255, 0
#define COLOR_R41     0, 255, 255
#define COLOR_R51     0, 255, 0
#define COLOR_R61     0, 0, 255
#define COLOR_R71     255, 0, 255
#define COLOR_R81     245, 255, 250
#define COLOR_311     245, 255, 250
#define COLOR_321     255, 80, 80
#define COLOR_331     0, 51, 0
#define COLOR_341     102, 102, 255
#define COLOR_351     204, 204, 0
#define COLOR_361     102, 0, 51
#define COLOR_371     231, 232, 228
#define COLOR_381     95, 95, 95
#define COLOR_411     102, 0, 204
#define COLOR_421     255, 153, 0

#define COLOR_Gum    255, 162, 143
#define COLOR_L1     255, 0, 0
#define COLOR_L2     255, 125, 0
#define COLOR_L3     255, 255, 0
#define COLOR_L4     0, 255, 255
#define COLOR_L5     0, 255, 0
#define COLOR_L6     0, 0, 255
#define COLOR_L7     255, 0, 255
#define COLOR_L8     255, 0, 255
#define COLOR_R1     0, 0, 255
#define COLOR_R2     0, 255, 0
#define COLOR_R3     0, 255, 255
#define COLOR_R4     255, 255, 0
#define COLOR_R5     255, 125, 0
#define COLOR_R6     255, 0, 0
#define COLOR_R7     255, 0, 255
#define COLOR_R8     202, 200, 232
#define COLOR_31     68, 114, 196
#define COLOR_32     255, 80, 80
#define COLOR_33     0, 51, 0
#define COLOR_34     102, 102, 255
#define COLOR_35     204, 204, 0
#define COLOR_36     102, 0, 51
#define COLOR_37     231, 232, 228
#define COLOR_38     95, 95, 95
#define COLOR_41     102, 0, 204
#define COLOR_42     255, 153, 0
#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

// unordered_map 中 std::vector<int> 不能作为 key, 需自定义
namespace std {
template<>
struct hash<std::vector<int>> {
    size_t operator()(const std::vector<int>& vec) const {
        std::size_t seed = vec.size();
        for (const auto& element : vec) {
            boost::hash_combine(seed, element);
        }
        return seed;
    }
};
}

namespace TeethColor {
static std::vector<std::vector<int>> COLORS = {
    { COLOR_Gum },
    { COLOR_L1  },
    { COLOR_L2  },
    { COLOR_L3  },
    { COLOR_L4  },
    { COLOR_L5  },
    { COLOR_L6  },
    { COLOR_L7  },
    { COLOR_L8  },
    { COLOR_R1  },
    { COLOR_R2  },
    { COLOR_R3  },
    { COLOR_R4  },
    { COLOR_R5  },
    { COLOR_R6  },
    { COLOR_R7  },
    { COLOR_R8  },
    { COLOR_31  },
    { COLOR_32  },
    { COLOR_33  },
    { COLOR_34  },
    { COLOR_35  },
    { COLOR_36  },
    { COLOR_37  },
    { COLOR_38  },
    { COLOR_41  },
    { COLOR_42  }
};

static std::vector<std::vector<int>> COLORS2 = {
    { COLOR_Gum1 },
    { COLOR_L11  },
    { COLOR_L21  },
    { COLOR_L31  },
    { COLOR_L41  },
    { COLOR_L51  },
    { COLOR_L61  },
    { COLOR_L71  },
    { COLOR_L81  },
    { COLOR_R11  },
    { COLOR_R21  },
    { COLOR_R31  },
    { COLOR_R41  },
    { COLOR_R51  },
    { COLOR_R61  },
    { COLOR_R71  },
    { COLOR_R81  },
    { COLOR_311  },
    { COLOR_321  },
    { COLOR_331  },
    { COLOR_341  },
    { COLOR_351  },
    { COLOR_361  },
    { COLOR_371  },
    { COLOR_381  },
    { COLOR_411  },
    { COLOR_421  }
};

static std::unordered_map<int, std::vector<int>> COLORSMap = {
    { 0, { COLOR_Gum } },
    { 1, { COLOR_L1 } },
    { 2, { COLOR_L2 } },
    { 3, { COLOR_L3 } },
    { 4, { COLOR_L4 } },
    { 5, { COLOR_L5 } },
    { 6, { COLOR_L6 } },
    { 7, { COLOR_L7 } },
    { 8, { COLOR_L8 } },
    { 9, { COLOR_R1 } },
    { 10, { COLOR_R2 } },
    { 11, { COLOR_R3 } },
    { 12, { COLOR_R4 } },
    { 13, { COLOR_R5 } },
    { 14, { COLOR_R6 } },
    { 15, { COLOR_R7 } },
    { 16, { COLOR_R8 } },
    { 17, { COLOR_31 } },
    { 18, { COLOR_32 } },
    { 19, { COLOR_33 } },
    { 20, { COLOR_34 } },
    { 21, { COLOR_35 } },
    { 22, { COLOR_36 } },
    { 23, { COLOR_37 } },
    { 24, { COLOR_38 } },
    { 25, { COLOR_41 } },
    { 26, { COLOR_42 } }
};

static std::unordered_map<std::vector<int>, int> reverseColorMap = {
    { { COLOR_Gum }, 0 },
    { { COLOR_L1 }, 1 },
    { { COLOR_L2 }, 2 },
    { { COLOR_L3 }, 3 },
    { { COLOR_L4 }, 4 },
    { { COLOR_L5 }, 5 },
    { { COLOR_L6 }, 6 },
    { { COLOR_L7 }, 7 },
    { { COLOR_L8 }, 8 },
    { { COLOR_R1 }, 9 },
    { { COLOR_R2 }, 10 },
    { { COLOR_R3 }, 11 },
    { { COLOR_R4 }, 12 },
    { { COLOR_R5 }, 13 },
    { { COLOR_R6 }, 14 },
    { { COLOR_R7 }, 15 },
    { { COLOR_R8 }, 16 },
    { { COLOR_31 }, 17 },
    { { COLOR_32 }, 18 },
    { { COLOR_33 }, 19 },
    { { COLOR_34 }, 20 },
    { { COLOR_35 }, 21 },
    { { COLOR_36 }, 22 },
    { { COLOR_37 }, 23 },
    { { COLOR_38 }, 24 },
    { { COLOR_41 }, 25 },
    { { COLOR_42 }, 26 }
};

}
