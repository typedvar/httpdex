#!/usr/bin/perl

$mailprog = '/usr/lib/sendmail';

# comma between multiple recipients

$recipient = 'Avinandan_Sengupta@syntelinc.com';


print "Content-type: text/html\n\n";
print "<Head><Title>Thank you</Title></Head>";
print "<Body><H3><CENTER>Thanks for your input</CENTER></H3>";

#Now send mail to $recipient

open (MAIL, "|$mailprog $recipient") || die "Can't open $mailprog!\n";
print MAIL "Hello \n";
print MAIL "Subject: Question/Comment\n";
print MAIL "\n-------------------\n";
print MAIL "Hello \n";
print MAIL "How things are moving to ..... \n";
close (MAIL);

print "Thanks. Your input has been mailed to enter the recipients name";
