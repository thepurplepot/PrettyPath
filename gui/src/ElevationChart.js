import React, { useState, useEffect } from "react";
import { Marker } from "react-leaflet";
import {
  LineChart,
  Line,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
} from "recharts";

const ElevationChart = ({ data, setLocationPosition, setIsMarkerVisible }) => {
  const [closestPoint, setClosestPoint] = useState(null);
  const [isVisible, setIsVisible] = useState(true);
  const [totalElevationGain, setTotalElevationGain] = useState(0);

  const uniqueSegmentIds = [...new Set(data.map((item) => item.segment_id))];

  const lines = uniqueSegmentIds.map((segmentId, index) => {
    const colour = data.find((item) => item.segment_id === segmentId).colour;
    return (
      <Line
        key={index}
        type="monotone"
        dataKey="elevation"
        stroke={colour}
        data={data.filter((item) => item.segment_id === segmentId)}
        dot={false}
      />
    );
  });

  const handleMouseMove = (e) => {
    if (e.isTooltipActive) {
      setClosestPoint(e.activePayload[0].payload);
      setIsMarkerVisible(true);
    }
  };

  const handleMouseLeave = () => {
    setIsMarkerVisible(false);
  };

  const handleClick = () => {
    if (closestPoint) {
      navigateToMapPoint(closestPoint);
    }
  };

  const navigateToMapPoint = (point) => {
    setLocationPosition([point.lat, point.lon]);
  };

  const toggleChartVisibility = () => {
    setIsVisible(!isVisible);
  };

  useEffect(() => {
    const getTotalEleGain = () => {
      let totalEleGain = 0;
      for (let i = 0; i < data.length - 1; i++) {
        if (data[i + 1].elevation > data[i].elevation) {
          totalEleGain += data[i + 1].elevation - data[i].elevation;
        }
      }
      return totalEleGain;
    };
    setTotalElevationGain(getTotalEleGain());
  }, [data]);

  return (
    <div className="absolute bottom-5 left-3 z-[100] w-3/4">
      <button
        onClick={toggleChartVisibility}
        className="bg-blue-500 hover:bg-blue-700 text-white font-bold py-2 px-4 rounded"
      >
        {isVisible ? "Hide" : "Show"} Elevation Chart
      </button>
      {isVisible && (
        <div className="bg-white">
          <div>Total Elevation Gain: {totalElevationGain} meters</div>
          <ResponsiveContainer width="100%" height={300}>
            <LineChart
              data={data}
              margin={{
                top: 5,
                right: 30,
                left: 20,
                bottom: 20,
              }}
              onMouseMove={handleMouseMove}
              onMouseLeave={handleMouseLeave}
              onClick={handleClick}
            >
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis
                type="number"
                dataKey="distance"
                label={{
                  value: "Distance (km)",
                  position: "insideBottomRight",
                  offset: -10,
                }}
                tickFormatter={(value) => value.toFixed(1)}
                // interval={0.5}
              />
              <YAxis
                type="number"
                label={{
                  value: "Elevation (m)",
                  angle: -90,
                  position: "insideLeft",
                }}
              />
              <Tooltip
                content={({ payload, label }) => {
                  if (payload && payload.length) {
                    const firstLineData = payload[0];
                    return (
                      <div className="custom-tooltip">
                        <p>{`Distance: ${label.toFixed(2)} km`}</p>
                        <p>{`Elevation: ${firstLineData.value} m`}</p>
                      </div>
                    );
                  }

                  return null;
                }}
              />
              {lines}
            </LineChart>
          </ResponsiveContainer>
        </div>
      )}
    </div>
  );
};

function LocationMarker({ position, isVisible }) {
  return isVisible ? (
    <Marker
      position={position}
      icon={window.L.AwesomeMarkers.icon({
        icon: "flag",
        prefix: "fa",
        markerColor: "white",
      })}
    />
  ) : null;
}

export default ElevationChart;
export { LocationMarker };
