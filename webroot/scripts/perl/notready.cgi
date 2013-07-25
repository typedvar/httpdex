#!/usr/bin/perl

print "Content-type: text/html \n\n";

read(STDIN,$buffer,$ENV{'CONTENT_LENGTH'});

@values = split(/&/,$buffer);

 foreach $i (@values){
    ($varname, $nydata) = split(/=/,$i);
    $nydata =~ tr/\+/ /;
    $nydata =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C",hex($1))/eg;

    if ($varname =~ /uid/){
      $gotdata = $nydata;
      }elsif ($varname =~ /pwd/){
       $gotpass = $nydata;
      }
}
print <<EndHdr;


<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html
   PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<CENTER>
<TABLE BORDER>
        <TR>   
                <TD>
                    <TH COLSPAN=2></TH> 
                </TD>
   </TR>

   <TR>
        <TD></TD> <TD><B>Service</B></TD> <TD><B>Price</B></TD> <TD><B>Description</B></TD>
   </TR>


   <TR>

<form method="post" action="t.cgi" enctype="application/x-www-form-urlencoded">
      <TD><INPUT TYPE=CHECKBOX NAME="Anti-SPAM" value=1.75 </INPUT> </TD> <TD>Anti-SPAM</TD> <TD>\$1.75 per month</TD> <TD>Service Description Here</TD>
   </TR>

   <TR>
        <TD><INPUT TYPE=CHECKBOX NAME="Email Anti-Virus" value=1.75 </INPUT></TD><TD>Email Anti-Virus</TD> <TD>\$1.75 per month</TD> <TD>Service Description Here</TD>
   </TR>
<center>

</TABLE>

<input TYPE="SUBMIT" VALUE="Continue" align="RIGHT" />
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="pwd" value="$gotpass" />
</form>

<form method="post" action="login.cgi" enctype="application/x-www-form-urlencoded">
<input TYPE="SUBMIT" VALUE="Cancel" align="LEFT" />
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="pwd" value="$gotpass" />
</form>

</center>

EndHdr


