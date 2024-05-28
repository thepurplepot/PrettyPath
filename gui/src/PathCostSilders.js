import React from "react";
import { MapContext } from "./MapContext";
import ControlPanel, { Range } from "react-control-panel";

function PathCostSliders() {
  const { pathWeights, setPathWeights } = React.useContext(MapContext);

  return (
    <ControlPanel
      theme="dark"
      title="Path Cost Weights"
      initialState={pathWeights}
      onChange={(key, val) =>
        setPathWeights((prev) => ({ ...prev, [key]: val }))
      }
      width={500}
      style={{ marginRight: 30 }}
      className="bg-opacity-25"
    >
      <Range label="lengthWeight" min={0} max={5} />
      <Range label="elevationWeight" min={0} max={1} />
      <Range label="difficultyWeight" min={0} max={1} />
      <Range label="carsWeight" min={0} max={10} />
    </ControlPanel>
  );
}

export default PathCostSliders;
