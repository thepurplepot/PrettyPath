import React, {
  useEffect,
  useState,
  useRef,
  useMemo,
  useCallback,
} from "react";
import {
  MapContainer,
  TileLayer,
  useMap,
  Marker,
  Popup,
  useMapEvent,
} from "react-leaflet";
import "leaflet/dist/leaflet.css";
import "leaflet-gpx";
import "leaflet.awesome-markers/dist/leaflet.awesome-markers.css";
import "leaflet.awesome-markers";
import { MapContext } from "./MapContext";
import TarnLayer from "./TarnLayer";
import "leaflet.heightgraph";
import "leaflet.heightgraph/dist/L.Control.Heightgraph.min.css";
import { parseGPXtoGeoJSON } from "./parseGPX";
import Settings from "./Settings";

function MapBoundsUpdater({ setBounds }) {
  const map = useMapEvent("moveend", () => {
    setBounds(map.getBounds());
  });

  return null;
}

function DraggableMarker({ position, setPosition, color = "green" }) {
  const [draggable, setDraggable] = useState(false);
  const markerRef = useRef(null);
  const eventHandlers = useMemo(
    () => ({
      dragend() {
        const marker = markerRef.current;
        if (marker != null) {
          setPosition(marker.getLatLng());
        }
      },
    }),
    [setPosition]
  );
  const toggleDraggable = useCallback(() => {
    setDraggable((d) => !d);
  }, []);

  return (
    <Marker
      draggable={draggable}
      eventHandlers={eventHandlers}
      position={position}
      ref={markerRef}
      icon={window.L.AwesomeMarkers.icon({
        icon: "flag",
        prefix: "fa",
        markerColor: color,
      })}
    >
      <Popup minWidth={90}>
        <span onClick={toggleDraggable}>
          {draggable
            ? "Marker is draggable"
            : "Click here to make marker draggable"}
        </span>
      </Popup>
    </Marker>
  );
}

function GPXLayer({
  gpx_url,
  gpxLoaded,
  gpxHidden,
  setGpxLoaded,
  setStartLocation,
}) {
  const map = useMap();
  // const hg = window.L.control.heightgraph({
  //   width: 800,
  //   height: 280,
  //   margins: {
  //     top: 10,
  //     right: 30,
  //     bottom: 55,
  //     left: 50
  //   },
  //   position: "bottomright",
  //   expand: false,
  // });
  // hg.addTo(map);
  //   const geojson = {type: "FeatureCollection",
  //   features: [{
  //       type: "Feature",
  //       geometry: {
  //           type: "LineString",
  //           coordinates: [
  //               [8.6865264, 49.3859188, 114.5],
  //               [8.6864108, 49.3868472, 114.3],
  //               [8.6860538, 49.3903808, 114.8]
  //           ]
  //       },
  //       properties: {
  //           attributeType: "3"
  //       }
  //   }, {
  //       type: "Feature",
  //       geometry: {
  //           type: "LineString",
  //           coordinates: [
  //               [8.6860538, 49.3903808, 114.8],
  //               [8.6857921, 49.3936309, 114.4],
  //               [8.6860124, 49.3936431, 114.3]
  //           ]
  //       },
  //       properties: {
  //           attributeType: "0"
  //       }
  //   }],
  //   properties: {
  //       Creator: "OpenRouteService.org",
  //       records: 2,
  //       summary: "Steepness"
  //   }
  // };
  //   hg.addData(geojson);
  //   window.L.geoJson(geojson).addTo(map);

  useEffect(() => {
    const colors = ["red", "orange", "brown", "green", "blue", "purple"];

    if (gpxHidden) {
      map.eachLayer((layer) => {
        if (layer instanceof window.L.GPX) {
          map.removeLayer(layer);
        }
      });
      return;
    }

    if (gpxLoaded) {
      return;
    } else {
      // Remove existing GPX layers
      map.eachLayer((layer) => {
        if (layer instanceof window.L.GPX) {
          map.removeLayer(layer);
        }
      });
    }

    new window.L.GPX(gpx_url, {
      async: true,
      marker_options: {
        startIconUrl: null,
        endIconUrl: null,
        shadowUrl: null,
        wptIcons: {
          "": new window.L.AwesomeMarkers.icon({
            icon: "flag",
            prefix: "fa",
            markerColor: "red",
            iconColor: "white",
          }),
        },
      },
      polyline_options: {
        color: colors[0],
        opacity: 0.75,
        weight: 3,
        lineCap: "round",
        lineJoin: "round",
      },
      parseElements: ["track", "waypoint"],
    })
      .on("loaded", function (e) {
        setGpxLoaded(true);
        console.log("GPX file loaded:", e);
        map.fitBounds(e.target.getBounds());
        let colorIndex = 0;
        e.target.getLayers().forEach((layer) => {
          if (
            layer instanceof window.L.LayerGroup ||
            layer instanceof window.L.FeatureGroup
          ) {
            Object.values(layer._layers).forEach((sublayer) => {
              if (sublayer instanceof window.L.Polyline) {
                const color = colors[colorIndex];
                colorIndex = (colorIndex + 1) % colors.length;
                sublayer.setStyle({ color: color });

                sublayer.on("click", (e) => {
                  const latlngs = sublayer.getLatLngs();
                  let length = 0;
                  for (let i = 0; i < latlngs.length - 1; i++) {
                    length += latlngs[i].distanceTo(latlngs[i + 1]);
                  }
                  window.L.popup()
                    .setLatLng(e.latlng)
                    .setContent(
                      `<p style="color: ${color}; font-size: 16px;">Length: ${length} m</p>`
                    )
                    .openOn(map);
                });
              }
              if (sublayer instanceof window.L.Marker) {
                if (sublayer.options.title === "Start") {
                  setStartLocation(sublayer._latlng);
                  map.removeLayer(sublayer);
                }
              }
            });
          } else {
            if (layer instanceof window.L.Polyline) {
              layer.setStyle({ color: colors[colorIndex] });
              colorIndex = (colorIndex + 1) % colors.length;
            }
          }
        });
        // console.log("GPX to GeoJSON")
        // console.log(e.target.toGeoJSON());
        // console.log("Parsed GPX to GeoJSON")
        // console.log(parseGPXtoGeoJSON(e.target.getLayers()));
        // hg.addData(parseGPXtoGeoJSON(e.target.getLayers()));
      })
      .on("error", function (e) {
        console.error("Error loading GPX file:", e);
      })
      .addTo(map);
  }, [gpx_url, gpxLoaded, map, setGpxLoaded, setStartLocation, gpxHidden]);

  return null;
}

export default function GPXMap({ gpxUrl }) {
  const [gpxLoaded, setGpxLoaded] = useState(false);
  const [gpxHidden, setGpxHidden] = useState(false);
  const [startLocation, setStartLocation] = useState([0, 0]);
  const [bounds, setBounds] = useState(null);
  const [pathWeights, setPathWeights] = useState({
    lengthWeight: 1,
    elevationWeight: 0.2,
    difficultyWeight: 0.1,
    carsWeight: 10,
  });
  const [tarnConstraints, setTarnConstraints] = useState({
    elevation: [0, 1000],
    area: [300, 10000],
    blacklist: [],
    useOrderedTarns: false,
  });
  const [pathConstraints, setPathConstraints] = useState({
    length: [0, 10000],
    maxElevation: 1000,
    maxDifficulty: 3,
    maxCars: 1,
  });
  const [fileNames, setFileNames] = useState({
    map_nodes: "data/nodes.csv",
    map_edges: "data/edges.csv",
    map_tarns: "data/tarns.csv",
    output_dir: "data/path/",
    gpx: "full_path.gpx",
  });
  const [tarns, setTarns] = useState([]);
  const [unorderedTarns, setUnorderedTarns] = useState([]);

  const reloadGPX = () => {
    setGpxLoaded(false);
  };

  const toggleGPX = () => {
    setGpxHidden((prev) => !prev);
    if (gpxHidden) {
      setGpxLoaded(false);
    }
  };

  return (
    <div>
      <MapContext.Provider
        value={{
          bounds,
          pathWeights,
          setPathWeights,
          startLocation,
          tarnConstraints,
          setTarnConstraints,
          pathConstraints,
          setPathConstraints,
          setTarns,
          tarns,
          unorderedTarns,
          fileNames,
          setFileNames,
        }}
      >
        <Settings
          reloadGPX={reloadGPX}
          toggleGPX={toggleGPX}
          gpxHidden={gpxHidden}
        />
        <MapContainer
          style={{ height: "100vh", width: "100%", zIndex: 10 }}
          center={[0, 0]}
          zoom={13}
          scrollWheelZoom={false}
        >
          <TileLayer
            url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
            attribution='&copy; <a href="http://osm.org/copyright">OpenStreetMap</a> contributors'
          />
          <GPXLayer
            gpx_url={gpxUrl}
            gpxLoaded={gpxLoaded}
            gpxHidden={gpxHidden}
            setGpxLoaded={setGpxLoaded}
            setStartLocation={setStartLocation}
          />
          <TarnLayer
            tarns={unorderedTarns}
            setTarns={setUnorderedTarns}
            orderedTarns={tarns}
            setOrderedTarns={setTarns}
            constraints={tarnConstraints}
          />
          <DraggableMarker
            position={startLocation}
            setPosition={setStartLocation}
          />
          <MapBoundsUpdater setBounds={setBounds} />
        </MapContainer>
      </MapContext.Provider>
    </div>
  );
}
