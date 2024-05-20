import staticmaps
from PIL import Image, ImageDraw, ImageFont
import os
import sys
import fnmatch

font_path = "/System/Library/Fonts/Helvetica.ttc"
regen_markers = True

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
    tarns = filename[:-len(".csv")]
    start_tarn, end_tarn = tarns.split("_to_")
    start_tarn = start_tarn.replace("_", " ")
    end_tarn = end_tarn.replace("_", " ")
    return start_tarn, end_tarn

def generate_tarn_marker(text, index, filename):
    side_pading = 10
    bottom_padding = 40
    arrow_height = 25
    arrow_width = 10
    arrow_color = (0, 255, 0, 255)
    font = ImageFont.truetype(font_path, 15)

    text_width, text_height = font.getbbox(text)[2:4]
    marker_width = text_width + side_pading*2
    marker_height = text_height + bottom_padding
    marker = Image.new("RGBA", (marker_width, marker_height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(marker)
    draw.ellipse((marker_width/2-arrow_width/2, marker_height-arrow_height-arrow_width/2, marker_width/2+arrow_width/2, marker_height-arrow_height+arrow_width/2), fill=arrow_color)
    draw.polygon([(marker_width/2, marker_height), (marker_width/2-arrow_width/2, marker_height-arrow_height), (marker_width/2+arrow_width/2, marker_height-arrow_height)], fill=arrow_color)
    draw.text((side_pading, 5), text, font=font, fill=(0, 0, 0, 255))
    draw.text((marker_width/2-5, bottom_padding-arrow_height+5), str(index), font=font, fill=(0, 0, 0, 255))
    marker.save(filename, "PNG")

def plot_paths(directory='data/path'):
    colours = [staticmaps.RED, staticmaps.GREEN, staticmaps.BLUE, staticmaps.PURPLE, staticmaps.YELLOW]
    visited_tarns = {}
    tarn_order = {}
    context = staticmaps.Context()
    context.set_tile_provider(staticmaps.tile_provider_OSM)
    for i, filename in enumerate(os.listdir(directory)):
        if not fnmatch.fnmatch(filename, '*.csv'):
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

        colour = colours[i % len(colours)]
        line = [staticmaps.create_latlng(node.latitude, node.longitude) for node in path]
        context.add_object(staticmaps.Line(line, width=1, color=colour))

        if start_tarn not in visited_tarns:
            visited_tarns[start_tarn] = staticmaps.create_latlng(path[0].latitude, path[0].longitude)
        if end_tarn not in visited_tarns:
            visited_tarns[end_tarn] = staticmaps.create_latlng(path[-1].latitude, path[-1].longitude)
        
        tarn_order[start_tarn] = end_tarn
    
    # Find tarn order
    first_tarn = None
    for tarn in visited_tarns.keys():
        if tarn not in tarn_order.values():
            first_tarn = tarn
            break
    tarns = [first_tarn]
    for _ in range(len(tarn_order)):
        tarns.append(tarn_order[tarns[-1]])

    for tarn, latlng in visited_tarns.items():
        image_filename = f"data/markers/{tarn}.png"
        if(f"{tarn}.png" not in os.listdir("data/markers") or regen_markers):
            print(f"Generating marker for {tarn}")
            tarn_index = tarns.index(tarn)
            generate_tarn_marker(tarn, tarn_index, image_filename)
        width, height = Image.open(image_filename).size
        context.add_object(staticmaps.ImageMarker(latlng, image_filename, width/2, height))
    return context




def main (directory='data/path', output='path.svg'):
    if not os.path.exists("data/markers"):
        os.makedirs("data/markers")

    context = plot_paths(directory)

    # # render non-anti-aliased png
    # image = context.render_pillow(800, 500)
    # image.save("path.png")

    # render svg
    svg_image = context.render_svg(800, 500)
    with open(output, "w", encoding="utf-8") as f:
        svg_image.write(f, pretty=True)

if __name__ == "__main__":
    directory = 'data/path'
    output = 'path.svg'
    if len(sys.argv) > 1:
        directory = sys.argv[1]
        if len(sys.argv) > 2:
            output = sys.argv[2]
    main(directory, output)