#!/usr/bin/perl

print "Content-type: text/html \n\n";

read(STDIN,$buffer,$ENV{'CONTENT_LENGTH'});

print <<EndOfHtml;
<html>
<head>
<title> Test </title>
</head>
<center>
<body>

<HTML>
<BODY LINK="#0000ff" VLINK="#800080">
EndOfHtml
print $buffer;

print <<EndOfText;
<body>
</html>
EndOfText


