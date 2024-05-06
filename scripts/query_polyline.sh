./../build/valhalla_service ../valhalla.json route '{"locations":[{"lat":51.50166540878063,"lon":-0.18284365986569398},{"lat":51.509793223963996,"lon":-0.18769830681451777}],"costing":"pedestrian","costing_options":{"pedestrian":{"use_lit":0,"less_crime_rate":false}},"directions_options":{"units":"kilometers"}}' | jq '.'

# Crossing Hyde Park
# 51.50166540878063, -0.18284365986569398
# 51.509793223963996, -0.18769830681451777
#
# 51.501872541217814, -0.18399788463246428
# 51.509226689865, -0.18742783060198462
