import json

# every way id maps to the number of crime occured on that way
wayid_crime: dict[int, int] = dict();

with open('./temp2.json') as f:
    data = json.load(f);
    x = 0
    for response in data:
        # A way may appear multiple times in a response, so we need this set to make
        # sure every crime should be added only once to every way
        added_ways = set();
        for edge in response["edges"]:
            way_id = edge["way_id"]
            # Check if the crime has been added to the way
            if ( way_id in added_ways):
                # If the crime has been added to the way, we will skip it
                continue
            else:
                # If the crime has not been added to the way, we add it into added_edges
                # to record that this crime has been added to that way
                added_ways.add(way_id)

                # Check if the way id has been added in the dict
                if (way_id in wayid_crime):
                    # If the way id exists, we inc the value indiciates that a crime record is
                    # added to this way
                    wayid_crime[way_id] += 1
                else:
                    # If the way id does not exist, we give it a start value 1
                    wayid_crime[way_id] = 1
    # Print the dict
    print(wayid_crime)
