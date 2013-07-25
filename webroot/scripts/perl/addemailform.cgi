#!/usr/bin/perl

print "Content-type: text/html \n\n";

print <<EndHdr;


<?xml version="1.0" encoding="iso-8859-1"?>
<!DOCTYPE html
	PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
	 "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="en-US" xml:lang="en-US"><head><title>Switchboard - Control Panel Home</title>
<link rev="made" href="mailto:wjustice%40pobox.com" />
<link rel="stylesheet" type="text/css" href="/index.css" />
<script language="JavaScript" src="/pwrules.js" type="text/javascript"></script>

</head><body bgcolor="#ffffff"><table bgcolor="#ffffff" border="0" cellspacing="0" cellpadding="2" width="100%">
<tr>
	<td bgcolor="#010189" align="left" height="50" valign="bottom" colspan="4"><img left border="0" height="65" src="../images/logo_blue.gif" width="315" />
</td>
</tr>
<tr>
	<td><h1>
	Internet Account Administration
</h1></td>
</tr>
</table>
<h1 class="control">
	Add Additional E-mail accounts
</h1>
<p>
	<b>
		E-mail Account Name Rules:
	</b>
</p>
<ul>
	<li>
		Your new e-mail account name must be 6 to 15 characters in length.<br>(Not counting the domain.)
	</li>
</ul>
<p>
	For example: <i>testuser</i>
</p>
<p />
<font class="control">
	<b>
		Password Rules
	</b>
	<ul>
		<li>
			Your password must be 6 to 15 characters in length.
		</li>
		<li>
			Your password must contain at least one capital letter, 
			                    one lowercase letter and one number.
		</li>
		<li>
			Your password must contain only alphanumeric characters.
		</li>
	</ul>
	For example:  <i>
		TestUs3r
	</i>
</font>
<p />
<form method="get" action="/cgi-bin/addemailaccount.pl" enctype="application/x-www-form-urlencoded" onsubmit="return confirm(this)" name="form1">
<input type="hidden" name="uid" value="generic@consolidated.net" /><input type="hidden" name="pwd" value="Generic1" /><input type="hidden" name="newpwd" value="" /><table bgcolor="#ffffff" border="0" cellspacing="0" cellpadding="2">
<tr>
	<td align="right"><font class="control">
	<b>
		E-mail Account Name:
	</b>
</font>
</td>
	<td align="right"><input type="text" name="newacct"  size="15" maxlength="80" /></td>
	<td align="right"><font class="control">
	<b>
		@consolidated.net
	</b>
</font>
</td>
</tr>
<tr>
	<td align="right"><font class="control">
	<b>
		New Password:
	</b>
</font>
</td>
	<td align="right"><input type="text" name="newpwd1"  size="15" maxlength="80" /></td>
	<td align="right"><font>
	&nbsp
</font>
</td>
</tr>
<tr>
	<td align="right"><font class="control">
	<b>
		Confirm New Password:
	</b>
</font>
</td>
	<td align="right"><input type="text" name="newpwd2"  size="15" maxlength="80" /></td>
	<td align="right"><font>
	&nbsp
</font>
</td>
</tr>
</table>
<p />
<input type="image" name="Continue" src="../images/gobutton.gif" align="MIDDLE" alt="Continue" border="0" /></form></body></html>

EndHdr
