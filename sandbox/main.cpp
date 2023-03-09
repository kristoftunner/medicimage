#include <iostream>
#include <fstream>


int main()
{
  std::fstream file;
  file.open("asd", std::ios::in | std::ios::out | std::ios::trunc);
  if(file.good())
  {
    const std::string data = "hello world";
    file << data;
  }
}