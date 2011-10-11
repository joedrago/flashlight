use strict;
use FindBin;

my $astylerc = $FindBin::Bin . "/astylerc";

my $list;
if($^O =~ /Win32/)
{
    $list = join(' ', grep { !/external/ } map { chomp; $_ } `dir /s /b *.c *.h`);
    $astylerc =~ s/\//\\/g;
}
else
{
    $list = join(' ', map { chomp; $_ } `find lib game -name \\*.c`);
    $list .= " " . join(' ', map { chomp; $_ } `find lib game -name \\*.h`);
}

my $cmd = "astyle --options=$astylerc $list";
system($cmd);

