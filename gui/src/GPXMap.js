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
import PathCostSliders from "./PathCostSilders";
import SaveConfigButton from "./SaveConfigButton";
import { MapContext } from "./MapContext";
import RunButton from "./RunButton";

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

function GPXLayer({ gpx_url, gpxLoaded, setGpxLoaded, setStartLocation }) {
  const map = useMap();

  useEffect(() => {
    const colors = ["red", "blue", "green", "purple"];

    if (gpxLoaded) {
      console.log("GPX already loaded");
      return;
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
          Start: null,
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
                sublayer.setStyle({ color: colors[colorIndex] });
                colorIndex = (colorIndex + 1) % colors.length;
              }
              if (sublayer instanceof window.L.Marker) {
                if (sublayer.options.title === "Start") {
                  setStartLocation([
                    sublayer._latlng.lat,
                    sublayer._latlng.lng,
                  ]);
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
      })
      .on("error", function (e) {
        console.error("Error loading GPX file:", e);
      })
      .addTo(map);
  }, [gpx_url, gpxLoaded, map, setGpxLoaded, setStartLocation]);

  return null;
}

export default function GPXMap({ gpxUrl }) {
  const [gpxLoaded, setGpxLoaded] = useState(false);
  const [startLocation, setStartLocation] = useState([0, 0]);
  const [bounds, setBounds] = useState(null);
  const [pathWeights, setPathWeights] = useState({
    lengthWeight: 1,
    elevationWeight: 0.2,
    difficultyWeight: 0.1,
    carsWeight: 10,
  });

  const reloadGPX = () => {
    setGpxLoaded(false);
  };

  return (
    <div>
      <button onClick={reloadGPX}>Reload GPX</button>
      <MapContext.Provider
        value={{ bounds, pathWeights, setPathWeights, startLocation }}
      >
        <PathCostSliders />
        <SaveConfigButton />
        <RunButton />
        <MapContainer
          style={{ height: "100vh", width: "100%" }}
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
            setGpxLoaded={setGpxLoaded}
            setStartLocation={setStartLocation}
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
