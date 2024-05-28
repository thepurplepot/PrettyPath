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
  const [open, setOpen] = React.useState(true);

  return (
    <div className="absolute top-5 right-3 z-50">
      {/* <Dropdown>
        <Dropdown.Toggle variant="success" id="dropdown-basic">
          Settings
        </Dropdown.Toggle>
        <Dropdown.Menu className="bg-transparent border-0"> */}
      <div className={open ? "flex" : "flex justify-end"}>
        {open && (
          <div className="flex flex-col">
            <FileNamePicker />
            <PathCostSliders />
          </div>
        )}
        <div className="flex flex-col">
          <Actions
            open={open}
            setOpen={setOpen}
            reloadGPX={reloadGPX}
            toggleGPX={toggleGPX}
            gpxHidden={gpxHidden}
          />
          {open && (
            <div>
              {!tarnConstraints.useOrderedTarns && (
                <div className="flex flex-col">
                  <PathConstraints />
                </div>
              )}
              <TarnConstraints />
            </div>
          )}
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
