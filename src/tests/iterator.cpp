#include <iostream>

#include "../connection.hpp"

using namespace xpp;

int main(int argc, char ** argv)
{
  connection c("");

  auto reply = c.query_tree(c.root());

  std::cerr << "#windows (children_len): " << reply->children_len << std::endl;
  std::cerr << "#windows (length):       " << reply->length << std::endl;

  std::cerr << std::hex;
  for (auto & window : reply) {
    std::cerr << "0x" << window << "; ";
  }
  std::cerr << std::dec << std::endl;;

  std::cerr << std::hex;
  for (auto it = reply.begin(); it != reply.end(); ++it) {
    std::cerr << "0x" << *it << "; ";
  }
  std::cerr << std::dec << std::endl;;

  std::cerr << std::hex;
  auto it = reply.begin();
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "++it: " << *(++it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "it++: " << *(it++) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "++it: " << *(++it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "--it: " << *(--it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "it--: " << *(it--) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << std::dec << std::endl;;

  return EXIT_SUCCESS;
}
