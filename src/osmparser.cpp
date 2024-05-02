#include <osmium/io/pbf_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/geom/haversine.hpp>
#include <iostream>
#include <fstream>
#include <gdal_priv.h>

class SlopeHandler : public osmium::handler::Handler {
public:
    SlopeHandler(std::string data_filename, std::string edges_filename, std::string nodes_filename) : m_edges_file(edges_filename), m_nodes_file(nodes_filename), m_elevation_filename(data_filename) {
        std::cout << "Reading elevation data..." << std::endl;
        read_elevation_data();
        std::cout << "Reading OSM data...\n";
    }

    ~SlopeHandler() {
        std::cout << "Done parsing OSM data\n";
        write_nodes();
        write_edges();
        std::cout << "Map bounds: " << m_map_min_lat << " -> " << m_map_max_lat << ", " << m_map_min_lon << " -> " << m_map_max_lon << std::endl;
        std::cout << "Cleaning up..." << std::endl;
        m_edges_file.flush();
        m_edges_file.close();
        m_nodes_file.flush();
        m_nodes_file.close();
    }

    void node(const osmium::Node& node) {
        const osmium::Location location = node.location();
        const float elevation = get_elevation(location.lat(), location.lon());
        m_nodes[node.id()] = NodeData(location, 0, elevation);
    }

    void way(const osmium::Way& way) {
        const bool walkable = is_walkable(way.tags());
        if (!walkable) {
            return; // Skip non-walkable ways
        }
        const int car = cars(way.tags());
        const int diff = difficulty(way.tags());
        const osmium::NodeRefList& nodes = way.nodes();
        std::vector<long> node_ids;
        if(nodes.size() < 2) {
            std::cerr << "Way " << way.id() << " has less than 2 nodes\n";
            return;
        }
        for (size_t i = 0; i < nodes.size(); ++i) {
            if(m_nodes.find(nodes[i].ref()) == m_nodes.end()) {
                std::cerr << "Way node not in nodes" << nodes[i].ref() << " not found\n";
                continue; // Skip nodes not in the node list
            }
            node_ids.push_back(nodes[i].ref());
            m_nodes[nodes[i].ref()].ways++;
        }
        m_ways[way.id()] = WayData(walkable, car, diff, node_ids);
    }

private:
    void read_elevation_data() {
        GDALAllRegister();

        m_poDataset = (GDALDataset *) GDALOpen(m_elevation_filename.c_str(), GA_ReadOnly);
        if (m_poDataset == NULL) {
            std::cerr << "Failed to open elevation dataset\n";
        }
        double adfGeoTransform[6];
        if (m_poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            m_ele_min_lon = adfGeoTransform[0];
            m_ele_max_lon = adfGeoTransform[0] + adfGeoTransform[1] * m_poDataset->GetRasterXSize();
            m_ele_max_lat = adfGeoTransform[3];
            m_ele_min_lat = adfGeoTransform[3] + adfGeoTransform[5] * m_poDataset->GetRasterYSize();
            std::cout << "Elevation data bounds: " << m_ele_min_lat << " -> " << m_ele_max_lat << ", " << m_ele_min_lon << " -> " << m_ele_max_lon << std::endl;
        } else {
            std::cerr << "Failed to get GeoTransform\n";
        }
    }

    float get_elevation(const double latitude, const double longitude) {
        if (latitude < m_ele_min_lat || latitude > m_ele_max_lat || longitude < m_ele_min_lon || longitude > m_ele_max_lon) {
            //std::cerr << "Out of bounds: " << latitude << ", " << longitude << "\n";
            return 0;
        }
        double adfGeoTransform[6];
        if (m_poDataset->GetGeoTransform(adfGeoTransform) == CE_None) {
            int x = int((longitude - adfGeoTransform[0]) / adfGeoTransform[1]);
            int y = int((latitude - adfGeoTransform[3]) / adfGeoTransform[5]);

            GDALRasterBand *poBand = m_poDataset->GetRasterBand(1);
            float elevation;
            CPLErr err = poBand->RasterIO(GF_Read, x, y, 1, 1, &elevation, 1, 1, GDT_Float32, 0, 0);
            if (err != CE_None) {
                std::cerr << "RasterIO failed: " << CPLGetLastErrorMsg() << "\n";
                return 0;
            }
            return elevation;
        }
        std::cerr << "Failed to get GeoTransform\n";
        return 0;
    }

    bool is_walkable(const osmium::TagList& tags) {
        const char* foot = tags["foot"];
        if (foot) {
            if (std::strcmp(foot, "no") == 0 || std::strcmp(foot, "private") == 0)
                return false;
        }

        const char* highway = tags["highway"];
        if (highway) {
            if (std::strcmp(highway, "motorway") == 0 || std::strcmp(highway, "motorway_link") == 0 || std::strcmp(highway, "trunk") == 0 || std::strcmp(highway, "trunk_link") == 0)
                return false;
        }

        return true;
    }

    int cars(const osmium::TagList& tags) {
        const char* highway = tags["highway"];
        if (highway) {
            if (std::strcmp(highway, "motorway") == 0)
                return 6;
            if (std::strcmp(highway, "trunk") == 0)
                return 5;
            if (std::strcmp(highway, "primary") == 0)
                return 4;
            if (std::strcmp(highway, "secondary") == 0)
                return 3;
            if (std::strcmp(highway, "tertiary") == 0)
                return 2;
            if (std::strcmp(highway, "residential") == 0 || std::strcmp(highway, "unclassified") == 0 || std::strcmp(highway, "service") == 0 || std::strcmp(highway, "motorway_link") == 0 || std::strcmp(highway, "trunk_link") == 0 || std::strcmp(highway, "primary_link") == 0 || std::strcmp(highway, "secondary_link") == 0 || std::strcmp(highway, "tertiary_link") == 0)
                return 1;
            if (std::strcmp(highway, "track") == 0 || std::strcmp(highway, "path") == 0 || std::strcmp(highway, "footway") == 0 || std::strcmp(highway, "bridleway") == 0 || std::strcmp(highway, "cycleway") == 0)
                return 0;
        }
        return -1;
    }

    int difficulty(const osmium::TagList& tags) {
        const char* sac_scale = tags["sac_scale"];
        if (sac_scale) {
            if (std::strcmp(sac_scale, "hiking") == 0)
                return 0;
            if (std::strcmp(sac_scale, "mountain_hiking") == 0)
                return 1;
            if (std::strcmp(sac_scale, "demanding_mountain_hiking") == 0)
                return 2;
            if (std::strcmp(sac_scale, "alpine_hiking") == 0)
                return 3;
            if (std::strcmp(sac_scale, "demanding_alpine_hiking") == 0)
                return 4;
            if (std::strcmp(sac_scale, "difficult_alpine_hiking") == 0)
                return 5;
        }
        return -1;
    }

    void write_nodes() {
        std::cout << "Writing nodes..." << std::endl;
        m_map_max_lat = m_map_max_lon = -1000;
        m_map_min_lat = m_map_min_lon = 1000;
        m_nodes_file << "id,lat,lon,elevation\n";
        m_nodes_file << std::fixed << std::setprecision(6);
        for(const auto& node_pair : m_nodes) {
            const long node_id = node_pair.first;
            const NodeData node_data = node_pair.second;
            const osmium::Location location = node_data.location;
            const float elevation = node_data.elevation;
            const int ways = node_data.ways;

            if (ways > 0) { // Store all nodes on a way 
                if (location.lat() > m_map_max_lat) m_map_max_lat = location.lat();
                if (location.lat() < m_map_min_lat) m_map_min_lat = location.lat();
                if (location.lon() > m_map_max_lon) m_map_max_lon = location.lon();
                if (location.lon() < m_map_min_lon) m_map_min_lon = location.lon();
                m_node_counter++;
                m_nodes_file << node_id << "," << location.lat() << "," << location.lon() << "," << elevation << "\n";
            }
        }
        std::cout << "Nodes: " << m_node_counter << std::endl;
    }

    void write_edges() {
        std::cout << "Writing edges..." << std::endl;
        m_edges_file << "id,osm_id,source_id,target_id,length,slope,difficulty,cars,geometry\n";
        m_edges_file << std::fixed << std::setprecision(6);
        for(const auto& way_pair : m_ways) {
            const long way_id = way_pair.first;
            const WayData& way_data = way_pair.second;
            long source, target = 0;
            float length = 0, ele_gain = 0, ele_loss = 0, ascend_lenght = 0, descend_lenght = 0;
            std::vector<long> edge_nodes;

            for(auto it = way_data.nodes.begin(); it != way_data.nodes.end(); ++it) {
                const osmium::NodeRef& node = *it;
                edge_nodes.push_back(node.ref());
                if(it == way_data.nodes.begin()){
                    source = node.ref();
                } else {
                    const osmium::NodeRef& prev_node = *(it - 1);
                    auto c1 = m_nodes.at(prev_node.ref()).location;
                    auto c2 = m_nodes.at(node.ref()).location;
                    auto elevation1 = get_elevation(c1.lat(), c1.lon());
                    auto elevation2 = get_elevation(c2.lat(), c2.lon());
                    const double distance = osmium::geom::haversine::distance(c1, c2);
                    length += distance;
                    const double ele_diff = elevation2 - elevation1;
                    if (ele_diff > 0) {
                        ele_gain += ele_diff;
                        ascend_lenght += distance;
                    } else {
                        ele_loss += -ele_diff;
                        descend_lenght += distance;
                    }

                    if((m_nodes.at(node.ref()).ways > 1 && node.ref() != source) || it == way_data.nodes.end() - 1){
                        target = node.ref();
                        // TODO add elevation gain and loss
                        const double slope = (ele_gain - ele_loss) / length;
                        const long id = m_edge_counter++;
                        m_edges_file << id << "," << way_id << "," << source << "," << target << "," << length << "," << slope << "," << way_data.difficulty << "," << way_data.cars;
                        for(auto it = edge_nodes.begin(); it != edge_nodes.end(); ++it) {
                            m_edges_file << "," << *it; // Store all nodes on the edge
                        }
                        m_edges_file << "\n";
                        source = target;
                        length = 0;
                        ele_gain = 0;
                        ele_loss = 0;
                        ascend_lenght = 0;
                        descend_lenght = 0;
                        edge_nodes.clear();
                        edge_nodes.push_back(node.ref());
                    }
                }   
            }
        }
        std::cout << "Edges: " << m_edge_counter << std::endl;
    }

private:
    struct NodeData {
        NodeData() : location(osmium::Location()), ways(0), elevation(0) {}
        NodeData(osmium::Location location, int ways, float elevation) : location(location), ways(ways), elevation(elevation) {}
        osmium::Location location;
        int ways;
        float elevation;
    };
    std::unordered_map<long, NodeData> m_nodes;
    std::ofstream m_edges_file;
    std::ofstream m_nodes_file;
    long m_node_counter = 0;
    long m_edge_counter = 0;
    GDALDataset* m_poDataset;
    std::string m_elevation_filename;
    double m_ele_min_lon, m_map_min_lon;
    double m_ele_max_lon, m_map_max_lon;
    double m_ele_min_lat, m_map_min_lat;
    double m_ele_max_lat, m_map_max_lat;
    struct WayData {
        WayData() : walkable(false), cars(-1), difficulty(-1), nodes(std::vector<long>()) {}
        WayData(bool walkable, int cars, int difficulty, std::vector<long> nodes) : walkable(walkable), cars(cars), difficulty(difficulty), nodes(nodes) {}
        bool walkable;
        int cars;
        int difficulty;
        std::vector<long> nodes;
    };
    std::unordered_map<long, WayData> m_ways;
};

int main(int argc, char* argv[]) {
    std::string osm_filename = "data/cumbria-latest.osm.pbf";
    std::string elevation_filename = "data/topography.tif";
    std::string edges_filename = "data/edges.csv";
    std::string nodes_filename = "data/nodes.csv";
    if (argc > 2) {
        osm_filename = argv[1];
        elevation_filename = argv[2];
    } else {
        std::cout << "No input files specified" << std::endl;
        std::cout << "Usage: " << argv[0] << " <osmfile> <DEMfile>" << std::endl;
        std::cout << "Using default files: " << osm_filename << ", " << elevation_filename << std::endl;
    }
    osmium::io::File osm_file(osm_filename);
    osmium::io::Reader reader(osm_file);

    SlopeHandler handler(elevation_filename, edges_filename, nodes_filename);
    osmium::apply(reader, handler);

    reader.close();
}