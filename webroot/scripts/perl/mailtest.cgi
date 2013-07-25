#!/opt/bin/perl 

$mailprog = '/usr/lib/sendmail';
# put a comma between multiple recipients
$recipient = 'Amitava_Banerjee@syntelinc.com';

# The e-mail address listed above should be changed to the complete 
# e-mail address where you would like the form's information delivered. 


print "Content-type: text/html\n\n";
print "<Head><Title>Thank you</Title></Head>";
print "<Body><H3><CENTER>Thanks for your input</CENTER></H3>";
# Get the input
read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});
@pairs = split(/&/, $buffer); 
foreach $pair (@pairs) 
{
($name, $value) = split(/=/, $pair); 
$value =~ tr/+/ /;
$value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg; 
$FORM{$name} = $value; 
}
#Now send mail to $recipient
open (MAIL, "|$mailprog $recipient") || die "Can't open $mailprog!\n";
print MAIL "Reply-to: $FORM{'username'} ($FORM{'realname'})\n";
print MAIL "Subject: Question/Comment\n"; 
print MAIL "\n-------------------\n";
print MAIL "Name: ", $FORM{'realname'}, "\n";
print MAIL "E-Mail: ", $FORM{'username'}, "\n";
print MAIL "Note: ",$FORM{'note'},"\n"; 
print MAIL "Remote IP address: $ENV{'REMOTE_ADDR'}\n";
close (MAIL);
print "Thanks. Your input has been mailed to enter the recipients name";
