cmake_minimum_required(VERSION 3.25)
project(cppBabySTL VERSION 1.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 添加编译选项
add_compile_options(-Wall -Wextra -Wpedantic)

# 列出所有的头文件
set(HEADER_FILES
        src/baby_string.h
        src/baby_vector.h
        src/baby_array.h
        src/baby_forwardlist.h
        src/baby_list.h
        src/baby_deque.h
        src/baby_queue.h
        src/baby_stack.h
        src/baby_priorityqueue.h
        src/rb_tree.h
        src/baby_map.h
        src/baby_set.h
        src/baby_multimap.h
        src/baby_multiset.h
        )

# 添加可执行目标
add_executable(cppBabySTL main.cpp ${HEADER_FILES})

# 设置目标属性
target_include_directories(cppBabySTL PRIVATE src)