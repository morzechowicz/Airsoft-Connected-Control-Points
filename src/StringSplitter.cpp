#include "StringSplitter.h"

StringSplitter::StringSplitter(char delimiter) {
  _delimiter = delimiter;
  _count = 0;
}

int StringSplitter::split(const String &input) {
  _count = 0;
  int lastIndex = 0;
  
  for (int i = 0; i <= input.length(); i++) {
    if (i == input.length() || input[i] == _delimiter) {
      if (_count < (sizeof(_parts)/sizeof(_parts[0]))) {
        _parts[_count++] = input.substring(lastIndex, i);
      }
      lastIndex = i + 1;
    }
  }
  return _count;
}

int StringSplitter::getItemCount() {
  return _count;
}

String StringSplitter::getItem(int index) {
  if (index >= 0 && index < _count) {
    return _parts[index];
  }
  return String();
}

void StringSplitter::clear() {
  _count = 0;
}

void StringSplitter::setDelimiter(char delimiter) {
  _delimiter = delimiter;
}