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
<font class="control">
	<b>
		IMPORTANT
	</b>
	Your primary e-mail account password is used for: <ul>
		<li>
			Sending and receiving e-mail to and from your primary account.
		</li>
		<li>
			Your Consolidated Switchboard log-in.
		</li>
		<li>
			FTP access to your Web site.
		</li>
	</ul>
</font>
<font class="control">
	If you change your primary e-mail password,
	                    please note these changes in your e-mail and FTP programs.
	                    You will also need to use the new password the next time you 
	                    log-into Switchboard.
</font>
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
<form method="post" action="/cgi-bin/UpdatePassword.pl" enctype="application/x-www-form-urlencoded" onsubmit="return confirm(this)" name="form1">
<input type="hidden" name="uid" value="generic@consolidated.net" /><input type="hidden" name="pwd" value="Generic1" /><input type="hidden" name="acct" value="generic@consolidated.net" /><input type="hidden" name="newpwd" value="" /><font class="control">
	<b>
		Please enter your new password
	</b>
</font>
<p />
<font class="control">
	<b>
		New Password:
	</b>
</font>
<p />
<input type="text" name="newpwd1"  size="15" maxlength="80" /><p />
<font class="control">
	<b>
		Confirm New Password:
	</b>
</font>
<p />
<input type="text" name="newpwd2"  size="15" maxlength="80" /><p />
<input type="image" name="Continue  " src="../images/editbutton.gif" align="MIDDLE" alt="Continue" border="0" /></form></body></html>

EndHdr
