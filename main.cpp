
#include <cstring>
#include "src/baby_string.h"
#include "src/baby_vector.h"
#include "src/baby_array.h"
#include <string_view>
#include <vector>
#include <chrono>

//// 记录开始时间点
//auto start = std::chrono::high_resolution_clock::now();
//
//vector<Test> _M_arr(10000000);
//
//// 记录结束时间点
//auto end = std::chrono::high_resolution_clock::now();
//
//// 计算时间间隔
//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
//// 输出执行时间
//std::cout << "time used: " << duration.count() << " ms" << std::endl;

using namespace std;

#include "src/baby_forwardlist.h"
#include "src/baby_list.h"
#include "src/baby_deque.h"
#include "src/baby_stack.h"
#include "src/baby_priorityqueue.h"

#include "src/rb_tree.h"
#include "src/baby_map.h"
#include "src/baby_set.h"
#include "src/baby_multimap.h"
#include <array>

int main() {
    BList<int> list = {1, 2, 3};
    list.insert(list.begin(), 4);

    for (const auto &it : list) {
        cout << it << " ";
    }
    return 0;
}