import React, { useEffect, useState } from "react";
import { MapContext } from "./MapContext";
import ControlPanel, {
  Checkbox,
  Custom,
  Interval,
  Button,
} from "react-control-panel";

function TarnList() {
  const { tarnConstraints, setTarnConstraints, bounds, unorderedTarns } =
    React.useContext(MapContext);
  const [tarnsInBounds, setTarnsInBounds] = useState(unorderedTarns);

  useEffect(() => {
    const updateTarnsInBounds = () => {
      const newTarnsInBounds = unorderedTarns.filter((tarn) =>
        bounds.contains(tarn.position)
      );
      setTarnsInBounds(newTarnsInBounds);
    };

    updateTarnsInBounds();
  }, [bounds, unorderedTarns]);

  const handleAddToBlacklist = (tarn) => {
    setTarnConstraints((prev) => ({
      ...prev,
      blacklist: [...prev.blacklist, tarn.name],
    }));
  };

  const handleRemoveFromBlacklist = (tarn) => {
    setTarnConstraints((prev) => ({
      ...prev,
      blacklist: prev.blacklist.filter((t) => t !== tarn.name),
    }));
  };

  return (
    <ControlPanel
      theme="dark"
      title="Tarns"
      className="bg-opacity-25 min-w-max"
    >
      <Custom
        Comp={({ theme }) => (
          <ul className="grid grid-cols-3 overflow-auto max-h-96">
            {tarnsInBounds.map((tarn) => (
              <li
                key={tarn.name}
                onClick={() =>
                  tarnConstraints.blacklist.includes(tarn.name)
                    ? handleRemoveFromBlacklist(tarn)
                    : handleAddToBlacklist(tarn)
                }
                style={{
                  color: tarnConstraints.blacklist.includes(tarn.name)
                    ? "red"
                    : theme.text1,
                }}
                className="mr-3"
              >
                {`${tarn.name} - ${tarn.elevation} m - ${tarn.area} m²`}
              </li>
            ))}
          </ul>
        )}
      />
      <Button
        label="Reset Blacklist"
        action={() =>
          setTarnConstraints((prev) => ({ ...prev, blacklist: [] }))
        }
      />
    </ControlPanel>
  );
}

function OrderedTarnList() {
  const { tarns, setTarns, tarnConstraints } = React.useContext(MapContext);

  const removeTarn = (tarn) => {
    setTarns((prev) => prev.filter((t) => t.name !== tarn.name));
  };

  const resetOrder = () => {
    setTarns([]);
  };

  const handleDragStart = (e, index) => {
    e.dataTransfer.setData("draggedItemIndex", index);
  };

  const handleDrop = (e, index) => {
    const draggedItemIndex = e.dataTransfer.getData("draggedItemIndex");
    const items = Array.from(tarns);
    const [reorderedItem] = items.splice(draggedItemIndex, 1);
    items.splice(index, 0, reorderedItem);

    setTarns(items);
  };

  return (
    <ControlPanel
      theme="dark"
      title="Ordered Tarns"
      className="bg-opacity-25 min-w-max"
    >
      <Custom
        Comp={({ theme }) => (
          <ul className="overflow-auto max-h-96">
            {tarns.map((tarn, index) => {
              return (
                <li
                  key={tarn.name}
                  draggable
                  onDragStart={(e) => handleDragStart(e, index)}
                  onDrop={(e) => handleDrop(e, index)}
                  onDragOver={(e) => e.preventDefault()}
                  onClick={() => removeTarn(tarn)}
                  style={{
                    color: tarnConstraints.blacklist.includes(tarn.name)
                      ? "red"
                      : theme.text1,
                  }}
                >
                  {`[${index}] ${tarn.name} - ${tarn.elevation} m - ${tarn.area} m²`}
                </li>
              );
            })}
            <li
              key="placeholder"
              draggable
              onDrop={(e) => handleDrop(e, tarns.length)}
              onDragOver={(e) => e.preventDefault()}
              style={{ minHeight: 10 }}
            ></li>
          </ul>
        )}
      />
      <Button label="Reset Order" action={resetOrder} />
    </ControlPanel>
  );
}

function TarnConstraints() {
  const { tarnConstraints, setTarnConstraints } = React.useContext(MapContext);

  return (
    <ControlPanel
      theme="dark"
      title="Tarn Constraints"
      initialState={tarnConstraints}
      onChange={(key, val) =>
        setTarnConstraints((prev) => ({ ...prev, [key]: val }))
      }
      width={500}
      // style={{ marginRight: 30 }}
      className="bg-opacity-25"
      onClick={(e) => console.log(e)}
    >
      <Interval label="elevation" min={0} max={1000} />
      <Interval label="area" min={100} max={100000} scale={"log"} />
      <Checkbox label="useOrderedTarns" />
      {/* <Custom Comp={({ theme }) => <TarnList theme={theme} />} /> */}
    </ControlPanel>
  );
}

export default TarnConstraints;
export { TarnList, OrderedTarnList };
