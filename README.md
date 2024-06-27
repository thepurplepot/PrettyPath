# Pretty Paths

# Table of contents
 - [Dependencies](#dependencies)
 - [Setup](#setup)
 - [Todos](#todos)

# Dependencies
- CMake >= 3.11
- C++ 17 compatible compiler
- libosmium 
- protozero
- nlohmann_json
- ZLIB
- GDAL
- Python: staticmaps

# Setup
Get OSM data for region from: https://download.geofabrik.de/europe/united-kingdom/england/cumbria.html
Get topography data from: https://opentopography.org/
N.B Ensure the topography data fully covers the OSM data exported.

Build the project using CMake:
```bash
cmake .
cmake --build .
```

Run OSMParser to generate nodes.csv and edges.csv files contaning routing information.
```bash
./OSMParser <OSMData.pbf> <topography.tif>
```

Run Pathfinding with -c flag to specify a config file.
```bash
./PrettyPath -c<config_file>
```

plot_path.py can be used to visulise the path.

### GUI
The project contains a webapp GUI built using React.
To begin the backend server and frontend GUI run:
```bash
cd gui/
npm start
```


### Todos

 - Add setting to route back to start
 - Rename tarns to POI in cpp

License
----

MIT
