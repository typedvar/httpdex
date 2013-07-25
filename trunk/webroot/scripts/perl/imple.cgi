#!/usr/bin/perl

use Time::localtime;
print "Content-type: text/html \n\n";

#print <<ENDhtml;
#<html><head><title>Processing </title></head>
#<body>
#   <h2> Please wait while some internal processing is going on .... </h2>
#</body>
#</html>

#ENDhtml

#use Time::localtime;

#print <<ENDhtml;
#<html><head><title>Processing </title></head>
#<body>
#   <h2> Please wait while some internal processing is going on .... </h2>
#</body>
#</html>

#ENDhtml

read(STDIN,$buffer,$ENV{'CONTENT_LENGTH'});
open (DATAFILE,"> data") or die "can't write data: $!";
@values = split(/&/,$buffer);
$count = 0; 
$state = 0;
 foreach $i (@values){
  ($varname, $nydata) = split(/=/,$i);
  $nydata =~ tr/+/ /;
  $nydata =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C",hex($1))/eg;

    if ($varname =~ /uid/){
     $gotdata = $nydata;
      }elsif ($varname =~ /pwd/){
       $gotpass = $nydata;
      } elsif ($varname =~ /services*/) {
        $varname =~ tr/\+/ /;
        $services[$count] = $varname;
        $servicesName[$count] = $nydata;
        $count = $count + 1;
      }
    if ($varname =~/purchased/){
       if($nydata == 0){
         print DATAFILE "$varname = \$0.00 \n";
         $purchasedinfo = "$varname = \$0.00 \n";
       }   else{
           print DATAFILE "$varname = \$$nydata \n";
           $purchasedinfo = "$varname = \$$nydata \n";
           }
    }else {
             print DATAFILE "$varname = $nydata \n";
             if($state == 0){
                print DATAFILE "Telephone = 217-382-4144 \n";
                $state=1;
             }
          }
 
}


$tm = localtime;
$dt=sprintf("%04d/%02d/%02d",$tm->year+1900,($tm->mon) + 1, $tm->mday);
$tm=sprintf("%02d:%02d:%02d\n",$tm->hour,$tm->min, $tm->sec);
print DATAFILE "$dt-$tm";
close (DATAFILE);

#  Sending mail with the following data .....

# $gotdata , @servicesName, $purchasedinfo and date-$dt,time-$t
 
# unattended Mail::Sendmail test, sends a message to the author
# but you probably want to change $mail{To} below
# to send the message to yourself.
# version 0.78

# if you change your mail server, you may need to change the From:
# address below.
$mail{From} = 'Sendmail Test <abanerjee@telephone.com>';

#$mail{Cc} = 'Sendmail Test <Amitava_Banerjee@syntelinc.com>';

$mail{To} = 'Amitava_Banerjee@syntelinc.com <Amitava_Banerjee@syntelinc.com>';

# $mail{To}   = 'Sendmail Test <Sridhar_Rangarajan@syntelinc.com>';

# if you want to get a copy of the test mail, you need to specify your
# own server here, by name or IP address
$server = '172.17.30.19';
#$server = 'my.usual.mail.server';

# BEGIN { $| = 1; print "1..2\n"; }
# END {print "not ok 1\n" unless $loaded;}

use Mail::Sendmail;
# $loaded = 1;
# print "ok 1\n";


if ($server) {
    $mail{Smtp} = $server;
#    print "Server set to: $server\n";
}

$mail{Subject} = "It's a test mail... from Linux ";

$mail{Message} = "Hi, \n\n";
$mail{Message} .= "You ordered The Following :\n\n ";
$mail{Message} .= "User-Id = $gotdata \n";
$mail{Message} .= "Telephone = 217-382-4144 \n";
     foreach $i (@servicesName){
    $mail{Message} .= "Services Name:  $i \n";
    }
 $mail{Message} .= "Purchased Amt =  $purchasedinfo \n";
 $mail{Message} .= "Date And Time : $dt - $tm \n";
 $mail{Message} .= "Administrator \n";

# Go send it
#print "Sending...\n";

if (sendmail %mail) {
#    print "content of \$Mail::Sendmail::log:\n$Mail::Sendmail::log\n";
    if ($Mail::Sendmail::error) {
#        print "content of \$Mail::Sendmail::error:\n$Mail::Sendmail::error\n";
    }
#    print "ok 2\n";
}
else {
#    print "\n!Error sending mail:\n$Mail::Sendmail::error\n";
#    print "not ok 2\n";
}

print <<ENDhtml;
<html><head><title>Back to the Internet Account Manager </title></head>
<body>
   <h2> Purchased Detail has been sent to your Primary account  .... </h2>
<form method="post" action="/cgi-bin/login.cgi" enctype="application/x-www-form-urlencoded">
<input TYPE="SUBMIT" VALUE="Back" align="RIGHT" />
<input type="hidden" name="uid" value="$gotdata" />
</body>
</html>
ENDhtml
