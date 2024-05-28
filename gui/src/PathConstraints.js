import React from "react";
import { MapContext } from "./MapContext";
import ControlPanel, { Interval, Range } from "react-control-panel";

function PathConstraints() {
  const { pathConstraints, setPathConstraints } = React.useContext(MapContext);

  return (
    <ControlPanel
      theme="dark"
      title="Path Constraints"
      initialState={pathConstraints}
      onChange={(newState) =>
        setPathConstraints((prev) => ({ ...prev, ...newState }))
      }
      width={500}
      // style={{ marginRight: 30 }}
      className="bg-opacity-25"
    >
      <Interval label="length" min={0} max={50000} />
      <Range label="maxElevation" min={0} max={5000} />
      <Range label="maxDifficulty" min={0} max={5} />
      <Range label="maxCars" min={0} max={5} />
    </ControlPanel>
  );
}

export default PathConstraints;
