const haversine = (lat1, lon1, lat2, lon2) => {
    const R = 6371e3; // metres
    const φ1 = (lat1 * Math.PI) / 180; // φ, λ in radians
    const φ2 = (lat2 * Math.PI) / 180;
    const Δφ = ((lat2 - lat1) * Math.PI) / 180;
    const Δλ = ((lon2 - lon1) * Math.PI) / 180;

    const a =
        Math.sin(Δφ / 2) * Math.sin(Δφ / 2) +
        Math.cos(φ1) * Math.cos(φ2) * Math.sin(Δλ / 2) * Math.sin(Δλ / 2);
    const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));

    return R * c; // in metres
}


// input e.target.getLayers()
const parseElevationData = (layers) => {
    const elevationData = [];
    let totalDistance = 0;
  
    layers.forEach((layer) => {
      if (
        layer instanceof window.L.LayerGroup ||
        layer instanceof window.L.FeatureGroup
      ) {
        Object.values(layer._layers).forEach((sublayer) => {
        if (sublayer instanceof window.L.Polyline) {
          const latlngs = sublayer.getLatLngs();
          for (let i = 0; i < latlngs.length - 1; i++) {
            const lat1 = parseFloat(latlngs[i].lat);
            const lon1 = parseFloat(latlngs[i].lng);
            // const ele1 = parseFloat(latlngs[i].meta.ele);
            const lat2 = parseFloat(latlngs[i + 1].lat);
            const lon2 = parseFloat(latlngs[i + 1].lng);
            const ele2 = parseFloat(latlngs[i + 1].meta.ele);
            const distance = haversine(lat1, lon1, lat2, lon2);
            totalDistance += distance;
            elevationData.push({ distance: totalDistance, elevation: ele2 });
          }
        }
        });
      } else {
        if (layer instanceof window.L.Polyline) {
          console.log("layer instanceof window.L.Polyline", layer);
        } else {
          console.log("Unknown layer", layer);
        }
      }
    });
  
    return elevationData;
  };
  
  export { parseElevationData };
  