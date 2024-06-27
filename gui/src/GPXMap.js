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
import Settings from "./Settings";
import { parseElevationData } from "./parseElevationData";
import ElevationChart, { LocationMarker } from "./ElevationChart";

function MapBoundsUpdater({ setBounds }) {
  const map = useMapEvent("moveend", () => {
    setBounds(map.getBounds());
  });

  return null;
}

function DraggableMarker({
  position,
  setPosition,
  color = "green",
  name = "Start",
}) {
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
      <Popup>
        <div onClick={toggleDraggable}>
          <h3 className="text-base font-bold">{name}</h3>
          <p className="text-sm">
            {draggable ? "Click to anchor" : "Click to drag"}
          </p>
        </div>
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
  setEleData,
}) {
  const map = useMap();

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
                  length = (length / 1000).toFixed(2);
                  window.L.popup()
                    .setLatLng(e.latlng)
                    .setContent(
                      `<p style="color: ${color}; font-size: 16px;">Length: ${length} km</p>`
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
        const eleData = parseElevationData(e.target.getLayers());
        setEleData(eleData);
      })
      .on("error", function (e) {
        console.error("Error loading GPX file:", e);
      })
      .addTo(map);
  }, [
    gpx_url,
    gpxLoaded,
    map,
    setGpxLoaded,
    setStartLocation,
    gpxHidden,
    setEleData,
  ]);

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
    maxCars: 4,
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
  const [eleData, setEleData] = useState([]);
  const [locationMarkerPosition, setLocationMarkerPosition] = useState([0, 0]);
  const [isLocationMarkerVisible, setIsLocationMarkerVisible] = useState(false);

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
          setUnorderedTarns,
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
        <ElevationChart
          data={eleData}
          setLocationPosition={setLocationMarkerPosition}
          setIsMarkerVisible={setIsLocationMarkerVisible}
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
            setEleData={setEleData}
          />

          <TarnLayer />
          <DraggableMarker
            position={startLocation}
            setPosition={setStartLocation}
          />
          <LocationMarker
            position={locationMarkerPosition}
            isVisible={isLocationMarkerVisible}
          />
          <MapBoundsUpdater setBounds={setBounds} />
        </MapContainer>
      </MapContext.Provider>
    </div>
  );
}
