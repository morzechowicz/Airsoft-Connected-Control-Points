#ifndef STRING_SPLITTER_H
#define STRING_SPLITTER_H

#include <Arduino.h>

class StringSplitter {
  public:
    StringSplitter(char delimiter = '/'); 
    int split(const String &input);
    int getItemCount();
    String getItem(int index);
    void clear();
    void setDelimiter(char delimiter);

  private:
    String _parts[20]; 
    int _count;
    char _delimiter;
};

#endif