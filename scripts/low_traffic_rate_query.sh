./../build/valhalla_service ../valhalla.json route '{"locations":[{"lat":41.7899513,"lon":-87.6997816},{"lat":41.7881185,"lon":-87.7009504}],"costing":"pedestrian","costing_options":{"pedestrian":{"use_lit":0,"less_crime_rate":false}},"directions_options":{"units":"kilometers"},"date_time":{"type":1,"value":"2024-05-06T10:04"}}' | jq '.'

# Somewhere in Chicago
# 41.7881185, -87.7009504
# 41.7899513, -87.6997816
# 41.7867448, -87.7009143
