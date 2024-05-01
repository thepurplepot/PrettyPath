import matplotlib.pyplot as plt
import staticmaps

class Node:
    def __init__(self, id, latitude, longitude, length=float('inf'), elevation=0):
        self.id = id
        self.longitude = longitude
        self.latitude = latitude
        self.length = length
        self.elevation = elevation

def get_path(filename='data/path.csv'):
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

path = get_path()

longitudes = [node.longitude for node in path]
latitudes = [node.latitude for node in path]

start = staticmaps.create_latlng(latitudes[0], longitudes[0])
end = staticmaps.create_latlng(latitudes[-1], longitudes[-1])

context = staticmaps.Context()
context.set_tile_provider(staticmaps.tile_provider_OSM)
# Add start and end markers
context.add_object(staticmaps.Marker(start, color=staticmaps.GREEN, size=12))
context.add_object(staticmaps.Marker(end, color=staticmaps.RED, size=12))
# Add path line
line = [staticmaps.create_latlng(node.latitude, node.longitude) for node in path]
context.add_object(staticmaps.Line(line, width=1))

# # render non-anti-aliased png
# image = context.render_pillow(800, 500)
# image.save("path.png")

# render svg
svg_image = context.render_svg(800, 500)
with open("path.svg", "w", encoding="utf-8") as f:
    svg_image.write(f, pretty=True)