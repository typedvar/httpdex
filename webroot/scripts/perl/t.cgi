#!/usr/bin/perl

print "Content-type: text/html \n\n";

read(STDIN,$buffer,$ENV{'CONTENT_LENGTH'});

@values = split(/&/,$buffer);
$count = 0; 
$purchased=0;

foreach $i (@values){
    ($varname, $nydata) = split(/=/,$i);
    $nydata =~ tr/+/ /;
    $nydata =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C",hex($1))/eg;

    if ($varname =~ /uid/){
     $gotdata = $nydata;
      }elsif ($varname =~ /pwd/){
       $gotpass = $nydata;
      } else {
        $varname =~ tr/\+/ /;
        $services[$count] = $varname;
        $purchased = $purchased + $nydata;
        $count = $count + 1;
      }
       
}

print <<EndOfHtml;
<html>
<head>
<title> Test </title>
</head>
<center>
<body>

<HTML>
<BODY LINK="#0000ff" VLINK="#800080">


<UL>
<P ALIGN="CENTER"><LI>Purchased services will be automatically provisioned and instantly available for use.</LI></P>
<P ALIGN="CENTER"><LI>The billing total based on services ordered</LI></P>
<P ALIGN="CENTER"><LI>Charges will appear on the same telephone number that their Internet service is billed on.</LI></P>
<P ALIGN="CENTER"><LI>Charges are effective as of the current date and are not prorated</LI></P></UL>

<FONT SIZE=2><P ALIGN="JUSTIFY">&nbsp;</P></FONT></BODY>
</HTML>

</body>
</center>

<center>

<form method="post" action="/cgi-bin/imple.cgi" enctype="application/x-www-form-urlencoded">
<input TYPE="SUBMIT" VALUE="Submit Order" align="RIGHT" />
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="services1" value="$services[0]" />
<input type="hidden" name="services2" value="$services[1]" />
<input type="hidden" name="purchased" value="$purchased" />
</form>
<form method="post" action="/cgi-bin/login.cgi" enctype="application/x-www-form-urlencoded">
<input TYPE="SUBMIT" VALUE="Cancel Order" align="LEFT" />
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="pwd" value="$gotpass" />
</form>

</center>
</html>

EndOfHtml


