import React from "react";
import { MapContext } from "./MapContext";

function PathCostSliders() {
  const { pathWeights, setPathWeights } = React.useContext(MapContext);

  return (
    <div>
      <label>
        Length Weight: {pathWeights.lengthWeight}
        <input
          type="range"
          min="0"
          max="5"
          step="0.1"
          value={pathWeights.lengthWeight}
          onChange={(e) =>
            setPathWeights((prev) => ({
              ...prev,
              lengthWeight: parseFloat(e.target.value),
            }))
          }
        />
      </label>
      <label>
        Elevation Weight: {pathWeights.elevationWeight}
        <input
          type="range"
          min="0"
          max="1"
          step="0.1"
          value={pathWeights.elevationWeight}
          onChange={(e) =>
            setPathWeights((prev) => ({
              ...prev,
              elevationWeight: parseFloat(e.target.value),
            }))
          }
        />
      </label>
      <label>
        Difficulty Weight: {pathWeights.difficultyWeight}
        <input
          type="range"
          min="0"
          max="1"
          step="0.1"
          value={pathWeights.difficultyWeight}
          onChange={(e) =>
            setPathWeights((prev) => ({
              ...prev,
              difficultyWeight: parseFloat(e.target.value),
            }))
          }
        />
      </label>
      <label>
        Cars Weight: {pathWeights.carsWeight}
        <input
          type="range"
          min="0"
          max="10"
          step="0.1"
          value={pathWeights.carsWeight}
          onChange={(e) =>
            setPathWeights((prev) => ({
              ...prev,
              carsWeight: parseFloat(e.target.value),
            }))
          }
        />
      </label>
    </div>
  );
}

export default PathCostSliders;
