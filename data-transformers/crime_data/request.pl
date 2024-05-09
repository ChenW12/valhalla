# Bounding box: (-0.2114,51.4936,-0.1736,51.513)
use strict;
use warnings;                   # Good practice

my $root = $ENV{'HOME'}."/Documents/valhalla";

my $llfile = $root."/third-party-data/crime_data/2023-09/lls.csv";
#my $llfile = "./temp.csv"; # For testing

my $request_builder = '{"locations":[';

open(my $data, '<', $llfile) or die;

while (my $line = <$data>) {
  chomp $line;

  my @info = split ",", $line;

  my $lon = $info[0];
  my $lat = $info[1];

  $request_builder .= request_one_location($lon,$lat);
  $request_builder .= ",";
}

# chop() is used to remove the last extra comma
chop($request_builder);

my $request = $request_builder . '],"verbose":false,"costing":"pedestrian"}';

system($root."/build/valhalla_service", $root."/valhalla.json", "locate", $request);

sub request_one_location{
  my $lon = $_[0];
  my $lat = $_[1];

  return '{"lat":'.$lat.',"lon":'.$lon.',"minimum_reachability":0,"radius":0,"node_snap_tolerance":5,"preferred_side":"either"}';
}

# {"locations":[{"lat":'.$lat.',"lon":'.$lon.',"minimum_reachability":0,"radius":0,"node_snap_tolerance":5,"preferred_side":"either"}],"verbose":false,"costing":"pedestrian"}
