#include <boost/property_tree/ptree_fwd.hpp>
#include <cstdint>
#include <cstdlib>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "baldr/graphreader.h"
#include "baldr/traffictile.h"
#include "config.h"
#include "third_party/microtar/src/microtar.h"

uint32_t indices[] = {
  81,82, 83, 84, 87, 88, 89, 90, 93, 94, 95, 105, 106, 107, 108, 117, 118, 121, 122,
  125, 126, 153, 154, 157, 158, 161, 162, 452, 453, 458
};

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
  if (argc != 3) {
    LOG_ERROR("Usage: " + std::string(argv[0]) + " config/file.json <speed>");
    return 1;
  }
  std::string config_file(argv[1]);
  int target_speed = std::atoi(argv[2]);
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

  // Road A4204 is in this graph
  int err = mtar_find(&tar, "0/003/194.gph", &tar_header);
  if (err != 0) {
    LOG_ERROR("mtar cannot find the file in tar, error code is: " + std::to_string(err));
    return 1;
  }

  valhalla::baldr::TrafficTile tile(
    std::make_unique<MMapGraphMemory>(memory,
                                      reinterpret_cast<char*>(tar.stream) + tar.pos +
                                          sizeof(mtar_raw_header_t_),
                                      tar_header.size));

  valhalla::baldr::GraphId tile_id(tile.header->tile_id);

  for (auto index : indices) {
    LOG_INFO("Adding the edge with index: " + std::to_string(index));
    valhalla::baldr::TrafficSpeed* target_edge =
      const_cast<valhalla::baldr::TrafficSpeed*>(tile.speeds + index);

    target_edge->breakpoint1 = 255;
    target_edge->encoded_speed1 = target_speed >> 1;
    target_edge->overall_encoded_speed = target_speed >> 1;
  }
  LOG_INFO("Adding live traffic data ...");
}
