#pragma once

#include <ostream>

#if defined(__SIZEOF_INT128__)
inline std::ostream& operator<<(std::ostream& stream, unsigned __int128 value) {
  char buffer[40];
  char* end = buffer + sizeof(buffer);
  char* current = end;
  do {
    *--current = static_cast<char>('0' + value % 10);
    value /= 10;
  } while (value != 0);
  return stream.write(current, end - current);
}

inline std::ostream& operator<<(std::ostream& stream, __int128 value) {
  const bool negative = value < 0;
  unsigned __int128 magnitude = static_cast<unsigned __int128>(value);
  if (negative) {
    magnitude = static_cast<unsigned __int128>(-(value + 1)) + 1;
  }
  char buffer[41];
  char* end = buffer + sizeof(buffer);
  char* current = end;
  do {
    *--current = static_cast<char>('0' + magnitude % 10);
    magnitude /= 10;
  } while (magnitude != 0);
  if (negative) {
    *--current = '-';
  }
  return stream.write(current, end - current);
}
#endif
