#include "helper.h"
#include <fstream>
#include <iostream>

bool check_file_exists (std::string& filename)
{
  bool exists(false);
  std::string answer;
  std::ifstream fin;

  fin.open (filename.c_str());
  if (fin.fail())
    return(false);
  fin.close();

  std::cerr << "The file <"<<filename<<"> exists, overwrite? ";
  std::cin >> answer;
  if (answer == "yes" || answer =="y" || answer == "Yes" || answer == "Y" )
    return(false);

  return(true);
}
