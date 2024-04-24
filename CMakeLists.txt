cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 20)

project(lzhb VERSION 0.9 LANGUAGES CXX C)

add_compile_options(-Wall -Wextra -Ofast)

add_executable(lzhb3
  lzhb3_main.cpp lzhb3.cpp lzhb3sa.cpp lzhb_common.cpp suffixtree.cpp truncatedSuffixArray.cpp libsais.c twodminheap.cpp
)

add_executable(lzhb4
  lzhb4_main.cpp lzhb4.cpp lzhb4sa.cpp lzhb_common.cpp suffixtree.cpp truncatedSuffixArray.cpp libsais.c twodminheap.cpp
)
