#include "handler.hh"

#define DATA_DIR "data/"
#define DEFAULT_OSM_FILE "cumbria-latest.osm.pbf"
#define DEFAULT_ELEVATION_FILE "topography.tif"

void handel_args(int argc, char* argv[], std::string& osm_filename,
                 std::string& elevation_filename) {
  if (argc > 2) {
    osm_filename = argv[1];
    elevation_filename = argv[2];
  } else {
    std::cout << "No input files specified" << std::endl;
    std::cout << "Usage: " << argv[0] << " <osmfile> <DEMfile>" << std::endl;
    std::cout << "Using default files: " << osm_filename << ", "
              << elevation_filename << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::string osm_filename = "data/cumbria-latest.osm.pbf";
  std::string elevation_filename = "data/topography.tif";
  //TODO: Allow non hardcoded filenames
  std::string edges_filename = "data/edges.csv";
  std::string nodes_filename = "data/nodes.csv";
  std::string tarns_filename = "data/tarns.csv";

  handel_args(argc, argv, osm_filename, elevation_filename);

  osmium::io::File osm_file(osm_filename);
  osmium::io::Reader reader(osm_file);

  osmparser::Handler handler(elevation_filename, edges_filename, nodes_filename,
                             tarns_filename);
  osmium::apply(reader, handler);

  reader.close();
}