// input e.target.getLayers()
const parseGPXtoGeoJSON = (layers) => {
  const geojson = {
    type: "FeatureCollection",
    features: [],
    properties: {
        "Creator": "OpenRouteService.org",
        "records": 1,
        "summary": "Steepness"
    }
  };

  //TODO add points to GeoJSON
  layers.forEach((layer) => {
    if (
      layer instanceof window.L.LayerGroup ||
      layer instanceof window.L.FeatureGroup
    ) {
      //   Object.values(layer._layers).forEach((sublayer) => {
      const sublayer = layer._layers[Object.keys(layer._layers)[0]];
      const feature = {
        type: "Feature",
        properties: {
          attributeType: "3",
        },
        geometry: {
          type: "LineString",
          coordinates: [],
        },
      };
      if (sublayer instanceof window.L.Polyline) {
        sublayer.getLatLngs().forEach((latlng) => {
          const lat = parseFloat(latlng.lat);
          const lon = parseFloat(latlng.lng);
          const ele = parseFloat(latlng.meta.ele);
          feature.geometry.coordinates.push([lon, lat, ele]);
        });
        geojson.features.push(feature);
      }
      //   });
    } else {
      if (layer instanceof window.L.Polyline) {
        console.log("layer instanceof window.L.Polyline", layer);
      } else {
        console.log("Unknown layer", layer);
      }
    }
  });

  //   geojson.features.forEach((feature) => {
  //     console.log("feature", feature);
  //     feature.geometry.coordinates.forEach((coordinate) => {
  //       if (isNaN(coordinate[2])) {
  //         console.error('Invalid elevation data:', coordinate);
  //       }
  //     });
  //   });

  return geojson;
};

export { parseGPXtoGeoJSON };
