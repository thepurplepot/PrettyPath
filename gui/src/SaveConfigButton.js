import React from "react";
import { MapContext } from "./MapContext";

function SaveConfigButton() {
  const { bounds, pathWeights, startLocation } = React.useContext(MapContext);

  const saveConfig = () => {
    const config = {
      path_cost: {
        length: pathWeights.lengthWeight,
        elevation: pathWeights.elevationWeight,
        difficulty: pathWeights.difficultyWeight,
        cars: pathWeights.carsWeight,
      },
      map_constraints: {
        min_lat: bounds.getSouth(),
        max_lat: bounds.getNorth(),
        min_lon: bounds.getWest(),
        max_lon: bounds.getEast(),
      },
      path_constraints: {
        start_location: {
          lat: startLocation.lat,
          lon: startLocation.lng,
        },
      },
    };

    console.log(config);

    fetch("http://localhost:3001/config", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(config),
    })
      .then((response) => response.json())
      .then((data) => console.log(data))
      .catch((error) => console.error("Error:", error));
  };

  return (
    <div>
      <button onClick={saveConfig}>Save</button>
    </div>
  );
}

export default SaveConfigButton;
