import matplotlib.pyplot as plt
import staticmaps
import os
import fnmatch

class Node:
    def __init__(self, id, latitude, longitude, length=float('inf'), elevation=0):
        self.id = id
        self.longitude = longitude
        self.latitude = latitude
        self.length = length
        self.elevation = elevation

def get_path(filename):
    path = []
    with open(filename, 'r') as file:
        next(file)  # Skip the first line
        for line in file:
            parts = line.split(',')
            id = int(parts[0])
            latitude = float(parts[1])
            longitude = float(parts[2])
            length = float(parts[3])
            elevation = float(parts[4])
            node = Node(id, latitude, longitude, length, elevation)
            path.append(node)
    return path

def calculate_ascent_descent(path):
    total_ascent = 0
    ascent_length = 0
    total_descent = 0
    descent_length = 0
    flat_length = 0

    for i in range(len(path) - 1):
        elevation_difference = path[i+1].elevation - path[i].elevation
        length = path[i].length
        if elevation_difference > 0:
            total_ascent += elevation_difference
            ascent_length += length
        elif elevation_difference < 0:
            total_descent += abs(elevation_difference)
            descent_length += length
        else:
            flat_length += length

    ascent_length /= 1000
    descent_length /= 1000
    flat_length /= 1000
    return total_ascent, ascent_length, total_descent, descent_length, flat_length

def extract_tarn_names(filename):
    tarns = filename[len("path_"):-len(".csv")]
    start_tarn, end_tarn = tarns.split("_to_")
    start_tarn = start_tarn.replace("_", " ")
    end_tarn = end_tarn.replace("_", " ")
    return start_tarn, end_tarn

def plot_paths(directory='data/path'):
    visited_tarns = {}
    context = staticmaps.Context()
    context.set_tile_provider(staticmaps.tile_provider_OSM)
    for filename in os.listdir(directory):
        if not fnmatch.fnmatch(filename, 'path_*.csv'):
            continue
        start_tarn, end_tarn = extract_tarn_names(filename)
        path = get_path(os.path.join(directory, filename))
        total_ascent, ascent_length, total_descent, descent_length, flat_length = calculate_ascent_descent(path)
        print("Path from", start_tarn, "to", end_tarn)
        print(f"Total ascent: {total_ascent:.2f} m in {ascent_length:.2f} km")
        print(f"Total descent: {total_descent:.2f} m in {descent_length:.2f} km")
        print(f"Flat: {flat_length:.2f} km")
        line = [staticmaps.create_latlng(node.latitude, node.longitude) for node in path]
        context.add_object(staticmaps.Line(line, width=1))
        if start_tarn not in visited_tarns:
            visited_tarns[start_tarn] = staticmaps.create_latlng(path[0].latitude, path[0].longitude)
        if end_tarn not in visited_tarns:
            visited_tarns[end_tarn] = staticmaps.create_latlng(path[-1].latitude, path[-1].longitude)
    for tarn, latlng in visited_tarns.items():
        context.add_object(staticmaps.Marker(latlng, color=staticmaps.GREEN, size=12))
    return context

context = plot_paths()

# # render non-anti-aliased png
# image = context.render_pillow(800, 500)
# image.save("path.png")

# render svg
svg_image = context.render_svg(800, 500)
with open("path.svg", "w", encoding="utf-8") as f:
    svg_image.write(f, pretty=True)