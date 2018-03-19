#!/bin/perl

sub checkmake_conf {
  if (`cat ~/.lusocore/luso.conf` !~ /rpc/) {
    system("killall lusod; killall luso-qt");
    $put="mnconflock=0
rpcuser=[[[RANDUSER]]]
rpcpassword=[[[RANDPASS]]]
rpcip=127.0.0.1
rpcallowip=127.0.0.1
server=1
listen=1
addnode=5.154.191.152
addnode=185.228.233.201";
    system("echo '$put' > ~/.lusocore/luso.conf");
  }
}


my $mncol=30000;
my $mncount=1;
my $clipath="../../src/luso-cli";
print "LusoCoin - Easy Masternode Tool\r\n";
print "$0 <cli-path> <mnnum>\r\n";
$clipath=$1 if ($1 ne "" && -f $1);
$mncount=int($2) if ($2 && $2 ne "");
checkmake_conf();
=pod
my $a=0;
while($a<$mncount){
  $genkey=`$clipath masternode genkey`;
  $addr=`$clipath createnewaddress`;
  print "Masternode$a $genkey $addr"
  #$checktx=`$clipath createnewaddress`;
  #while()
  $a++;
}
=cut
