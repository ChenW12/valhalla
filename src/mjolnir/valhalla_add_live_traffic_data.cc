#include <boost/property_tree/ptree_fwd.hpp>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "baldr/graphreader.h"
#include "baldr/traffictile.h"
#include "config.h"
#include "third_party/microtar/src/microtar.h"

/** Copy of raw header for use with sizeof() **/
typedef struct {
  char name[100];
  char mode[8];
  char owner[8];
  char group[8];
  char size[12];
  char mtime[12];
  char checksum[8];
  char type;
  char linkname[100];
  char _padding[255];
} mtar_raw_header_t_;

// Copy from test/
struct MMap {
  MMap(const char* filename) {
    fd = open(filename, O_RDWR);
    struct stat s;
#ifdef _MSC_VER
    _fstat64(fd, &s);
#else
    fstat(fd, &s);
#endif
    data = mmap(0, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    length = s.st_size;
  }

  ~MMap() {
    munmap(data, length);
    close(fd);
  }

  int fd;
  void* data;
  size_t length;
};

class MMapGraphMemory final : public valhalla::baldr::GraphMemory {
public:
  MMapGraphMemory(std::shared_ptr<MMap> mmap, char* data_, size_t size_) : mmap_(std::move(mmap)) {
    data = data_;
    size = size_;
  }

private:
  const std::shared_ptr<MMap> mmap_;
};

int main(int argc, char** argv) {
  if ( argc != 2) {
    LOG_ERROR("Usage: " + std::string(argv[0]) + " config/file.json");
    return 1;
  }
  std::string config_file(argv[1]);
  boost::property_tree::ptree config = valhalla::config(config_file);


  const auto memory =
      std::make_shared<MMap>(config.get<std::string>("mjolnir.traffic_extract").c_str());

  mtar_t tar;
  tar.stream = memory->data;
  tar.read = [](mtar_t* tar, void* data, unsigned size) -> int {
    memcpy(data, reinterpret_cast<char*>(tar->stream) + tar->pos, size);
    return MTAR_ESUCCESS;
  };
  tar.write = [](mtar_t* tar, const void* data, unsigned size) -> int {
    memcpy(reinterpret_cast<char*>(tar->stream) + tar->pos, data, size);
    return MTAR_ESUCCESS;
  };
  tar.seek = [](mtar_t* /*tar*/, unsigned /*pos*/) -> int { return MTAR_ESUCCESS; };
  tar.close = [](mtar_t* /*tar*/) -> int { return MTAR_ESUCCESS; };

  valhalla::baldr::GraphReader reader(config.get_child("mjolnir"));
  mtar_header_t tar_header;

  // TODO: using the name of the target tile
  mtar_find(&tar, "", &tar_header);

  valhalla::baldr::TrafficTile tile(
    std::make_unique<MMapGraphMemory>(memory,
                                      reinterpret_cast<char*>(tar.stream) + tar.pos +
                                          sizeof(mtar_raw_header_t_),
                                      tar_header.size));

  valhalla::baldr::GraphId tile_id(tile.header->tile_id);

  auto speed = valhalla::baldr::TrafficSpeed();
  speed.encoded_speed1 = 50 >> 1;
  speed.breakpoint1 = 255;
  printf("Adding live traffic data ...\n");


  printf("speed.encoded_speed1: %d\n", speed.get_speed(0));
}
