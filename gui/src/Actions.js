import React from "react";
import { MapContext } from "./MapContext";
import ControlPanel, { Button, Custom } from "react-control-panel";

function Actions({ toggleGPX, reloadGPX, gpxHidden, open, setOpen }) {
  const {
    bounds,
    pathWeights,
    startLocation,
    tarnConstraints,
    tarns,
    pathConstraints,
    fileNames,
  } = React.useContext(MapContext);
  const [progress, setProgress] = React.useState(0);

  const saveConfig = async () => {
    const config = {
      filenames: {
        map_nodes: fileNames.map_nodes,
        map_edges: fileNames.map_edges,
        map_tarns: tarnConstraints.useOrderedTarns
          ? "data/tarns.json"
          : fileNames.map_tarns,
        output_dir: fileNames.output_dir,
        gpx: fileNames.gpx,
      },
      path_cost: {
        length_weight: pathWeights.lengthWeight,
        elevation_weight: pathWeights.elevationWeight,
        difficulty_weight: pathWeights.difficultyWeight,
        cars_weight: pathWeights.carsWeight,
      },
      map_constraints: {
        min_latitude: bounds.getSouth(),
        max_latitude: bounds.getNorth(),
        min_longitude: bounds.getWest(),
        max_longitude: bounds.getEast(),
      },
      path_constraints: {
        max_length: pathConstraints.length[1],
        min_length: pathConstraints.length[0],
        max_elevation: pathConstraints.maxElevation,
        max_difficulty: pathConstraints.maxDifficulty,
        max_cars: pathConstraints.maxCars,
        start_location: {
          latitude: startLocation.lat,
          longitude: startLocation.lng,
        },
      },
      tarn_constraints: {
        ...(tarnConstraints.useOrderedTarns
          ? { use_ordered_tarns: true }
          : {
              min_elevation: tarnConstraints.elevation[0],
              max_elevation: tarnConstraints.elevation[1],
              min_area: tarnConstraints.area[0],
              max_area: tarnConstraints.area[1],
              blacklist: tarnConstraints.blacklist,
            }),
      },
    };

    console.log(config);

    await fetch("http://localhost:3001/config", {
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

  const sendTarns = () => {
    fetch("http://localhost:3001/tarns", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(
        tarns.filter((tarn) => bounds.contains(tarn.position))
      ),
    })
      .then((response) => {
        if (!response.ok) {
          throw new Error("Network response was not ok");
        }
      })
      .catch((error) => console.error("Error:", error));
  };

  const save = () => {
    saveConfig();
    if (tarnConstraints.useOrderedTarns) {
      sendTarns();
    }
  };

  const run = async () => {
    setProgress(1);
    save();
    try {
      const response = await fetch("http://localhost:3001/run", {
        method: "POST",
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const reader = response.body.getReader();
      const stream = new ReadableStream({
        start(controller) {
          function push() {
            reader.read().then(({ done, value }) => {
              if (done) {
                controller.close();
                return;
              }
              const text = new TextDecoder("utf-8").decode(value);
              const match = text.match(/Progress: \[.*\] (\d+) %/m);
              if (match) {
                setProgress(parseInt(match[1], 10));
              }
              controller.enqueue(value);
              push();
            });
          }
          push();
        },
      });

      const result = await new Response(stream, {
        headers: { "Content-Type": "text/plain" },
      }).text();
      console.log(result);
      reloadGPX();
    } catch (error) {
      console.error("There was a problem with the fetch operation:", error);
    }
  };

  return (
    <ControlPanel
      theme="dark"
      title="Actions"
      width={500}
      // style={{ marginRight: 30 }}
      className="bg-opacity-25"
    >
      <Button
        label={open ? "Close Settings" : "Open Settings"}
        action={() => setOpen(!open)}
      />
      <Button label={gpxHidden ? "Show GPX" : "Hide GPX"} action={toggleGPX} />
      {/*<Button label="Save" action={save} />*/}
      <Button label="Run" action={run} />
      <Custom
        Comp={() =>
          progress > 0 ? <progress value={progress} max="100" /> : null
        }
      />
    </ControlPanel>
  );
}

export default Actions;
