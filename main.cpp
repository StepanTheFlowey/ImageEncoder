#include <set>
#include <vector>
#include <iostream>
#include <fstream>
#include <WinSock2.h>
#include <SFML/Graphics.hpp>

namespace fs = std::filesystem;

template <typename T> class ArgsMan {
  using string = std::basic_string<T>;

  std::set<T> keys;
  std::set<string> args;
  std::vector<string> params;
  bool provided = false;
public:

  ArgsMan(int argc, T** argv) {
    for(int i = 0; i < argc; ++i) {
      string arg(argv[i]);
      if(arg.substr(0, 2) == "--") {
        args.insert(arg.substr(2));
        continue;
      }
      if(arg.substr(0, 1) == "-") {
        for(auto& i : arg.substr(1)) {
          keys.insert(i);
        }
        continue;
      }
      params.push_back(arg);
    }
    provided = argc != 1;
  }

  bool isKeyProvided(const T key) const {
    return keys.find(key) != keys.end();
  }

  bool isArgProvided(const string& arg) const {
    return args.find(arg) != args.end();
  }

  bool isProvided() {
    return provided;
  }

  size_t getParamsCount() const {
    return params.size();
  }

  string getParam(const size_t& index) const {
    if(index >= params.size()) return "";
    return params[index];
  }

#ifdef DEBUG
  void debug() {
    std::cout << "Keys: ";
    for(auto& i : keys) {
      std::cout << i;
    }
    std::cout << std::endl;
    std::cout << "Arguments: ";
    for(auto& i : args) {
      std::cout << i << " ";
    }
    std::cout << std::endl;
    std::cout << "Parameters: ";
    for(auto& i : params) {
      std::cout << i << " ";
    }
    std::cout << std::endl;
  }
#endif // DEBUG
};

void encodeImage(const std::string& in, const std::string& out) {
  sf::Image image;
  if(!image.loadFromFile(in)) {
    std::cout << "Error file reading!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sf::Vector2u size = image.getSize();
  if(size.x > UINT16_MAX || size.y > UINT16_MAX) {
    std::cout << "Input image is too big!\n";
    std::cout << "Image size is: " << size.x << 'x' << size.y << '\n';
    std::cout << "Max size is: " << UINT16_MAX << 'x' << UINT16_MAX << std::endl;
    exit(EXIT_FAILURE);
  }
  uint32_t dataSize = size.x * size.y * 4;

  std::ofstream file(out, std::ios_base::binary | std::ios_base::trunc);

  if(!file) {
    std::cout << "Error opening file for writing!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sf::Vector2<uint16_t> size16(
    htons(static_cast<uint16_t>(size.x)),
    htons(static_cast<uint16_t>(size.y))
  );

  file << "flBIv1";
  file.write(reinterpret_cast<char*>(&size16), sizeof(size16));
  file.write(reinterpret_cast<const char*>(image.getPixelsPtr()), dataSize);
  file.close();
}

void decodeImage(const std::string& in, const std::string& out) {
  std::ifstream file(in, std::ios_base::binary);

  if(!file) {
    std::cout << "Error opening file for reading!" << std::endl;
    exit(EXIT_FAILURE);
  }

  char buff[7]{};
  file.read(buff, 6);
  if(file.gcount() != 6) {
    std::cout << "File header reading error!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sf::Vector2<uint16_t> size;
  if(file.read(reinterpret_cast<char*>(&size), sizeof(size)).gcount() != sizeof(size)) {

  }
  size.x = ntohs(size.x);
  size.y = ntohs(size.y);

  if(size.x == 0 || size.y == 0) {
    std::cout << "File image size invalid!\n";
    std::cout << "Readed size: " << size.x << 'x' << size.y << std::endl;
    exit(EXIT_FAILURE);
  }

  std::vector<sf::Color> data;
  data.resize(static_cast<size_t>(size.x) * size.y);

  file.read(reinterpret_cast<char*>(data.data()), static_cast<uint64_t>(size.x) * size.y * 4);
  if(file.gcount() != static_cast<uint64_t>(size.x) * size.y * 4) {
    std::cout << "File image data reading error!" << std::endl;
    exit(EXIT_FAILURE);
  }
  file.close();

  sf::Image image;
  image.create(size.x, size.y, reinterpret_cast<sf::Uint8*>(data.data()));
  if(!image.saveToFile(out)) {
    std::cout << "Error saving output file!" << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char** argv) {
  std::ios::sync_with_stdio(false);

  ArgsMan<char> arguments(argc, argv);

#ifdef DEBUG
  arguments.debug();
#endif // DEBUG

  if(!arguments.isProvided() || arguments.getParamsCount() == 0) {
    std::cout << "No input file!" << std::endl;
    return EXIT_FAILURE;
  }

  if(arguments.isKeyProvided('d') || arguments.isArgProvided("decode")) {
    if(arguments.getParamsCount() > 2) {
      decodeImage(arguments.getParam(1), arguments.getParam(2));
    }
    else {
      decodeImage(arguments.getParam(1), "out.png");
    }
  }
  else {
    if(arguments.getParamsCount() > 2) {
      encodeImage(arguments.getParam(1), arguments.getParam(2));
    }
    else {
      encodeImage(arguments.getParam(1), "out.bin");
    }
  }

  return EXIT_SUCCESS;
}
