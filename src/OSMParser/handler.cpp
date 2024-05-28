#include "handler.hh"

osmparser::Handler::~Handler() {
  std::cout << "Done parsing OSM data\n";
  write_nodes_file();
  write_edges_file();
  write_tarns_file();
  std::cout << "Map bounds: " << m_map_min_lat << " -> " << m_map_max_lat
            << ", " << m_map_min_lon << " -> " << m_map_max_lon << std::endl;
  std::cout << "Cleaning up..." << std::endl;
  m_edges_file.flush();
  m_edges_file.close();
  m_nodes_file.flush();
  m_nodes_file.close();
  m_tarns_file.flush();
  m_tarns_file.close();
}

void osmparser::Handler::node(const osmium::Node& node) {
  const osmium::Location location = node.location();
  const float elevation = get_elevation(location.lat(), location.lon());
  m_nodes[node.id()] = NodeData(location, 0, elevation);
}

void osmparser::Handler::way(const osmium::Way& way) {
  std::vector<osmium::object_id_type> node_ids;

  std::string tarn_name = is_tarn(way.tags());
  const bool walkable = is_walkable(way.tags());

  if (!tarn_name.empty() && walkable) {  // TODO debug
    std::cout << "ERROR: Tarn is walkable\n";
    for (const auto& tag : way.tags()) {
      std::cout << "Tag: " << tag.key() << " = " << tag.value() << std::endl;
    }
  }

  if (!walkable && tarn_name.empty()) {
    return;  // Skip all non tarns and non-walkable ways
  }

  if (way.nodes().size() < 2) {
    std::cerr << "Way " << way.id() << " has less than 2 nodes\n";
    return;
  }

  for (const auto& node : way.nodes()) {
    if (m_nodes.find(node.ref()) == m_nodes.end()) {
      std::cerr << "Node " << node.ref() << " not found\n";
      return;  // Skip nodes not in the node list
    }
    // Check all way nodes are within lake district bounds otherwise exclude
    // from data
    auto location = m_nodes.at(node.ref()).location;
    if (location.lat() < LakeDistrict.min_lat ||
        location.lat() > LakeDistrict.max_lat ||
        location.lon() < LakeDistrict.min_lon ||
        location.lon() > LakeDistrict.max_lon) {
      return;
    }
    node_ids.push_back(node.ref());
    if (walkable) {
      // Only increment ways for walkable ways
      m_nodes[node.ref()].ways++;
    }
  }

  if (!tarn_name.empty()) {
    // Avoid duplicate tarn names
    if (m_tarn_names.find(tarn_name) == m_tarn_names.end()) {
      m_tarn_names[tarn_name] = std::vector<osmium::object_id_type>();
    }
    m_tarn_names[tarn_name].push_back(way.id());
    const size_t tarn_name_size = m_tarn_names[tarn_name].size();
    if (tarn_name_size > 1) {
      // tarn_name += " " + std::to_string(tarn_name_size);
      std::cout << "Skipping duplicate tarn: " << tarn_name << "\n";
      return;
    }
    m_tarns[way.id()] = TarnData(tarn_name, node_ids);
    return;
  }

  const int car = get_cars(way.tags());
  const int diff = get_difficulty(way.tags());

  m_ways[way.id()] = WayData(walkable, car, diff, node_ids);
}

void osmparser::Handler::read_elevation_data() {
  GDALAllRegister();

  m_poDataset =
      (GDALDataset*)GDALOpen(m_elevation_filename.c_str(), GA_ReadOnly);
  if (m_poDataset == NULL) {
    std::cerr << "Failed to open elevation dataset\n";
  }
  double adfGeoTransform[6];
  if (m_poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
    m_ele_min_lon = adfGeoTransform[0];
    m_ele_max_lon =
        adfGeoTransform[0] + adfGeoTransform[1] * m_poDataset->GetRasterXSize();
    m_ele_max_lat = adfGeoTransform[3];
    m_ele_min_lat =
        adfGeoTransform[3] + adfGeoTransform[5] * m_poDataset->GetRasterYSize();
    std::cout << "Elevation data bounds: " << m_ele_min_lat << " -> "
              << m_ele_max_lat << ", " << m_ele_min_lon << " -> "
              << m_ele_max_lon << std::endl;
  } else {
    std::cerr << "Failed to get GeoTransform\n";
  }
}

float osmparser::Handler::get_elevation(const double latitude,
                                        const double longitude) {
  if (latitude < m_ele_min_lat || latitude > m_ele_max_lat ||
      longitude < m_ele_min_lon || longitude > m_ele_max_lon) {
    // std::cerr << "Out of bounds: " << latitude << ", " << longitude << "\n";
    return -std::numeric_limits<float>::infinity();
  }
  double adfGeoTransform[6];
  if (m_poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
    int x = int((longitude - adfGeoTransform[0]) / adfGeoTransform[1]);
    int y = int((latitude - adfGeoTransform[3]) / adfGeoTransform[5]);

    GDALRasterBand* poBand = m_poDataset->GetRasterBand(1);
    float elevation;
    CPLErr err = poBand->RasterIO(GF_Read, x, y, 1, 1, &elevation, 1, 1,
                                  GDT_Float32, 0, 0);
    if (err != CE_None) {
      std::cerr << "RasterIO failed: " << CPLGetLastErrorMsg() << "\n";
      return 0;
    }
    return elevation;
  }
  std::cerr << "Failed to get GeoTransform\n";
  return 0;
}

// TODO: fix
std::string osmparser::Handler::is_tarn(const osmium::TagList& tags) {
  const char* natural = tags["natural"];
  if (natural) {
    if (std::strcmp(natural, "water") == 0) {
      const char* water = tags["water"];
      if (water) {
        if (std::strcmp(water, "river") != 0 &&
            std::strcmp(water, "stream") != 0) {
          const char* name = tags["name"];
          if (name) {
            return name;
          }
        }
      } else {
        const char* name = tags["name"];
        if (name) {
          return name;
        }
      }
    }
  }
  return std::string();
}

//TODO: fix (rivers, boundaries are being set as walkable)
bool osmparser::Handler::is_walkable(const osmium::TagList& tags) {
  const char* foot = tags["foot"];
  if (foot) {
    if (std::strcmp(foot, "no") == 0 || std::strcmp(foot, "private") == 0)
      return false;
  }

  const char* highway = tags["highway"];
  if (highway) {
    if (std::strcmp(highway, "motorway") == 0 ||
        std::strcmp(highway, "motorway_link") == 0 ||
        std::strcmp(highway, "trunk") == 0 ||
        std::strcmp(highway, "trunk_link") == 0)
      return false;
  }

  const char* natural = tags["natural"];
  if (natural) {
    if (std::strcmp(natural, "water") == 0) return false;
  }

  return true;
}

int osmparser::Handler::get_cars(const osmium::TagList& tags) {
  const char* highway = tags["highway"];
  if (highway) {
    if (std::strcmp(highway, "motorway") == 0) return 6;
    if (std::strcmp(highway, "trunk") == 0) return 5;
    if (std::strcmp(highway, "primary") == 0) return 4;
    if (std::strcmp(highway, "secondary") == 0) return 3;
    if (std::strcmp(highway, "tertiary") == 0) return 2;
    if (std::strcmp(highway, "residential") == 0 ||
        std::strcmp(highway, "unclassified") == 0 ||
        std::strcmp(highway, "service") == 0 ||
        std::strcmp(highway, "motorway_link") == 0 ||
        std::strcmp(highway, "trunk_link") == 0 ||
        std::strcmp(highway, "primary_link") == 0 ||
        std::strcmp(highway, "secondary_link") == 0 ||
        std::strcmp(highway, "tertiary_link") == 0)
      return 1;
    if (std::strcmp(highway, "track") == 0 ||
        std::strcmp(highway, "path") == 0 ||
        std::strcmp(highway, "footway") == 0 ||
        std::strcmp(highway, "bridleway") == 0 ||
        std::strcmp(highway, "cycleway") == 0)
      return 0;
  }
  return -1;
}

int osmparser::Handler::get_difficulty(const osmium::TagList& tags) {
  const char* sac_scale = tags["sac_scale"];
  if (sac_scale) {
    if (std::strcmp(sac_scale, "hiking") == 0) return 0;
    if (std::strcmp(sac_scale, "mountain_hiking") == 0) return 1;
    if (std::strcmp(sac_scale, "demanding_mountain_hiking") == 0) return 2;
    if (std::strcmp(sac_scale, "alpine_hiking") == 0) return 3;
    if (std::strcmp(sac_scale, "demanding_alpine_hiking") == 0) return 4;
    if (std::strcmp(sac_scale, "difficult_alpine_hiking") == 0) return 5;
  }
  return -1;
}

void osmparser::Handler::write_nodes_file() {
  std::cout << "Writing nodes..." << std::endl;
  m_map_max_lat = m_map_max_lon = -1000;
  m_map_min_lat = m_map_min_lon = 1000;
  m_nodes_file << "id,lat,lon,elevation\n";
  m_nodes_file << std::fixed << std::setprecision(6);
  for (const auto& node_pair : m_nodes) {
    const osmium::object_id_type node_id = node_pair.first;
    const NodeData node_data = node_pair.second;
    const int ways = node_data.ways;

    if (ways > 0) {  // Store all nodes on a way
      const osmium::Location location = node_data.location;
      const float elevation = node_data.elevation;
      if (location.lat() > m_map_max_lat) m_map_max_lat = location.lat();
      if (location.lat() < m_map_min_lat) m_map_min_lat = location.lat();
      if (location.lon() > m_map_max_lon) m_map_max_lon = location.lon();
      if (location.lon() < m_map_min_lon) m_map_min_lon = location.lon();
      m_node_counter++;
      m_nodes_file << node_id << "," << location.lat() << "," << location.lon()
                   << "," << elevation << "\n";
    }
  }
  std::cout << "Nodes: " << m_node_counter << std::endl;
}

void osmparser::Handler::write_edges_file() {
  std::cout << "Writing edges..." << std::endl;
  m_edges_file << "id,osm_id,source_id,target_id,length,slope,difficulty,cars,"
                  "geometry\n";
  m_edges_file << std::fixed << std::setprecision(6);
  for (const auto& way_pair : m_ways) {
    const osmium::object_id_type way_id = way_pair.first;
    const WayData& way_data = way_pair.second;
    osmium::object_id_type source, target = 0;
    float length = 0, ele_gain = 0, ele_loss = 0, ascend_length = 0,
          descend_length = 0;
    std::vector<osmium::object_id_type> edge_nodes;

    for (auto it = way_data.nodes.begin(); it != way_data.nodes.end(); ++it) {
      const osmium::NodeRef& node = *it;
      edge_nodes.push_back(node.ref());
      if (it == way_data.nodes.begin()) {
        source = node.ref();
      } else {
        const osmium::NodeRef& prev_node = *(it - 1);
        const auto c1 = m_nodes.at(prev_node.ref()).location;
        const auto c2 = m_nodes.at(node.ref()).location;
        const auto elevation1 = get_elevation(c1.lat(), c1.lon());
        const auto elevation2 = get_elevation(c2.lat(), c2.lon());
        const double ele_diff = elevation2 - elevation1;
        const auto distance = osmium::geom::haversine::distance(c1, c2);
        length += distance;

        if (ele_diff > 0) {
          ele_gain += ele_diff;
          ascend_length += distance;
        } else if (ele_diff < 0) {
          ele_loss += -ele_diff;
          descend_length += distance;
        }

        if ((m_nodes.at(node.ref()).ways > 1 ||
             it == way_data.nodes.end() - 1) &&
            node.ref() != source) {
          target = node.ref();
          // TODO how to process elevation ??
          const double ascent_slope =
              ascend_length > 0 ? ele_gain / ascend_length : 0;
          const double descent_slope =
              descend_length > 0 ? ele_loss / descend_length : 0;
          const double slope = (ele_gain - ele_loss) / length;
          const auto id = m_edge_counter++;
          m_edges_file << id << "," << way_id << "," << source << "," << target
                       << "," << length << "," << slope << ","
                       << way_data.difficulty << "," << way_data.cars;
          for (auto it = edge_nodes.begin(); it != edge_nodes.end(); ++it) {
            m_edges_file << "," << *it;  // Store all nodes on the edge
          }
          m_edges_file << "\n";
          source = target;
          length = 0;
          ele_gain = 0;
          ele_loss = 0;
          ascend_length = 0;
          descend_length = 0;
          edge_nodes.clear();
          edge_nodes.push_back(node.ref());
        }
      }
    }
  }
  std::cout << "Edges: " << m_edge_counter << std::endl;
}

std::pair<const double, const double> osmparser::Handler::latlon_to_utm(
    const double lat, const double lon) {
  // Constants
  const double R = 6371e3;  // Earth's radius in meters
  const double TO_RAD =
      M_PI / 180.0;  // Conversion factor from degrees to radians

  // Convert latitude and longitude to radians
  double lat_rad = lat * TO_RAD;
  double lon_rad = lon * TO_RAD;

  // Convert latitude and longitude to meters
  const double x = R * lon_rad * cos(lat_rad);
  const double y = R * lat_rad;

  return std::make_pair(x, y);
}

std::pair<osmium::Location, double>
osmparser::Handler::get_tarn_location_and_area(
    const std::vector<osmium::object_id_type>& nodes) {
  double lat = 0, lon = 0, area = 0;
  std::vector<std::pair<const double, const double>> utm_coords;

  for (size_t i = 0; i < nodes.size(); i++) {
    const osmium::object_id_type node_id = nodes[i];
    const osmium::Location location = m_nodes.at(node_id).location;
    lat += location.lat();
    lon += location.lon();

    utm_coords.push_back(latlon_to_utm(location.lat(), location.lon()));
  }

  for (size_t i = 0; i < nodes.size(); i++) {
    const auto& p1 = utm_coords[i];
    const auto& p2 = utm_coords[(i + 1) % nodes.size()];
    area += p1.first * p2.second - p2.first * p1.second;
  }
  area = std::abs(area) / 2.f;

  return std::make_pair(
      osmium::Location(lon / nodes.size(), lat / nodes.size()), area);
}

void osmparser::Handler::write_tarns_file() {
  std::cout << "Writing tarns..." << std::endl;
  m_tarns_file << "osm_id,name,lat,lon,elevation,area\n";
  m_tarns_file << std::fixed << std::setprecision(6);
  for (const auto& tarn_pair : m_tarns) {
    const osmium::object_id_type tarn_id = tarn_pair.first;
    const TarnData& tarn_data = tarn_pair.second;
    osmium::Location location;
    double area;
    std::tie(location, area) = get_tarn_location_and_area(tarn_data.nodes);
    auto elevation = get_elevation(location.lat(), location.lon());
    m_tarn_counter++;
    m_tarns_file << tarn_id << ",\"" << tarn_data.name << "\","
                 << location.lat() << "," << location.lon() << "," << elevation
                 << "," << std::round(area) << "\n";
  }
  std::cout << "Tarns: " << m_tarn_counter << std::endl;
}
