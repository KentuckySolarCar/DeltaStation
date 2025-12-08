from staticmap import StaticMap, CircleMarker
import sys

size = 200
try:
    m = StaticMap(size, size)
    m.add_marker(CircleMarker((float(sys.argv[2]), float(sys.argv[1])), 'blue', 8)) # add marker in center
    image = m.render(zoom=13) # make image
    image.save('../src/gps/map.png')
    sys.exit(0)
except Exception:
    sys.exit(1)