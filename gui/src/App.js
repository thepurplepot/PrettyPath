import "./App.css";

import GPXMap from "./GPXMap";

function App() {
  return (
    <div className="App">
      <GPXMap gpxUrl="http://localhost:3001/full_path.gpx" />
    </div>
  );
}

export default App;
