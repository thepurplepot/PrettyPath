import staticmaps
from PIL import Image, ImageDraw, ImageFont
import os
import fnmatch

font_path = "/System/Library/Fonts/Helvetica.ttc"

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

def generate_tarn_marker(text, filename):
    image = Image.new("RGBA", (100, 30), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(font_path, 15)
    draw.ellipse((0, 10, 100, 30), fill=(100, 255, 100, 255))
    draw.polygon([(50, 0), (40, 20), (60, 20)], fill=(100, 255, 100, 255))  # Draw a triangle
    draw.text((10, 10), text, font=font, fill=(0, 0, 0, 255))
    image.save(filename, "PNG")

def plot_paths(directory='data/path'):
    visited_tarns = {}
    index = 0
    context = staticmaps.Context()
    context.set_tile_provider(staticmaps.tile_provider_OSM)
    for filename in os.listdir(directory):
        if not fnmatch.fnmatch(filename, 'path_*.csv'):
            continue

        start_tarn, end_tarn = extract_tarn_names(filename)
        path = get_path(os.path.join(directory, filename))
        if len(path) < 2:
            continue

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

        index += 1

    for tarn, latlng in visited_tarns.items():
        context.add_object(staticmaps.Marker(latlng, color=staticmaps.GREEN, size=12))
        image_filename = f"data/markers/{tarn}.png"
        if(f"{tarn}.png" not in os.listdir("data/markers")):
            print(f"Generating marker for {tarn}")
            generate_tarn_marker(tarn, image_filename)
        context.add_object(staticmaps.ImageMarker(latlng, image_filename, 12, 12))
    return context




if not os.path.exists("data/markers"):
    os.makedirs("data/markers")

context = plot_paths()

# # render non-anti-aliased png
# image = context.render_pillow(800, 500)
# image.save("path.png")

# render svg
svg_image = context.render_svg(800, 500)
with open("path.svg", "w", encoding="utf-8") as f:
    svg_image.write(f, pretty=True)