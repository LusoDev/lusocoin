#!/usr/bin/perl
use strict;
use warnings;

# PARAMS
my $testnet=1;
my $mncol=30000;
my $mncount=1;
my $clipath="/home/user/luso-cli";
my $method="";
my $extip="127.0.0.1";


sub randstrz() {
  my $n=shift;
  my $nr;
  for (1 .. $n) {
      $nr.="%08X", rand(0xffffffff);
  }
  return $nr;
}

sub checkmake_user {
  my $user=shift;
  if (-d "/home/$user/"){
    print "User $user exists...\n";
  } else {
    system("adduser --disabled-password --gecos '' $user");
    system("su $user -c 'mkdir ~/.lusocore; mkdir ~/.lusocore/testnet3'");
    print "User $user created...\n";
    return 0;
  }
  return 1;
}

sub checkmake_sentinel {

}



sub checkmake_conf {
  my $user=shift;
  my $thisportrpc=shift;
  $thisportrpc=12312 if (!$thisportrpc || $thisportrpc eq "");
  my $thisport=shift;
  $thisport=12311 if (!$thisport || $thisport eq "");
  my $rpcuser=&randstrz(12);
  my $rpcpass=&randstrz(32);
  my $condfile="/home/$user/.lusocore/luso.conf";

  if (!-f $condfile || `su $user -c 'cat $condfile'` !~ /rpc/) {
    my $put="rpcuser=$rpcuser
rpcpassword=$rpcpass
rpcip=127.0.0.1
rpcport=$thisportrpc
rpcallowip=127.0.0.1
server=1
listen=1
externalip=$extip
remoteip=$extip
";
    system("su $user -c \"touch $condfile; echo '$put' > $condfile\"");
    return 0;
  }
  return 1;
}

sub confirmtxid {
  my $user=shift;
  my $txid=shift;
  my $confirmd=0;
  my $tries=0;
  while ($confirmd < 1 && $tries < 7) {
    if (my $try1=&try4ever("su $user -c '~/luso-cli gettransaction $txid'","confirmations\"")) {
      my ($confs) = ($try1 =~ /confirmations\": ([0-9]+)/);
      if (int($confs) > 15) {
        print "txid confirmed $confs times!\n";
        $confirmd=1;
      } else {
        print "txid found, ".(15-int($confs))." left, confirming...\n";
        sleep(90);
      }
    } else {
      $tries++;
      sleep(30);
    }
  }
  return $confirmd;
}

sub try4ever {
  my $cmd=shift;
  my $regex=shift;
  my $force=shift;
  my $tries=0;
  $tries=-30 if ($force);
  my $try1=`$cmd`;
  if ($try1 !~ /$regex/) {
    while($try1 !~ /$regex/) {
      return 0 if ($tries > 10);
      $try1=`$cmd`;
      sleep(30);
      $tries++;
    }
  }
  return $try1;
}

sub checkmake_confmn {
  my ($usrt,$genkey,$txid,$thisport) = @_;

  my $condfile="/home/$usrt/.lusocore/masternode.conf";
  $condfile="/home/$usrt/.lusocore/testnet3/masternode.conf" if ($testnet);

  if (!-f $condfile || `su $usrt -c 'cat $condfile'` !~ /$genkey/) {
    system("su $usrt -c \"touch $condfile; echo '$usrt $extip:$thisport $genkey $txid' > $condfile\"");
    my $conf="masternodeaddr=$extip:$thisport
masternode=1
masternodeprivkey=$genkey";
    system("su $usrt -c \"echo '$conf' >> /home/$usrt/.lusocore/luso.conf\"");
    return 0;
  }
  return 1;
}

sub checkmake_sentinel {
    my $usr=shift;
    my $put = "# specify path to luso.conf or leave blank
# default is the same as DashCore
dash_conf=/home/$usr/.lusocore/luso.conf

# valid options are mainnet, testnet (default=mainnet)
network=";
if ($testnet){
  $put.="testnet";
} else {
  $put.="mainnet";
}
$put.="
# database connection details
db_name=database/sentinel.db
db_driver=sqlite";
    system("su $usr -c 'echo \"$put\" > ~/sentinel/sentinel.conf'");
}

print "LusoCoin - Easy Masternode Tool\r\n";
print "$0 <method> <daemon-path> <mnnum> <externalip>\r\n";
$clipath=$ARGV[1] if ($ARGV[1] && $ARGV[1] ne "");
$mncount=int($ARGV[2]) if ($ARGV[2] && $ARGV[2] ne "");
$method=$ARGV[0] if ($ARGV[0] && $ARGV[0] ne "");
$extip=$ARGV[3] if ($ARGV[3] && $ARGV[3] ne "");

print "Params: <$method> <$clipath> <$mncount> <$extip>\r\n";

if ($method eq "") {
  print "No method...\n";
  die;
}

if (!-f $clipath."luso-cli") {
  print "File not found:\n$clipath\n";
  die;
}


my $a=-1;
while($a<($mncount-1)) {
  $a++;
  my $thisport=30001+$a;
  my $thisportrpc=31001+$a;
  my $usrt="luso$a";
  my $istestnet="";
  $istestnet=" -testnet" if ($testnet);
  my $clipatht="/home/$usrt/";
  my $flcli=$clipatht."luso-cli";
  my $fldmn=$clipatht."lusod";
  my $txid;
  if ($method eq "update") {

  } elsif ($method eq "new") {
    if (checkmake_user($usrt)) {
      print "User already exists.\n";
      #next;
    } else {
      system("cp ".$clipath."* $clipatht");
    }
    if (&checkmake_conf($usrt,$thisportrpc,$thisport)) {
      print "Already exists? checking daemon...\n";
      my $mnfl="$clipatht.lusocore/masternode.conf";
      $mnfl="$clipatht.lusocore/testnet3/masternode.conf" if ($testnet);
      my $ctmn=`cat $mnfl`;
      my ($mnname,$mnip,$mnkey,$txids,$txln) = ($ctmn =~ /([a-zA-Z0-9]+)\s([0-9\:\.]+)\s([a-zA-Z0-9]+)\s([a-zA-Z0-9]+)\s([0-9]+)\s/);
      if (`ps aux | grep lusod` !~ /port\=$thisport/){
        system("su $usrt -c '$fldmn$istestnet -daemon -port=$thisport 2>\&1 \&'");
        sleep(30);
      }
      print "Confirming $txids $txln ...\n";
      if (!&confirmtxid($usrt,$txids)) {
        next;
      }
      print "$mnname confirmed, available at $mnip\n";


      #next;
    } else {
      system("su $usrt -c '$fldmn$istestnet -daemon -port=$thisport 2>\&1 \&'");
      sleep(10);
      my $genkey=`su $usrt -c '$flcli masternode genkey'`;
      chomp($genkey);
      my $addr=`su $usrt -c '$flcli getnewaddress'`;
      chomp($addr);
      `su $usrt -c '$flcli stop'`;
      sleep 10;
      print "Masternode $a\nKey: $genkey\nAddress: $addr\nPort: $thisport\nIP: $extip\nRPCPort: $thisportrpc\n\nRestarting daemon, please send 30 000 LUSO to the this address, and paste here the full txid: ";
      $txid = <STDIN>;
      $txid =~ s/[\n\r\f\t]//g;
      chomp($txid);
      my ($txids,$txln) = split("-",$txid);
      $txln=int($txln);

      if (&checkmake_confmn($usrt,$genkey,$txids." ".$txln,$thisport)) {
        print "Configure this MN manually.";
        next;
      }

      system("su $usrt -c '$fldmn$istestnet -daemon -port=$thisport 2>\&1 \&'");
      print "Configured, going to confirm tx after starting daemon and waiting for syncing...\n";
      sleep(30);

      print "Confirming $txids $txln ...\n";
      if (!&confirmtxid($usrt,$txids)) {
        next;
      }


    }


    if (my $try1=&try4ever("su $usrt -c '$flcli masternode start-all'","Successfully started")) {
      print "Masternode is working, checking status!\n";
    } else {
      print "Unable to start-all, please do this manually or run this script again later.\n";
    }

    if (my $try2=&try4ever("su $usrt -c '$flcli masternode status'","Masternode successfully started")) {
      print "Masternode is working, finishing with sentinel!\n";
    } else {
      print "Unable to status, please do this manually or run this script again later.\n";
    }


    #&checkmake_sentinel($usrt);
  }
}
