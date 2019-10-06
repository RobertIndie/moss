#include <cstdint>
#include <map>
#include "iostream"

template <typename FuncType>
int Call(FuncType i) {
  std::cout << sizeof(static_cast<FuncType>(i)) << std::endl;
  return 0;
}

int main() {
  std::map<std::string, int (*)(int)> m;
  m["test"] = &Call<long>;
  m["test"](1);
}
