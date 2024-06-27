import "./App.css";

import GPXMap from "./GPXMap";
import React from "react";

function App() {
  return (
    <div className="App">
      <GPXMap gpxUrl="http://localhost:3001/full_path.gpx" />
    </div>
  );
}

export default App;
