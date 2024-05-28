import { useEffect } from "react";
import { Marker, Popup } from "react-leaflet";
import Papa from "papaparse";

function TarnLayer({ tarns, setTarns, orderedTarns, setOrderedTarns, constraints }) {

  const orderTarn = (tarn) => {
    if(orderedTarns.find((orderedTarn) => orderedTarn.name === tarn.name)) {
      return;
    }
    setOrderedTarns((prev) => [...prev, tarn]);
  };

  const resetOrder = () => {
    setOrderedTarns([]);
  };

  useEffect(() => {
    fetch("http://localhost:3001/tarns")
      .then((response) => response.text())
      .then((csv) => {
        const { data } = Papa.parse(csv, { header: true, dynamicTyping: true });
        setTarns(
          data
            .filter(
              (row) =>
                row.lat &&
                row.lon &&
                row.name &&
                row.area &&
                row.elevation &&
                (!constraints.area[0] || row.area >= constraints.area[0]) &&
                (!constraints.area[1] || row.area <= constraints.area[1]) &&
                (!constraints.elevation[0] ||
                  row.elevation >= constraints.elevation[0]) &&
                (!constraints.elevation[1] ||
                  row.elevation <= constraints.elevation[1]) //&&
                // (!constraints.blacklist ||
                //   !constraints.blacklist.includes(row.name))
            )
            .map((row) => ({
              position: [row.lat, row.lon],
              name: row.name,
              area: row.area,
              elevation: row.elevation,
            }))
        );
      })
      .catch((error) => console.error("Error:", error));
  }, [constraints, setTarns]);

  return tarns.map((tarn, index) => (
    <Marker
      key={index}
      position={tarn.position}
      opacity={0.5}
      icon={window.L.AwesomeMarkers.icon({
        icon: "flag",
        prefix: "fa",
        markerColor: constraints.blacklist.includes(tarn.name) ? "black" : "orange",
      })}
      eventHandlers={{
        click: () => {
          orderTarn(tarn);
        },
        dblclick: () => {
          resetOrder();
        }
        
      }}
    >
      <Popup>
        <div>
          <h2>{tarn.name}</h2>
          <p>
            Index:{" "}
            {
              orderedTarns.findIndex((orderedTarn) => orderedTarn.name === tarn.name)
            }
          </p>
          <p>Area: {tarn.area} m</p>
          <p>Elevation: {tarn.elevation} m²</p>
        </div>
      </Popup>
    </Marker>
  ));
}

export default TarnLayer;
