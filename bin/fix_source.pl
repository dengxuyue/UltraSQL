#!/usr/bin/perl -w

use File::Copy;
my $dir = $ARGV[0];
$dir =~ s/^[\.\/]+//;
$dir =~ s/[\.\/]+$//;

my @files = glob("$dir/*.c $dir/*.h");
for my $f(@files) {
    if ( -f "$f.1" ){
        print "Cannot remove $f.1", next unless unlink "$f.1";
    }

    copy $f, "$f.1";
    open my $fh1, ">", $f or next;
    open my $fh2, "<", "$f.1" or next;
    while(my $line = <$fh2>) {
        $line =~ s/\s+$//g;
        print $fh1 "$line\n";
    }
    close $fh1;
    close $fh2;

    unlink "$f.1";
}
