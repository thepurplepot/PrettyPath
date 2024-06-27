import React, { useEffect, useContext, useState } from "react";
import { Marker, Popup } from "react-leaflet";
import { MapContext } from "./MapContext";
import Papa from "papaparse";

function TarnLayer() {
  const {
    tarnConstraints,
    setUnorderedTarns,
    unorderedTarns,
    tarns,
    setTarns,
    fileNames,
  } = useContext(MapContext);
  const [data, setData] = useState([]);

  const orderTarn = (tarn) => {
    if (tarns.find((orderedTarn) => orderedTarn.name === tarn.name)) {
      return;
    }
    setTarns((prev) => [...prev, tarn]);
  };

  useEffect(() => {
    fetch(
      `http://localhost:3001/pois?file=${encodeURIComponent(
        fileNames.map_tarns
      )}`
    )
      .then((response) => response.text())
      .then((csv) => {
        const { data } = Papa.parse(csv, { header: true, dynamicTyping: true });
        setData(data);
      })
      .catch((error) => console.error("Error:", error));
  }, [fileNames.map_tarns, setData]);

  useEffect(() => {
    const filteredData = data
      .filter((row) => {
        var valid =
          row.lat &&
          row.lon &&
          row.name &&
          row.elevation &&
          (!tarnConstraints.elevation[0] ||
            row.elevation >= tarnConstraints.elevation[0]) &&
          (!tarnConstraints.elevation[1] ||
            row.elevation <= tarnConstraints.elevation[1]);
        if (row.area) {
          valid =
            valid &&
            (!tarnConstraints.area[0] || row.area >= tarnConstraints.area[0]) &&
            (!tarnConstraints.area[1] || row.area <= tarnConstraints.area[1]);
        }
        return valid;
      })
      .map((row) => ({
        position: [row.lat, row.lon],
        name: row.name,
        area: row.area,
        elevation: row.elevation,
      }));
    setUnorderedTarns(filteredData);
  }, [data, tarnConstraints, setUnorderedTarns]);

  return unorderedTarns.map((tarn, index) => (
    <Marker
      key={index}
      position={tarn.position}
      opacity={0.5}
      icon={window.L.AwesomeMarkers.icon({
        icon: "flag",
        prefix: "fa",
        markerColor: tarnConstraints.blacklist.includes(tarn.name)
          ? "black"
          : "orange",
      })}
      eventHandlers={{
        click: () => {
          if (tarnConstraints.useOrderedTarns) {
            orderTarn(tarn);
          }
        },
      }}
    >
      <Popup>
        <div>
          <h3 className="text-base font-bold">{tarn.name}</h3>
          <p className="text-sm">
            {tarn.area && `Area: ${tarn.area} m², `}
            Elevation: {tarn.elevation} m
          </p>
        </div>
      </Popup>
    </Marker>
  ));
}

export default TarnLayer;
