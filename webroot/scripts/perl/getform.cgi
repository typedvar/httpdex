#! /usr/bin/perl 

print <<ENDhtml;

<html>
<form action="get.cgi" method="GET">
First Name : <input type="text" name="fname" size=30><p>
Last Name : <input type="text" name="lname" size=30><p>
<input type="submit">
</form>
</html>
ENDhtml
