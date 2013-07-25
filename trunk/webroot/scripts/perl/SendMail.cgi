#!/usr/bin/perl -w
#
# SendMail.cgi -- This is a perl CGI script which is using SendMail.pm
#		  module to send the submitted data.
#		  It does not need any sendmail program, just needs the
#		  SendMail.pm. And it works in both UNIX and Windows
#		  platforms. But it require perl version 5.
#		  Source code for this cgi script and SendMail.pm can
#		  be downloaded at
#		  http://www.tneoh.zoneit.com/perl/SendMail/
#		  If you are on Windows platform, make sure your machine
#		  is the SMTP server itself, else instead of using 
#		  $sm = new SendMail();
#		  change it to:
#		  $sm = new SendMail("your.smtp.server");
#		  And for Windows platform users, make sure you have
#		  used a correct perl path at the first line of this
#		  CGI script. It should be something like:
#		  #!C:/perl5/perl-5.000/perl/bin/perl.exe
#
#		  And change the sender's email address if you want.
#
# Simon Tneoh Chee-Boon tneohcb@pc.jaring.my
# 
# Copyright (c) 1998-2003 Simon Tneoh Chee-Boon. All rights reserved.
# This program is free software; you can redistribute it and/or
# modify it under the same terms as Perl itself.
#

use strict;
use CGI;
$| = 1;

print CGI->header();

#
# Please refer to http://www.tneoh.zoneit.com/perl/SendMail/
#
eval("use SendMail;");

print "Error: $@" if $@;

my($sender)   = "\"Your Name\" <yourid\@your.mail.domain>";
my($subject)  = "Thanks for testing SendMail.cgi";
my($errorsto) = $sender;
my($replyto)  = $sender;
my($cgi)   = new CGI;

my($username) = $cgi->param('username');
$username ||= "Your Name";
my($useremail) = $cgi->param('email');
$useremail ||= "";
my($message) = $cgi->param('message');
$message ||= "";
my($recipient) = "\"$username\" <$useremail>";
my($sm) = new SendMail;
$sm->From($sender);
$sm->To($recipient);
$sm->ReplyTo($sender);
$sm->ErrorsTo($errorsto);
$sm->Subject($subject);
$sm->setMailHeader("URL", "http://www.tneoh.zoneit.com/perl/SendMail/");

my($mailbody) = <<__END__MAILBODY__;

Dear $username,

   Thanks for testing the SendMail.cgi CGI script.

   You have submitted the following information:
Your Name: $username
Your Email Add.: $useremail
Your Message: 
$message

Regards,

--
Simon Tneoh Chee-Boon
System Analyst	MyBiz International Group
tneohcb\@pc.jaring.my	60-3-8996-0966
http://www.tneoh.zoneit.com/simon/

__END__MAILBODY__
$sm->setMailBody($mailbody);
if ($sm->sendMail() != 0) {
  printError($sm->{'error'});
  exit;
}
printThankYou();

#
# You can uncomment the following four lines if you're on
# UNIX platform.
#
#my($pid) = fork;
#$pid = 0 unless defined $pid;
#exit 0 if $pid;
#close(STDOUT);

#
# Now send the submitted data to the sender.
#

#
# We reset the SendMail object to clear the data that we have set
# into the object, like sender, recipient, subject and etc.
#
$sm->reset();
$replyto = $recipient;
$recipient = $sender;
$subject = "User tested SendMail.cgi.";
$sm->From($sender);
$sm->Subject($subject);
$sm->To($recipient);

my($localtime) = scalar(localtime());
$mailbody = <<__END__MAILBODY__;

Time: $localtime
User Name: $username
User Email Add.: $useremail
User Message: 
$message

__END__MAILBODY__
$sm->setMailBody($mailbody);
if ($sm->sendMail() != 0) {
  printError($sm->{'error'});
  exit;
}
exit 0;

sub printError {
    my($error) = shift;

    print <<__END__ERROR__;
<html>
<head>
<title>Error</title>
</head>
<body bgcolor="white">
<h2>Error</h2>
<hr>
<blockquote>
$error<p>
Please click <a href="javascript:history.go(-1)">here</a> to try again.<p>
</blockquote>
<hr>
</body>
</html>
__END__ERROR__

    return 0;
}

sub printThankYou {
    print <<__END__THANKYOU__;
<html>
<head>
<title>Thank You</title>
</head>
<body bgcolor="white">
<h2>Thank You</h2>
<hr>
<blockquote>
Dear $username,<p>
Thanks for testing.<p>
Your submission has been sent to me and make a copy to you.<p>
Please click <a href="./index.html">here</a> to go back to SendMail.pm
demonstration page.<p>
</blockquote>
<hr>
</body>
</html>
__END__THANKYOU__
    return 0;
}
