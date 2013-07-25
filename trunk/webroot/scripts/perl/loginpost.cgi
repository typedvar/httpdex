#!/usr/bin/perl

print "Content-type: text/plain \n\n";

read(STDIN,$buffer,$ENV{'CONTENT_LENGTH'});

# @values = split(/&/,$ENV{'QUERY_STRING'});

@values = split(/&/,$buffer);

 foreach $i (@values){
    ($varname, $nydata) = split(/=/,$i);
    $nydata =~ tr/+/ /;
    $nydata =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C",hex($1))/eg;
$nydata =~ s/\+/ /g; # the word seperator + is being replaced by a blank
#  if ($varname =~ /uid/){
 print "$varname = $nydata \n";
# }
# $FORM{$NAME} = $value;
}

