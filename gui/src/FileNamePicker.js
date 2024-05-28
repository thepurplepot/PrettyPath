import React from "react";
import { MapContext } from "./MapContext";
import ControlPanel, { Text } from "react-control-panel";

function FileNamePicker() {
  const { fileNames, setFileNames } = React.useContext(MapContext);

  return (
    <ControlPanel
      theme="dark"
      title="File Names"
      initialState={fileNames}
      onChange={(key, val) => setFileNames((prev) => ({ ...prev, [key]: val }))}
      width={500}
      // style={{ marginRight: 30 }}
      className="bg-opacity-25"
    >
      <Text label="map_nodes" />
      <Text label="map_edges" />
      <Text label="map_tarns" />
      <Text label="output_dir" />
      <Text label="gpx" />
    </ControlPanel>
  );
}

export default FileNamePicker;
