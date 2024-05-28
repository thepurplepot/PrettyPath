import React from "react";
import { Dropdown } from "react-bootstrap";
import { MapContext } from "./MapContext";
import PathCostSliders from "./PathCostSilders";
import TarnConstraints, { TarnList, OrderedTarnList } from "./TarnConstraints";
import PathConstraints from "./PathConstraints";
import FileNamePicker from "./FileNamePicker";
import Actions from "./Actions";

const Settings = ({ reloadGPX, toggleGPX, gpxHidden }) => {
  const { tarnConstraints } = React.useContext(MapContext);

  return (
    <div className="absolute top-5 right-3 z-50">
      {/* <Dropdown>
        <Dropdown.Toggle variant="success" id="dropdown-basic">
          Settings
        </Dropdown.Toggle>
        <Dropdown.Menu className="bg-transparent border-0"> */}
          <div className="grid grid-cols-2">
            <div className="flex flex-col">
              <Actions
                reloadGPX={reloadGPX}
                toggleGPX={toggleGPX}
                gpxHidden={gpxHidden}
              />
            </div>
            <div className="flex flex-col">
              <PathCostSliders />
            </div>
            <div className="flex flex-col">
              <TarnConstraints />
            </div>
            {!tarnConstraints.useOrderedTarns && (
              <div className="flex flex-col">
                <PathConstraints />
              </div>
            )}
            <div className="flex flex-col">
              <FileNamePicker />
            </div>
          </div>
          <div className="flex flex-row py-2">
            <TarnList />
            {tarnConstraints.useOrderedTarns && <OrderedTarnList />}
          </div>
        {/* </Dropdown.Menu>
      </Dropdown> */}
    </div>
  );
};

export default Settings;
