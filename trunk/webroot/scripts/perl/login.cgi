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
<html xmlns="http://www.w3.org/1999/xhtml" lang="en-US" xml:lang="en-US"><head><title>Switchboard - Control Panel Home</title>
<link rev="made" href="mailto:wjustice%5C%40pobox.com" />
<link rel="stylesheet" type="text/css" href="/index.css" />
</head><body bgcolor="#ffffff"><table bgcolor="#ffffff" border="0" cellspacing="0" cellpadding="2" width="100%">
<tr>
	<td bgcolor="#010189" align="left" height="50" valign="bottom" colspan="4"><img left border="0" height="65" src="/images/logo_blue.gif" width="315" />
</td>
</tr>
<tr>
	<td><h1>
	Internet Account Administration
</h1></td>
</tr>
<tr>
	<td bgcolor="#bbbbbb" colspan="4"><tt class="control">
	<b>E-mail Accounts</b>
</tt>
</td>
</tr>
<tr bgcolor="#ffffff">
	<th align="left">
		Address
	</th>
	<th align="left">
		Type
	</th>
	<th align="left">
		
	</th>
	<th align="left">
		
	</th>
</tr>
<form method="POST" action="/cgi-bin/EditAccount.cgi" enctype="application/x-www-form-urlencoded" name="form_parent">
<input type="hidden" name="acct" value="$gotdata" /><input type="hidden" name="uid" value="$gotdata" /><input type="hidden" name="pwd" value="$gotpass" /><tr bgcolor="#dddddd">
<td width="50%">$gotdata</td>
<td width="20%">Primary</td>
<td align="left" width="20%"><input type="image" name="edit" src="/images/editbutton.gif" align="MIDDLE" /></td>
<td align="right" width="10%"></td>
</tr>
</form></table>
<table bgcolor="#ffffff" border="0" cellspacing="0" cellpadding="2" width="100%">
<tr>
<td bgcolor="#ffffff" height="1" colspan="2"><hr size="5" noshade="10" /></td>
</tr>
<tr>
<td colspan="2" />
</tr>
<tr>
<td><font class="control">
	<b>Add Additional E-mail Accounts</b>
</font>
</td>
<td>
<form method="post" action="/cgi-bin/addemailform.cgi" enctype="application/x-www-form-urlencoded">
<input type="hidden" name="uid" value="$gotdata" /><input type="hidden" name="pwd" value="$gotpass" /><input type="image" name="Add" src="/images/addbutton.gif" align="RIGHT" /></form></td>
</tr>
<tr>
<td colspan="2" />
</tr>
<tr>
	<td bgcolor="#ffffff" colspan="2"><hr size="2" noshade="1" /></td>
</tr>
<tr>
<td colspan="2" />
</tr>
</table>
<table bgcolor="#ffffff" border="0" cellspacing="0" cellpadding="2" width="100%">
<tr>
	<td bgcolor="#bbbbbb" colspan="3"><tt class="control">
	<b>
		Account Management
	</b>
</tt>
</td>
</tr>
<tr>
<td><img height="25" src="/images/shim.gif" width="1" /></td>
<td valign="middle"><font class="control">
	<b>
		Change Primary Account Password
	</b>
</font>
</td>
<td>
<form method="post" action="/cgi-bin/EditAccount.cgi" enctype="application/x-www-form-urlencoded">
<input type="hidden" name="acct" value="$gotdata" />
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="pwd" value="$gotpass" />
<input type="image" name="edit" src="/images/gobutton.gif" align="RIGHT" />

</form></td>
</tr>
<tr bgcolor="#dddddd">
<td><img height="25" src="/images/shim.gif" width="1" /></td>
<td valign="middle"><font class="control">
	<b>
		Change Forgotten Password Information
	</b>
</font>
</td>
<td>
<form method="post" action="/cgi-bin/ChangeQNA.cgi" enctype="application/x-www-form-urlencoded">
<input type="hidden" name="uid" value="$gotdata" /><input type="hidden" name="pwd" value="$gotpass" /><input type="image" name="Go" src="/images/gobutton.gif" align="RIGHT" /></form></td>
</tr>

<td><img height="25" src="/images/shim.gif" width="1" /></td>
<td valign="middle"><font class="control">
	<b>
		Add Messaging Services
	</b>
</font>
</td>
<td>
<form method="post" action="/cgi-bin/notready.cgi" enctype="application/x-www-form-urlencoded">
<input type="hidden" name="uid" value="$gotdata" />
<input type="hidden" name="pwd" value="$gotpass" />
<input type="image" name="Go" src="/images/gobutton.gif" align="RIGHT" />
</form></td>
</tr>
<tr>
	<td bgcolor="#ffffff" height="1" colspan="3"><hr size="5" noshade="1" /></td>
</tr>
<tr>
<td colspan="3" />
</tr>
</table>
<p />
</body></html>

EndHdr
