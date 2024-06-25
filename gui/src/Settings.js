import React, { useEffect } from "react";
import { MapContext } from "./MapContext";
import PathCostSliders from "./PathCostSilders";
import TarnConstraints, { TarnList, OrderedTarnList } from "./TarnConstraints";
import PathConstraints from "./PathConstraints";
import FileNamePicker from "./FileNamePicker";
import Actions from "./Actions";

const Settings = ({ reloadGPX, toggleGPX, gpxHidden }) => {
  const { tarnConstraints } = React.useContext(MapContext);
  const [open, setOpen] = React.useState(true);
  const [hidden, setHidden] = React.useState(false);

  useEffect(() => {
    const toggleHidden = (event) => {
      if (event.key === "s") {
        setHidden((prev) => !prev);
      }
    };

    window.addEventListener("keydown", toggleHidden);

    return () => {
      window.removeEventListener("keydown", toggleHidden);
    };
  }, []);

  return (
    <div className="absolute top-5 right-3 z-50">
      {hidden && (
        <div className="flex justify-end">
          <div className="hint-dialog">Press "S" to show the settings menu</div>
        </div>
      )}
      {!hidden && (
        <div>
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
        </div>
      )}
    </div>
  );
};

export default Settings;
