#include <set>
#include <iostream>
#include <fstream>
#include <filesystem>
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
    for(int i = 0; i < argc; i++) {
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

  bool isKeyProvided(T key) {
    return keys.find(key) != keys.end();
  }

  bool isArgProvided(string arg) {
    return args.find(arg) != args.end();
  }

  bool isProvided() {
    return provided;
  }

  size_t getParamsCount() {
    return params.size();
  }

  string getParam(size_t index) {
    if(index >= params.size()) return "";
    return params[index];
  }

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
};

enum class EncodingType {
  Null = 0,
  Auto = 1,
  BrickImage,
  ArduinoArray
};

void encodeImage(std::string in, std::string out, EncodingType type = EncodingType::BrickImage) {
  sf::Image image;
  if(!image.loadFromFile(in)) {
    std::cout << "Error file reading!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sf::Vector2u size = image.getSize();
  if(size.x > 255 || size.y > 255) {
    std::cout << "Input image is too big!" << std::endl;
    std::cout << "Image size is: " << size.x << "x" << size.y << "." << std::endl;
    std::cout << "Max size is: 255x255." << std::endl;
    exit(EXIT_FAILURE);
  }
  uint32_t dataSize = size.x * size.y * 4;

  if(fs::exists(out)) {
    fs::remove(out);
  }

  std::ofstream file(out);

  if(!file) {
    std::cout << "Error opening file for writing!" << std::endl;
    exit(EXIT_FAILURE);
  }

  file << "flBIv1";
  file.put(static_cast<uint8_t>(size.x));
  file.put(static_cast<uint8_t>(size.y));
  file.write(reinterpret_cast<const char*>(image.getPixelsPtr()), dataSize);
  file.close();
  std::cout << "Image succefully encoded to " << out << "." << std::endl;
}

void decodeImage(std::string in, std::string out) {
  std::ifstream file(in);

  if(!file) {
    std::cout << "Error opening file for reading!" << std::endl;
    exit(EXIT_FAILURE);
  }

  char buff[7] {};
  file.read(buff, 6);
  if(file.gcount() != 6) {
    std::cout << "File header reading error!" << std::endl;
    exit(EXIT_FAILURE);
  }

  sf::Vector2<uint8_t> size;
  file.read(reinterpret_cast<char*>(&size), 2);

  if(size.x == 0 || size.y == 0) {
    std::cout << "File image size invalid!" << std::endl;
    std::cout << "Readed size: " << size.x << "x" << size.y << std::endl;
    exit(EXIT_FAILURE);
  }

  std::vector<sf::Color> data;
  data.resize(static_cast<uint16_t>(size.x) * size.y);

  file.read(reinterpret_cast<char*>(data.data()), static_cast<uint64_t>(size.x) * size.y * 4);
  if(file.gcount() != static_cast<uint64_t>(size.x) * size.y * 4) {
    std::cout << "File image data reading error!" << std::endl;
    exit(EXIT_FAILURE);
  }
  file.close();

  if(fs::exists(out)) {
    fs::remove(out);
  }

  sf::Image image;
  image.create(size.x, size.y, reinterpret_cast<sf::Uint8*>(data.data()));
  if(!image.saveToFile(out)) {
    std::cout << "Error saving output file!" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << "Image succefully decoded to " << out << "." << std::endl;
}

int main(int argc, char** argv) {
  ArgsMan<char> arguments(argc, argv);
#ifdef DEBUG
  arguments.debug();
#endif // DEBUG

  if(!arguments.isProvided()) {
    std::cout << std::endl << "No input file!" << std::endl;
    return EXIT_FAILURE;
  }

  if(arguments.isKeyProvided('d') || arguments.isArgProvided("decode")) {
    if(arguments.getParamsCount() > 2) {
      decodeImage(arguments.getParam(1), arguments.getParam(2));
    }
    else {
      decodeImage(arguments.getParam(1), "out.bin");
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