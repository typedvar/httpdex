#!/usr/bin/perl

print "Content-type: text/html \n\n";

print <<EndHdr;

<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html
	PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en-US" xml:lang="en-US"><head><title>New Password Form</title>
<link rev="made" href="mailto:wjustice%40pobox.com" />
<link rel="stylesheet" type="text/css" href="/index.css" />
</head><body bgcolor="#ffffff"><img src="../images/i_questionanswer.gif" />
<p />
<b>
	Current Question is: 
</b>
Not Available<p />
<b>
	Current Answer is: 
</b>
Not Available<p />
<form method="get" action="/cgi-bin/UpdateQNA.pl" enctype="application/x-www-form-urlencoded" name="form1">
<input type="hidden" name="uid" value="generic@consolidated.net" /><input type="hidden" name="pwd" value="Generic1" /><p />
<font class="control">
	<b>
		Please enter your new question and answer
	</b>
</font>
<p />
<font class="control">
	<b>
		New Question:
	</b>
</font>
<input type="text" name="newQ"  size="25" maxlength="80" /><p />
<font class="control">
	<b>
		New Answer:
	</b>
</font>
<input type="text" name="newA"  size="15" maxlength="80" /><p />
<input type="image" name="Continue" src="../images/editbutton.gif" align="MIDDLE" alt="Continue" border="0" /></form></body></html>



EndHdr
