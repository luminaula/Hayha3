#include "keyboard.hpp"
#include <windows.h>
namespace OS {
bool checkIfPressed(int key) { return GetAsyncKeyState(key); }
} // namespace OS