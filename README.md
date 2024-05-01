# Pretty Paths

# Table of contents
 - [Dependencies](#dependencies)
 - [Setup](#setup)
 - [Todos](#todos)

# Dependencies
CMake >= 3.11
C++ 17 compatible compiler
libosmium 
protozero
ZLIB
GDAL
Python: staticmaps

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

Run Pathfinding with -s, -g flags for start and goal latitudes/longitudes to find the best path, outputed to path.csv.
```bash
./PrettyPath -s<latitude,longitude> -g<latitude,longitude>
```

plot_path.py can be used to visulise the path.

### Todos

 - Refactor

License
----

GPL-3.0
