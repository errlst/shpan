#include "fileblob.h"
#include <iostream>

auto main() -> int {

  // 测试写文件时哈希
  // auto fb = FileBlob{"./fileblob_text_out_2K", 1024 * 2};
  // char data[1024];
  // for (auto i = 0; i < 2; ++i) {
  //   fb.write(data);
  //   std::cout << fb.trunk_hash() << "\n";
  // }
  // std::cout << fb.file_hash() << "\n";

  // 测试读文件时哈希
  // auto fb = FileBlob{"./fileblob_test_1K"};
  // std::cout << fb.file_hash() << "\n";
  // fb.read();
  // std::cout << fb.trunk_hash() << "\n";

  return 0;
}