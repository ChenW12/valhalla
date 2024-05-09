# Crime ID,Month,Reported by,Falls within,Longitude,Latitude,Location,LSOA code,LSOA name,Crime type,Last outcome category,Context
#
# Bounding box: (-0.2114,51.4936,-0.1736,51.513)
use strict;
use warnings;                   # Good practice

use Scalar::Util qw(looks_like_number);

my $max_lat = 51.513;
my $min_lat = 51.4936;
my $max_lon = -0.1736;
my $min_lon = -0.2114;

my $root = $ENV{'HOME'}."/Documents/valhalla";

my $file = $root."/third-party-data/crime_data/2023-09/2023-09-metropolitan-street.csv";

open(my $data, '<', $file) or die;

while (my $line = <$data>) {
  chomp $line;

  my @info = split ",", $line;

  my $lon = $info[4];
  my $lat = $info[5];

  if (looks_like_number($lon) && within_box($lat,$lon)) {
    print "$lon,$lat\n";
  }
}

sub within_box {
  my $lat = $_[0];
  my $lon = $_[1];

  return ($lat < $max_lat && $lat > $min_lat) && ($lon < $max_lon && $lon > $min_lon);
}
