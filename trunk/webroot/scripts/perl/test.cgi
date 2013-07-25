#!/usr/bin/perl 
use Time::localtime;
$tm = localtime;
printf("The current date is %04d-%02d-%02d\n",$tm->year+1900,($tm->mon) + 1, $tm->mday);
printf("\nThe current time is %02d:%02d:%02d\n",$tm->hour,$tm->min, $tm->sec);
$p="testing@consolidated.net";
$k = $p;
print "\n$k\n";

$p = 1.75;
$m = 1.75;

$r = $p + $m;
print "$r\n";

