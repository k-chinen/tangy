#!/usr/bin/perl
#
# tangy - figure generator
#       by k-chinen, 2013
#

use strict;
use Getopt::Std;

my %opt;
getopts('vhSsGgbLZ:o:X' => \%opt);

my $outputguideline = 0;
if($opt{'G'}) {
    $outputguideline = 1;
}

my $outputgrid = 0;
if($opt{'g'}) {
    $outputgrid = 1;
}

my $outputbb = 0;
if($opt{'b'}) {
    $outputbb = 1;
}

my $outputexl = 0;
if($opt{'L'}) {
    $outputexl = 1;
}

my $debug = 0;
if($opt{'X'}) {
    $debug = 1;
}

my $outputfilename = 'out.eps';
if($opt{'o'}) {
    $outputfilename = $opt{'o'};
}

my $defaultobjclass = 'box';
if($opt{'Z'}) {
    $defaultobjclass = $opt{'Z'};
}

if($opt{'S'}) {
    print <<EOM;
put object with relations
    put <object>[,<object>] hori
    put <object>[,<object>] xhori
    put <object> (right|left|up|down)[near] <object>
    put <object> (hslots|vslots) [heads <heads>] [lastletter <lastletter>]

set object's properties
    option <object> <prop-key> <prop-val> [<prop-key> <prop-val>]
        prop-key:   label str;  framecolor color;  fillcolor color;
                    thick num

link objects
    link <object>[,<object>] <linetype> [<props>] [indirect]
        linetype: line|cline|xline
        linestyle: dashed|dotted|chained
    link from <object>[,<object>] to <object>[,<object>] <linetype> [<props>] [indirect]

gather objects
    gather <obejct>[,<object>] (box|rbox) [<props>]
EOM
    exit 0;
}

if($opt{'s'}) {
    print <<EOM;
### sample of tangy script
# sample1
put A,B,C hori
option A class drum label input
option B label something
option C label output
link A,B,C line arrow
gather A,B rbox back fillcolor \@fcf fill noframe
# sample2
put A vslots lastletter 32
put A3,e,B xhori
link from A1,A2,A3,A32 to B cline
EOM
    exit 0;
}

if($opt{'h'}) {
    print <<EOM;
tangy - PS picture generator
usage: $0 [options] <files>
option:
    -h          print this
    -v          print version
    -s          print sample
    -S          print syntax
    -o file     set output filename (default $outputfilename)
    -Z class    set default object class
  for debug
    -L          output explicit label
    -g          output grid
    -b          output boundingbox
    -G          output guidelines

object class:
    box, rbox, xbox, diamond, circle, drum, pipe
    hcdots, vcdots, switch, router, cloud, file
draw type
    link: line, cline, xline, thunder, wave
    box: box, rbox -- dashed, dotted, chained
 
EOM
    exit 0;
}

if($opt{'v'}) {
    print <<EOM;
tangy version 0.02 by k-chinen at 2013/08/31
EOM
    exit 0;
}

my @boxcatname = ("box", "rbox", "ellipse");
my %boxcat;

{
    foreach my $i (@boxcatname) {
        $boxcat{$i}++;
    }
}


sub dashresolve {
    my($pp) = @_;
    my $cont;
    
    $cont = '';

    if(defined $pp->{longdashed}) {
        $cont .= " [1 1] 0 setdash\n";
    }
    if(defined $pp->{dashed}) {
        $cont .= " [0.5 0.5] 0 setdash\n";
    }
    elsif(defined $pp->{dotted}) {
        $cont .= " [0.25 0.25] 0 setdash\n";
    }
    elsif(defined $pp->{chained}) {
        $cont .= " [0.75 0.25 0.25 0.25] 0 setdash\n";
    }

    return $cont;
}

#
#   colorname or hex (c.g., @fff and  @ffffff )
#
sub colorresolve {
    my($src) = @_;
    my $dst;
    if($src eq 'white') {
        $dst = "1 1 1";
    }
    elsif($src eq 'gray') {
        $dst = "0.5 0.5 0.5";
    }
    elsif($src eq 'lightgray') {
        $dst = "0.75 0.75 0.75";
    }
    elsif($src eq 'darkgray') {
        $dst = "0.25 0.25 0.25";
    }
    elsif($src eq 'black') {
        $dst = "0 0 0 ";
    }
    elsif($src eq 'red') {
        $dst = "1 0 0 ";
    }
    elsif($src eq 'green') {
        $dst = "0 1 0 ";
    }
    elsif($src eq 'blue') {
        $dst = "0 0 1 ";
    }
    elsif($src eq 'cyan') {
        $dst = "0 1 1 ";
    }
    elsif($src eq 'yellow') {
        $dst = "1 1 0 ";
    }
    elsif($src eq 'magenta') {
        $dst = "1 0 1 ";
    }
    elsif($src =~ /\@/) {
        my @cp = split(//, $src);
        print "cp num $#cp\n";
        if($#cp==3) {
            $dst .= hex($cp[1])/15 . " ";
            $dst .= hex($cp[2])/15 . " ";
            $dst .= hex($cp[3])/15 . " ";
        }
        elsif($#cp==6) {
            $dst .= hex($cp[1].$cp[2])/255 . " ";
            $dst .= hex($cp[3].$cp[4])/255 . " ";
            $dst .= hex($cp[5].$cp[6])/255 . " ";
        }
    }

    return $dst;
}

sub mkprop {
    my($pp) = @_;
    my $pre;

    $pre = '';
    if(defined $pp->{thick}) {
        $pre .= "  $pp->{thick} 0.1 mul setlinewidth\n";
    }
    if(defined $pp->{framecolor}) {
        my $cn = &colorresolve($pp->{framecolor});
        if($cn eq '') {
            $pre .= "%ignore color; fail safe\n";
            $cn = '0 0 0';
        }
        $pre .= "  $cn setrgbcolor\n";
    }

    {
        my $tmp;
        $tmp = &dashresolve($pp);
        if($tmp ne '') {
            $pre .= $tmp;
        }
    }
    
    return $pre;
}

#
# insert hash into array of hashes
#        record into array of records
sub insert_mtoa {
    my($an, $kn, $kv) = @_;
    for my $q (keys %{ $kv }) {
        $an->{$kn}->{$q} = $kv->{$q};
    }
}


my %genattrdefault = ('thick', '1', 'style', 'solid',
        'fhead', 'solid', 'rhead', 'solid',
        'framecolor', 'black', 'fillcolor', 'white' );

sub insert_mtoa_withdefaults {
    my($an, $kn, $kv) = @_;
    for my $n (sort keys %genattrdefault) {
print " kn $kn n $n ; $genattrdefault{$n}\n";
        $an->{$kn}->{$n} = $genattrdefault{$n};
    }
    for my $q (keys %{ $kv }) {
print " kn $kn q $q ; $kv->{$q}\n";
        $an->{$kn}->{$q} = $kv->{$q};
    }
}

sub printdict {
    my($dn) = @_;
    my ($k, $fk);
    my (%v);

#   print "dn $dn\n";
    foreach $k (sort keys %{$dn}) {
#       print "k |$k|\n";
        print sprintf("k %s\n", $k);
        %v = %{${$dn}{$k}};
        foreach $fk (sort keys %v) {
#           print "  fk |$fk| fv |$v{$fk}|\n";
            print sprintf("    %-12s = %s\n", $fk, $v{$fk});
        }
    }
}

sub printdictX {
    my($dn,$spfk) = @_;
    my ($k, $fk);
    my (%v);
    my ($vv);

    foreach $k (keys %{$dn}) {
        print sprintf("k %s\n", $k);
        %v = %{${$dn}{$k}};
        foreach $fk (keys %v) {
            my $l;
            if($fk eq $spfk) {
#                $l = scalar ( @{ $v{$fk} } );
#                print sprintf("    %-8s = [", $fk);
#                print join(" ", @{ $v{$fk} } );
#                print "] $l \n";

                print sprintf("    %-8s = [", $fk);
                $l = 0;
                foreach $vv ( @{ $v{$fk} } ) {
                    print "$vv ";
                    $l++;
                }
                print "] $l \n";
            }
            else {
                print sprintf("    %-8s = %s\n", $fk, $v{$fk});
            }
        }
    }
}


my %classtemplate;
my $pfn = 'preload.ps';

my %oq;
my $oqu=1;

my %objx;
my %objy;
my %objz;
my %objllx;
my %objlly;
my %objurx;
my %objury;

my %objw;
my %objh;
my %objox;
my %objoy;

my %objlabel;
my %objclass;
my %objvis;

my %g2sx;
my %g2sy;
my %g2ex;
my %g2ey;

#my $xpitch = 0.6;
#my $ypitch = 0.3;
my $xpitch = 8;
my $ypitch = 6; 

#
#guideline G1 (0,1)-(3,1)
#guideline G2 (0,0)-(3,0)
#put AC1,AS1,D1 on G1
#put AC2,AS2,D2 on G2
#setx AC1 0
#sety AC2 2
#

my %g2o;
my @glist;
my @olist;
my %gdict;
my %odict;
my %pos;
my %postype;
my %rel;
my %reltype;

my %complist;

my $back_str='';
my $fore_str='';
my $over_str='';
my $super_str='';

sub expand_masks {
    my($kv) = @_;
    my($x,$y);
    my(@ar);
    
    @ar = ();
    if(!defined $kv->{masks}) {
        for($y=0;$y<$kv->{h};$y++) {
            for($x=0;$x<$kv->{w};$x++) {
                push(@{ $kv->{masks} }, 1);
            }
        }
    }
#   print "AR ".$kv->{masks}."\n";
}

#
# x,y,name
#
sub isinbbox {
    my($x,$y,$o) = @_;
    if($x>=$objllx{$o} && $x<=$objurx{$o}) {
        if($y>=$objlly{$o} && $y<=$objury{$o}) {
            return 1;
        }
    }
    return 0;
}

#
# x,y,llx,lly,urx,ury
#
sub isinrect {
    my($x,$y,$llx,$lly,$urx,$ury) = @_;
    if($x>=$llx && $x<=$urx) {
        if($y>=$lly && $y<=$ury) {
            return 1;
        }
    }
    return 0;
}

sub loadbuiltinclass {
    my %foo;

    print "LOADBUILTINCLASSES\n";

    %foo = (
        ox => 0,
        oy => 0,
        w  => 0,
        h  => 0,
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'ghost', \%foo);

    %foo = (
        ox => 2,
        oy => 1,
        w  => 4,
        h  => 2,
        psfunc => "tangybox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'vhalfbox', \%foo);

    %foo = (
        ox => 1,
        oy => 2,
        w  => 2,
        h  => 4,
        psfunc => "tangybox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'hhalfbox', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangybox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'box', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangyrbox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'rbox', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangyxbox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'xbox', \%foo);

    %foo = (
        ox => 2,
        oy => 1,
        w  => 4,
        h  => 2,
        psfunc => "tangylabelbox",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'labelbox', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangydiamond",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'diamond', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangycircle",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'circle', \%foo);

    %foo = (
        ox => 4,
        oy => 3,
        w  => 8,
        h  => 6,
        psfunc => "tangycloud",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'cloud', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangydrum",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'drum', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangypipe",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'pipe', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangywpipe",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'wpipe', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangyhcdots",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'hcdots', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangyvcdots",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'vcdots', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 5,
        psfunc => "tangyswitch",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'switch', \%foo);

    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 5,
        psfunc => "tangyrouter",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'router', \%foo);


    %foo = (
        ox => 2,
        oy => 2,
        w  => 4,
        h  => 4,
        psfunc => "tangyfile",
    );
    &expand_masks(\%foo);
    &insert_mtoa(\%classtemplate, 'file', \%foo);
    
}

sub loadbuiltins {
    print "LOADBUILTINS\n";
    &loadbuiltinclass;

#   &printdictX(\%classtemplate, 'masks');
}



sub directpos {
    my($on) = @_;
    my $gx;
    my $gy;
    if($objx{$on} ne '' && $objy{$on} ne '') {
        $gx = $objx{$on};
        $gy = $objy{$on};
        $oq{$gx.",".$gy} = $on;
        print "__\t$gx\t$gy\t$on\n";
    }
    else {
        print "why call directpos\n";
    }
}

sub tryputpack {
    my($mg,$x,$y,$n,$ign,$tn,$xp,$yp) = @_;
    return &_tryputpack($mg,1,$x,$y,$n,$ign,$tn,100,$xp,$yp);
}

sub tryputforce {
    my($mg,$x,$y,$n,$ign,$tn,$xp,$yp) = @_;
    return &_tryputpack($mg,0,$x,$y,$n,$ign,$tn,0,$xp,$yp);
}

sub _tryputpack {
    my($mg,$dodupchk,$x,$y,$n,$ign,$tn,$ntry,$xp,$yp) = @_;
    my($gx,$gy);
    my($llx,$lly,$urx,$ury);
    my($o,$k);
    my $c;
    my $cq;
    my $p;
    my $r;
    my $ii;
    my $spn = '';

    $r = -1;
    
print "tryputpack $mg $x $y $n $ign $tn $xp $yp     ; $objclass{$n}\n";

    if(defined $objvis{$n}) {
        if($objvis{$n}<0) {
            $spn = '!';
        }
    }

 if(0) {
    if($objclass{$n} =~ /[hv]?guideline/) {
            while($oq{$x.",".$y} ne '' &&
                !($objclass{$oq{$x.",".$y}} =~ /[hv]?guideline/) ) {
                $x++;
            }
            $objx{$n} = $x;
            $objy{$n} = $y;
            $oq{$x.",".$y} = $n;
            print "SOLVE $n\t,,$mg\t$x\t$y\n";
    }
    else {
    }
 }
      $ii = 0;
      do {
        print "  try $n [$ii] $x $y; $tn? $xp,$yp\n";

        if($objclass{$n} =~ /[hv]?guideline/) {
            $llx = $x;
            $lly = $y;
            $urx = $x;
            $ury = $y;
        }
        else {
            $llx = $x-2;
            $lly = $y-2;
            $urx = $x+2;
            $ury = $y+2;
        }
        print "llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";

 if(1)
 {
        print "TEMP\n";
        $llx = 0;
        $lly = 0;
        $urx = $objw{$n};
        $ury = $objh{$n};
#       print "raw  TEMP llx,lly,urx,ury = $llx,$lly,$urx,$ury for $n\n";
        $llx -= $objox{$n} - $x;
        $urx -= $objox{$n} - $x;
        $lly -= $objoy{$n} - $y;
        $ury -= $objoy{$n} - $y;
        print "cook TEMP llx,lly,urx,ury = $llx,$lly,$urx,$ury for $n\n";

        $objllx{$n} = $llx;
        $objlly{$n} = $lly;
        $objurx{$n} = $urx;
        $objury{$n} = $ury;
 }

 if(0)
 {
    if(defined $classtemplate{$objclass{$n}}) {
        print "TEMP\n";
        $llx = 0;
        $lly = 0;
        $urx = $classtemplate{$objclass{$n}}{w};
        $ury = $classtemplate{$objclass{$n}}{h};
        print "raw  llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";
        $llx -= $classtemplate{$objclass{$n}}{ox} - $x;
        $urx -= $classtemplate{$objclass{$n}}{ox} - $x;
        $lly -= $classtemplate{$objclass{$n}}{oy} - $y;
        $ury -= $classtemplate{$objclass{$n}}{oy} - $y;
        print "cook llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";

        $objllx{$n} = $llx;
        $objlly{$n} = $lly;
        $objurx{$n} = $urx;
        $objury{$n} = $ury;
    }
 }

        $c = 0;
        if($dodupchk) {
            for($gy=$lly;$gy<=$ury;$gy++) {
                for($gx=$llx;$gx<=$urx;$gx++) {
                    $p = $oq{$gx.",".$gy};
                    if($p ne '') {
#                       print "\t\t$gx $gy -> $p\n"; 
                        if($p ne $ign) {
                            $c++;
                        }
                    }
                }
            }
        }

        print "hit $c points\n";

        if($c==0) {
            $objx{$n} = $x;
            $objy{$n} = $y;
            print "SOLVE $n\t::$mg\t$x\t$y\n";

            if($lly==$ury && $llx==$urx) {
            }
            else {
                for($gy=$lly;$gy<=$ury;$gy++) {
                    for($gx=$llx;$gx<=$urx;$gx++) {
                        $oq{$gx.",".$gy} = $spn . $n;
                    }
                }
            }

 if(1) {

    if($objclass{$n} eq 'hguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $gx + ($#om+1)*$xpitch;
        $g2ey{$n} = $g2sy{$n};
print "GG2 h\n";
    }
    elsif($objclass{$n} eq 'vguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $g2sx{$n};
        $g2ey{$n} = $gy + ($#om+1)*$ypitch;
print "GG2 v\n";
    }
  }

            if($lly==$ury && $llx==$urx) {
            }
            else {
                $oq{$x.",".$y} = $spn . $n . "*";
            }
            $r = 0;
            last;
        }

        if($tn==0) {
            last;
        }
        $ii++;
        if($ii>$ntry) {
            print "ERROR give-up\n";
            last;
        }
        
        if($xp!=0) { $x += $xp; print "shift hori\n";}
        if($yp!=0) { $y += $yp; print "shift vert\n";}
      } while($c>0);

    if(0) {
    }

    print "ret $r\n";
    return $r;
}

sub Xtryputforce {
    my($mg,$x,$y,$n,$ign,$tn,$xp,$yp) = @_;
    my($gx,$gy);
    my($llx,$lly,$urx,$ury);
    my($o,$k);
    my $c;
    my $cq;
    my $p;
    my $r;
    my $ii;
    my $spn = '';

    $r = -1;
    
print "tryputforce $mg $x $y $n $ign $tn $xp $yp     ; $objclass{$n}\n";

    if(defined $objvis{$n}) {
        if($objvis{$n}<0) {
            $spn = '!';
        }
    }

      $ii = 0;
      do {
        print "  try $x $y; $tn? $xp,$yp [$ii]\n";

        if($objclass{$n} =~ /[hv]?guideline/) {
            $llx = $x;
            $lly = $y;
            $urx = $x;
            $ury = $y;
        }
        else {
            $llx = $x-2;
            $lly = $y-2;
            $urx = $x+2;
            $ury = $y+2;
        }
        print "llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";

 {
    if(defined $classtemplate{$objclass{$n}}) {
        print "TEMP\n";
        $llx = 0;
        $lly = 0;
        $urx = $classtemplate{$objclass{$n}}{w};
        $ury = $classtemplate{$objclass{$n}}{h};
        print "raw  llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";
        $llx -= $classtemplate{$objclass{$n}}{ox} - $x;
        $urx -= $classtemplate{$objclass{$n}}{ox} - $x;
        $lly -= $classtemplate{$objclass{$n}}{oy} - $y;
        $ury -= $classtemplate{$objclass{$n}}{oy} - $y;
        print "cook llx,lly,urx,ury = $llx,$lly,$urx,$ury $n\n";

        $objllx{$n} = $llx;
        $objlly{$n} = $lly;
        $objurx{$n} = $urx;
        $objury{$n} = $ury;
    }
 }

        $c = 0;
        for($gy=$lly;$gy<=$ury;$gy++) {
            for($gx=$llx;$gx<=$urx;$gx++) {
                $p = $oq{$gx.",".$gy};
                if($p ne '') {
#                   print "\t\t$gx $gy -> $p\n"; 
                    if($p ne $ign) {
                        $c++;
                    }
                }
            }
        }

        print "hit $c points\n";

        if($c==0) {
        }
        # it does not care override
        {
            $objx{$n} = $x;
            $objy{$n} = $y;
            print "SOLVE $n\t::$mg\t$x\t$y\n";

            if($lly==$ury && $llx==$urx) {
            }
            else {
                for($gy=$lly;$gy<=$ury;$gy++) {
                    for($gx=$llx;$gx<=$urx;$gx++) {
                        $oq{$gx.",".$gy} = $spn . $n;
                    }
                }
            }

 if(1) {

    if($objclass{$n} eq 'hguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $gx + ($#om+1)*$xpitch;
        $g2ey{$n} = $g2sy{$n};
print "GG2 h\n";
    }
    elsif($objclass{$n} eq 'vguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $g2sx{$n};
        $g2ey{$n} = $gy + ($#om+1)*$ypitch;
print "GG2 v\n";
    }
  }

            if($lly==$ury && $llx==$urx) {
            }
            else {
                $oq{$x.",".$y} = $spn . $n . "*";
            }
            $r = 0;
            last;
        }

        if($tn==0) {
            last;
        }
        $ii++;
        if($ii>100) {
            last;
        }
        
        if($xp!=0) { $x += $xp; print "shift hori\n";}
        if($yp!=0) { $y += $yp; print "shift vert\n";}
      } while($c>0);

    if(0) {
    }

    print "ret $r\n";
    return $r;
}



my $e_seq=1;

sub _escannerstr {
    my($u) = @_;
    my $ne;
    my $ostr;
    my ($k,$j);

    if($u =~ /^e/) {
        if($u eq 'e') {
            $k = -1;
            for($j=$e_seq;$j<$e_seq+1000;$j++) {
                $ne = sprintf("_e%d", $j);
                if(!defined $odict{$ne}) {
                    $k = $j;
                    $odict{$ne}++;
                    last;
                }
            }
            if($k>=0) {
                $e_seq = $k+1;
            }
print STDERR "_escannerstr: ne $ne\n";

            $objclass{$ne} = $defaultobjclass;
            $objvis{$ne} = -1;
            $ostr = $ne;
        }
        else {
            $objclass{$u} = $defaultobjclass;
            $objvis{$u} = -1;
            $ostr = $u;
        }
    }
    else {
        $ostr = $u;
    }

    return $ostr;
}

sub escannerar {
    my(@tmp) = @_;
    my @dst;
    my $u;
    my $ne;
    my $w;

    foreach $u (@tmp) {
#print " ar elem $u\n";
        $w = &_escannerstr($u);
        push(@dst, $w);
    }
#print " ar elem $u\n";
#print " return  ".(join("-",@dst))."\n";

    return @dst;
}

sub escannerstr {
    my($src,$sep) = @_;
#print " str |$src|\n";
    return &escannerar(split(/$sep/, $src));
}

#
# 3box,3circle -> box,box,box,circle,circle,circle
# 3A -> A01,A02,A03
#
# expand numeric members
#
sub numexpandM {
    my($line)=@_;
    my(@dst);
    my($e,$n,$b,$i);
    my $bb;
    @dst = ();
    foreach $e (split(/,/,$line)) {
        if($e =~/^(\d+)([^\d]*)/) {
            $n = $1;
            $b = $2;
            if($b =~ /^[A-Z_]+/) {
                for($i=0;$i<$n;$i++) {
                    $bb = sprintf("%s%d", $b, $e_seq);
                    $e_seq++;
                    $objclass{$bb} = $defaultobjclass;
                    push(@dst, $bb);
                }
            }
            else {
                for($i=0;$i<$n;$i++) {
                    push(@dst, $b);
                }
            }
        }
        else {
            push(@dst, $e);
        }
    }
    return @dst;
}

#
# 3box,3circle -> box,box,box,circle,circle,circle
#
# expand numeric members
#
sub numexpand {
    my($line,$sep)=@_;
    my(@dst);
    my($e,$n,$b,$i);
    @dst = ();
    foreach $e (split(/$sep/,$line)) {
        if($e =~/^(\d+)([^\d]*)/) {
            $n = $1;
            $b = $2;
            for($i=0;$i<$n;$i++) {
                push(@dst, $b);
            }
        }
        else {
            push(@dst, $e);
        }
    }
    return @dst;
}

sub gentempname {
    my($b) = @_;
    my($j,$k);
    my($tn);

    $k = -1;
    for($j=1;$j<1000;$j++) {
        $tn = sprintf("%s%d", $b, $j);
        if(!defined $odict{$tn}) {
print "FOUND $tn : $b\n";
            $k = $j;
            $odict{$tn}++;
            last;
        }
    }

    if($k<0) {
        return "";
    }

    return $tn;
}

sub metaidlistexpand{
    my($istr) = @_;
    my($sep) = ",";
    my($mstr);
    my @mlist;
    my @olist;
    my @solist;
    my $u;
    my $s;
    my ($n,$b,$i);
    my $k;
    my $tn;
    my $j;
print STDERR "istr   $istr\n";

    @mlist = split(/,/, $istr);
    $mstr = join($sep, @mlist);
#print STDERR "mstr   $mstr\n";
    @olist = ();
    foreach $u (@mlist) {
#       print "\t$u\n";
        if($u eq '') {
            # skip
        }
        elsif($u =~ /\//) {
#           print "\t---\n";
            @solist=();
            foreach $s (split(/\//, $u)) {
                if($s eq '') {
                    # skip
                }
                elsif($s =~ /^e/) {
                    $tn = &_escannerstr($s);
                    push(@solist, $tn);
                }
                elsif($s =~ /^(\d+)([^\d]*)/) {
                    $n = $1;
                    $b = $2;
#                   print "\t\t\t$n\t$b\n";
                    for($i=0;$i<$n;$i++) {  
                        if($b =~ /^e/) {
                            $tn = &_escannerstr($b);
                            if($tn ne '') {
                                push(@solist, $tn);
                            }
                        }
                        else {
                            $tn = &gentempname($b);
                            if($tn ne '') {
                                push(@solist, $tn);
                            }
                        }
                    }
                }
                else {
                    push(@solist, $s);
                }
            }
            push(@olist, join("/", @solist));
        }
        elsif($u =~ /^e/) {
            $tn = &_escannerstr($u);
            push(@olist, $tn);
        }
        else {
            if($u =~ /^(\d+)([^\d]*)/) {
                $n = $1;
                $b = $2;
#               print "\t\t$n\t$b\n";
                for($i=0;$i<$n;$i++) {  
                    if($b =~ /^e/) {
                        $tn = &_escannerstr($b);
                        if($tn ne '') {
                            push(@olist, $tn);
                        }
                    }
                    else {
                        $tn = &gentempname($b);
                        if($tn ne '') {
                            push(@olist, $tn);
                        }
                    }
                }
            }
            else {
                push(@olist, $u);
            }
        }
    }

print STDERR "olist  ".(join($sep,@olist))."\n";
    return @olist;
}

sub idlistexpand{
    my($istr, $sep) = @_;
    my($mstr);
    my @mlist;
    my @olist;
    @mlist = &numexpandM($istr,$sep);
print STDERR "mlist  ".(join($sep,@mlist))."\n";
#    $mstr = join($sep, @mlist);
#    @olist = &escannerstr($mstr,$sep);
    @olist = &escannerar(@mlist);
print STDERR "idlist ".(join($sep,@olist))."\n";
    return @olist;
}

sub onlinepos {
#    my($on) = @_;
    my($on, $gn) = @_;
    my $x;
    my $y;
    my $k;
    my $o;
    my $gx;
    my $gy;
    my $ik;

   print "onlinepos: $on $gn\n";

#    my $gn;
#    $gn = $pos{$on};
#    if($gn eq '') {
#        return;
#    }
#    if($postype{$on} ne 'onguideline') {
#        return;
#    }

    print " gn $gn\n";
    print " $g2sx{$gn} $g2sy{$gn} -> $g2ex{$gn} $g2ey{$gn}\n";

    $k = ($g2ey{$gn}-$g2sy{$gn})/($g2ex{$gn}-$g2sx{$gn});
#   print "k $k\n";

    if($objx{$on} eq '' && $objy{$on} eq '') {
        for($x = $g2sx{$gn}; $x<=$g2ex{$gn}; $x+=$xpitch) {
            $y  = $k * $x + $g2sy{$gn};
            $gx = $x;
            $gy = $y;
 if(0) {
            $o = $oq{$gx.",".$gy};
            if($o eq '' || $o eq $gn) {
                $objx{$on} = $gx;
                $objy{$on} = $gy;
                $oq{$gx.",".$gy} = $on;
                print "SOLVE $on\txy\t$gx\t$gy\n";
                last;
            }
 }

            $ik = &tryputpack("xy",$gx,$gy,$on,$gn, 0, 0, 0);
            if($ik==0) {
                last;
            }

        }
    }
    elsif($objy{$on} eq '') {
            $x = $objx{$on};
            $y  = $k * $x + $g2sy{$gn};
            $gx = $x;
            $gy = $y;
 if(0) {
            while($oq{$gx.",".$gy} ne '' &&
                !($objclass{$oq{$gx.",".$gy}} =~ /[hv]?guideline/) ) {
                $gx++;
            }
            $objx{$on} = $gx;
            $objy{$on} = $gy;
            $oq{$gx.",".$gy} = $on;
            print "SOLVE $on\t_y\t$gx\t$gy\n";
 }
            $ik = &tryputpack("y_",$gx,$gy,$on,$gn, 1, 0, $ypitch);

    }
    elsif($objx{$on} eq '') {
            print "SOLVE $on\tx_\t$x\t$gx\t$y\t$gy\n";
    }
    else {
            print "__\n";
    }

}

sub listoq {
    my $x;
    foreach $x (keys %oq) {
        print ":: $x $oq{$x}\n";
    }
}

sub scanbb_byobj {
    my ($minx,$miny,$maxx,$maxy)=(9999,9999,-9999,-9999);
    my $n;

    foreach $n (@olist) {
        print "$n $objx{$n} $objy{$n}\n";
        if($objx{$n}<$minx) { $minx = $objx{$n}; }
        if($objy{$n}<$miny) { $miny = $objy{$n}; }
        if($objx{$n}>$maxx) { $maxx = $objx{$n}; }
        if($objy{$n}>$maxy) { $maxy = $objy{$n}; }
    }

    return ($minx,$miny,$maxx,$maxy);
}

sub scanbb_byoq {
    my ($refoq) = @_;
    my ($minx,$miny,$maxx,$maxy)=(9999,9999,-9999,-9999);
    my $p;
    my ($x, $y);

    foreach $p (keys %{ $refoq } ) {
        if($refoq->{$p} eq '') {
            next;
        }
        ($x,$y) = split(/,/,$p);
        if($x<$minx) { $minx = $x; }
        if($x>$maxx) { $maxx = $x; }
        if($y<$miny) { $miny = $y; }
        if($y>$maxy) { $maxy = $y; }
    }

    return ($minx,$miny,$maxx,$maxy);
}


sub mapoq {
    my($refoq) = @_;
    my $x;
    my $y;
    my $p;
    my $gx;
    my $gy;
    my $n;
    my $nw;
    my $maxnw;
    
#    my ($minx,$miny,$maxx,$maxy)=&scanbb_byoq;
    my ($minx,$miny,$maxx,$maxy)=&scanbb_byoq($refoq);
    print "min $minx,$miny max $maxx,$maxy\n";

    $maxnw = 0;
    foreach $n (keys %{ $refoq } ) {
        $nw = length($refoq->{$n});
        if($nw>$maxnw) { $maxnw = $nw; }
    }
#   print "maxnw $maxnw\n";
    if($maxnw<2) {
        $maxnw = 2;
        print "maxnw $maxnw\n";
    }

    print "= = =\n";

        print "\t:";
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
#           print sprintf("%4s ",$gx);
            print sprintf("%*s ", $maxnw, $x);
        }
        print "\n";

        print "\t:";
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
#            print "$gx\t";
            print sprintf("%*s ", $maxnw, "..");
        }
        print "\n";

    for($y=$maxy;$y>=$miny;$y--) {
        $gy = $y;
#        print "$gy\t:";
        print sprintf("%4s \t",$y);
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
            $p = $refoq->{$x.",".$y};
#            print "$p\t";
            print sprintf("%*s ", $maxnw, $p);
        }
        print "\n";
    }
    print "= = =\n";
}

sub Xmapoq {
    my $x;
    my $y;
    my $p;
    my $gx;
    my $gy;
    my $n;


 if(0) {
    my ($minx,$miny,$maxx,$maxy)=(9999,9999,-9999,-9999);
    foreach $n (@olist) {
        print "$n $objx{$n} $objy{$n}\n";
        if($objx{$n}<$minx) { $minx = $objx{$n}; }
        if($objy{$n}<$miny) { $miny = $objy{$n}; }
        if($objx{$n}>$maxx) { $maxx = $objx{$n}; }
        if($objy{$n}>$maxy) { $maxy = $objy{$n}; }
    }
 }

 if(0) {
    my ($minx,$miny,$maxx,$maxy)=(9999,9999,-9999,-9999);
    foreach $p (keys %oq) {
        if(!defined $oq{$p}) {
            next;
        }
        if($oq{$p} eq '') {
            next;
        }
        ($x,$y) = split(/,/,$p);
        if($x<$minx) { $minx = $x; }
        if($x>$maxx) { $maxx = $x; }
        if($y<$miny) { $miny = $y; }
        if($y>$maxy) { $maxy = $y; }
    }
 }

    
    my ($minx,$miny,$maxx,$maxy)=&scanbb_byoq(\%oq);

    print "min $minx,$miny max $maxx,$maxy\n";

 if(0) {
    print "X ";
    for($x=$minx;$x<=$maxx;$x++) {
        print "($x) ";
        $gx = $x;
        print "$gx ";
    }
    print "\n";
 }
 if(0) {
    print "Y ";
    for($y=$maxy;$y>=$miny;$y--) {
        print "($y) ";
        $gy = $y;
        print "$gy ";
    }
    print "\n";
 }

    print "= = =\n";

        print "\t:";
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
#            print "$gx\t";
            print sprintf("%4s ",$gx);
        }
        print "\n";

        print "\t:";
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
#            print "$gx\t";
            print sprintf("%4s ","..");
        }
        print "\n";

    for($y=$maxy;$y>=$miny;$y--) {
        $gy = $y;
#        print "$y $gy\t:";
        print "$gy\t:";
        for($x=$minx;$x<=$maxx;$x++) {
            $gx = $x;
            $p = $oq{$x.",".$y};
#            print "$p\t";
            print sprintf("%4s ",$p);
        }
        print "\n";
    }
    print "= = =\n";
}


#           $escapedname = $n;
#           $escapedname =~ s#(\))#\\25#g;
#print STDERR "n  $n\n";
#print STDERR "n' $escapedname\n";

sub encpslabel {
    my($src) = @_;
    my($tmp) = $src;
    $tmp =~ s#(\()#\\050#g;
    $tmp =~ s#(\))#\\051#g;
#print STDERR "src  $src\n";
#print STDERR "dst  $tmp\n";
    return $tmp;
}


sub roughdraw {
    my ($ofn, $reflen, $margin) = @_;
    my $x;
    my $y;
    my $p;
    my $gx;
    my $gy;
    my $n;
    my $sca;
    my $scax;
    my $scay;
    my $qqw;
    my $qqh;
    my $gx;
    my $gy;
    my $acw;
    my $ach;
    my $ow;
    my $oh;

    my $u;

    $u = 1;

#    my ($minx,$miny,$maxx,$maxy)=&scanbb_byobj;
#    print "min $minx,$miny max $maxx,$maxy\n";
    my ($minx,$miny,$maxx,$maxy)=&scanbb_byoq(\%oq);
    print "min $minx,$miny max $maxx,$maxy\n";

    $qqw = $maxx-$minx;
    $qqh = $maxy-$miny;

    $acw = ($reflen-$margin*2);
    $ach = ($reflen-$margin*2);
    $scax = $acw/$qqw;
    $scay = $ach/$qqh;

    print "qqw x qqh $qqw x $qqh; acw x ach $acw x $ach\n";

    if($scax > $scay) { $sca = $scay; } else { $sca = $scax; }

    print "sca $sca [scax $scax, scay $scay]\n";

    $ow = $margin*2+$qqw*$sca;
    $oh = $margin*2+$qqh*$sca;
    if($scax > $scay) { 
        $ow = int($ow)+1;
        $oh = int($oh);
    }
    else {
        $ow = int($ow);
        $oh = int($oh)+1;
    }
    
    print "ow $ow oh $oh\n";

    open(F, ">$ofn");

    print F "%!PS-Adobe EPSF-1.0\n";
    print F "%%BoundingBox: 0 0 $ow $oh\n";

    if(-e $pfn) {
        print F "%\n% include preload file '$pfn'\n%\n";
        open(P, "<$pfn");
        while(<P>) {
            if(/^%%EOF/) {
                last;
            }
            print F $_;
        }
        close(P);
        print F "%\n% include end\n%\n";
    }
    else {
        print F "% not found preload file '$pfn'\n";
    }

###
### debug with translate
###
    if($outputgrid) {
        print F "%\n% draw grid for margins\n%\n";
        print F " 0 $margin moveto $ow 0 rlineto stroke\n";
        print F " 0 $oh $margin sub moveto $ow 0 rlineto stroke\n";
        print F " $margin 0 moveto 0 $oh rlineto stroke\n";
        print F " $ow $margin sub 0 moveto 0 $oh rlineto stroke\n";
    }

    print F " $margin $margin translate\n";

###
### grids
###

    # sheet aware grid
    if(0*$outputgrid) {
        my $x;
        my $y;
        print F "%\n% draw grid\n%\n";
        print F "  gsave\n";
        print F "    0.7 setgray\n";
        for($x=0;$x<=$acw;$x+=100) {
            print F " $x 0 moveto 0 $ach rlineto stroke\n";
        }
        for($y=0;$y<=$ach;$y+=100) {
            print F " 0 $y moveto $acw 0 rlineto stroke\n";
        }
        print F "  grestore\n";
    }

    print F "% scale+translate\n";
    print F " $sca $sca scale\n";
#    print F " $minx $miny translate\n";
    print F " $minx neg $miny neg translate\n";

    # object aware grid
    if(1*$outputgrid) {
        my $x;
        my $y;
        my ($sx,$ex);
        my ($sy,$ey);
        my ($gw,$gh);

        if($minx<0) {
            $sx = (int($minx/5)-1)*5;
        }
        else {
            $sx = (int($minx/5)-0)*5;
        }
#       $ex = (int($maxx/5)+1)*5;
        $ex = int(($maxx+4)/5)*5;
        if($miny<0) {
            $sy = (int($miny/5)-1)*5;
        }
        else {
            $sy = (int($miny/5)-0)*5;
        }
#       $ey = (int($maxy/5)+1)*5;
        $ey = int(($maxy+4)/5)*5;
        $gw = $ex - $sx;
        $gh = $ey - $sy;

        print F "%\n% draw grid\n%\n";
#        print F "%         qqh $qqh\n";
#        print F "%         qqw $qqw\n";
        print F "%      minx $minx maxx $maxx \n";
        print F "%      sx $sx ex $ex -> gw $gw\n";
        print F "%      miny $miny maxy $maxy\n";
        print F "%      sy $sy ey $ey -> gh $gh\n";

        print F "  gsave\n";

        print F "    0.8 setgray\n";
        print F "    [0.2 0.2] 0 setdash\n";
        print F "    0.03 setlinewidth\n";
        for($x=$sx;$x<=$ex;$x+=1) {
            print F "    $x $sy moveto 0 $gh rlineto stroke\n";
        }
        for($y=$sy;$y<=$ey;$y+=1) {
            print F "    $sx $y moveto $gw 0 rlineto stroke\n";
        }

        print F "    0.8 setgray\n";
        print F "    [] 0 setdash\n";
        print F "    0.07 setlinewidth\n";
        for($x=$sx;$x<=$ex;$x+=5) {
            print F "    $x $sy moveto 0 $gh rlineto stroke\n";
        }
        for($y=$sy;$y<=$ey;$y+=5) {
            print F "    $sx $y moveto $gw 0 rlineto stroke\n";
        }

        print F "    0.1 setlinewidth\n";
        print F "    0.7 0.7 1.0 setrgbcolor\n";
        print F "    $sx $sy moveto $ex $sy lineto $ex $ey lineto $sx $ey lineto closepath stroke\n";
        print F "    1 0.7 0.7 setrgbcolor\n";
        print F "    $minx $miny moveto $maxx $miny lineto $maxx $maxy lineto $minx $maxy lineto closepath stroke\n";
        print F "  grestore\n";
    }

###
### guidelines
###
    if($outputguideline) {
        print F "%\n% draw guidelines\n%\n";

        print F " gsave\n";
        print F " 0.7 setgray\n";
        foreach $n (@olist) {
            if($objclass{$n} =~ /[hv]?guideline/) {
                print F "% $objclass{$n} $n\n";
                $gx = $g2sx{$n} ; $gy = $g2sy{$n};
                print F " $gx $gy moveto\n";
                $gx = $g2ex{$n} ; $gy = $g2ey{$n};
                print F " $gx $gy lineto stroke\n";
            }
        }
        print F " grestore\n";
    }

###
### background
###

    if($back_str ne '') {
        print F "%\n% background string start ---\n%\n";
 if(0) {
        print F "gsave\n";
        print F "  0.7 setgray\n";
        print F "  currentlinewidth 2 div setlinewidth\n";
        print F $back_str;
        print F "grestore\n";
 }
        print F "gsave\n";
#        print F "  $minx neg $miny neg translate\n";
        print F "  currentlinewidth 2 div setlinewidth\n";
        print F $back_str;
        print F "grestore\n";
        print F "%\n% background string end   ---\n%\n";
    }


###
### objects (foreground)
###

    print F "%\n% draw objects\n%\n";

    my $escapedname;

    foreach $n (@olist) {
        if($objclass{$n} =~ /[hv]?guideline/) {
            next;
        }

        my $pp;
        my $vislabel;
        if(defined $objlabel{$n}) {
            $vislabel = $objlabel{$n};
        }
        else {
            $vislabel = $n;
        }
        $escapedname = &encpslabel($vislabel);

        if(defined $classtemplate{$objclass{$n}}) {
            $pp = $classtemplate{$objclass{$n}};
        }
        else {
            $pp = $classtemplate{$defaultobjclass};
        }

        my($cx, $cy);
        my($gx, $gy);
        my($gw, $gh);
        my($ggr);

        $cx = $objx{$n};
        $cy = $objy{$n};
#        $gx = $objx{$n}-$pp->{ox};
#        $gy = $objy{$n}-$pp->{oy};
#        $gw = $pp->{w};
#        $gh = $pp->{h};
        $gx = $objx{$n}-$objox{$n};
        $gy = $objy{$n}-$objoy{$n};
        $gw = $objw{$n};
        $gh = $objh{$n};

        if($gw>$gh) { $ggr = $gh; } else { $ggr = $gw; }
#       $ggr = int($ggr / 8)+1;
        $ggr = $ggr / 8;

        print F "% $objclass{$n} $n\n";

        if($outputbb) {
            print F "  gsave\n";
            print F "    ".(($gw+$gh)/2*.05)." setlinewidth\n";
            print F "      currentlinewidth 2 div setlinewidth\n";
            print F "    0.7 setgray\n";
            print F "    $gx $gy moveto\n";
            print F "    $gw 0 rlineto 0 $gh rlineto -$gw 0 rlineto 0 -$gh rlineto closepath stroke\n";

            print F "    gsave\n";
            print F "      $cx $cy moveto\n";
            print F "      $ggr $ggr rmoveto\n";
            print F "      $ggr 2 mul neg $ggr 2 mul neg rlineto\n";
            print F "      0 $ggr 2 mul rmoveto\n";
            print F "      $ggr 2 mul $ggr 2 mul neg rlineto\n";
            print F "      closepath stroke\n";
            print F "    grestore\n";


            print F "  grestore\n";
        }


        if(defined $objvis{$n}) {
            if($objvis{$n}<0) {
                next;
            }
        }


        if(defined $pp->{psfunc}) {
            print F "  gsave\n";
            print F "    /Helvetica findfont $u scalefont setfont\n";
            print F "    ".(($gw+$gh)/2*.05)." setlinewidth\n";
            print F "    $gx $gy $gw $gh ($escapedname) ".($pp->{psfunc})."\n";
            print F "  grestore\n";
        }

        if($outputexl*0) {
            print F "  gsave\n";
            print F "    /Helvetica findfont $u scalefont setfont\n";
            print F "    ".(($gw+$gh)/2*.05)." setlinewidth\n";
            print F "    $gx $gy $gh 1.2 mul add $gw ($escapedname) fixwidthshow\n";
            print F "  grestore\n";
        }
    }

###
### over 
###

    if($over_str ne '') {
        print F "%\n% over string start ---\n%\n";
 if(0) {
        print F "gsave\n";
        print F "  0.7 setgray\n";
        print F "  currentlinewidth 2 div setlinewidth\n";
        print F $over_str;
        print F "grestore\n";
 }
        print F "gsave\n";
#        print F "  $minx neg $miny neg translate\n";
        print F "  currentlinewidth 2 div setlinewidth\n";
        print F $over_str;
        print F "grestore\n";
        print F "%\n% over string end   ---\n%\n";
    }


        if($outputexl) {
    foreach $n (@olist) {
        if($objclass{$n} =~ /[hv]?guideline/) {
            next;
        }
        if(defined $objvis{$n}) {
            if($objvis{$n}<0) {
                next;
            }
        }

        $escapedname = &encpslabel($n);
        my $pp;

        if(defined $classtemplate{$objclass{$n}}) {
            $pp = $classtemplate{$objclass{$n}};
        }
        else {
            $pp = $classtemplate{$defaultobjclass};
        }

        my($cx, $cy);
        my($gx, $gy);
        my($gw, $gh);
        my($ggr);

        $cx = $objx{$n};
        $cy = $objy{$n};
        $gx = $objx{$n}-$pp->{ox};
        $gy = $objy{$n}-$pp->{oy};
        $gw = $pp->{w};
        $gh = $pp->{h};

        if($gw>$gh) { $ggr = $gh; } else { $ggr = $gw; }
#       $ggr = int($ggr / 8)+1;
        $ggr = $ggr / 8;


        if($outputexl) {
            print F "  gsave\n";
            print F "    /Helvetica findfont $u scalefont setfont\n";
            print F "    ".(($gw+$gh)/2*.05)." setlinewidth\n";
            print F "    $gx $gy $gh 1.2 mul add $gw ($escapedname) whbgfixwidthshow\n";
            print F "  grestore\n";
        }
    }
        }


    print F "%%%\n%%% end of file\n%%%\n";

    close F;
}

my $_z = 0;
sub genobjid {
    $_z++;
    return "_".$_z;
}

my @optrequiredargs = ('thick', 'name', 'label', 'class', 'framecolor', 'fillcolor', 'delta', 'rad', 'width', 'height', 'offx', 'offy', 'heads', 'lastletter');
my %optmap;

{
    my $i;
    foreach $i (@optrequiredargs) {
        $optmap{$i} = 1;
    }
    foreach $i (keys %optmap) {
        print "optmap $i $optmap{$i}\n";
    }
}

sub applyargs {
    my($tn, $args) = @_;
    my $n;
    my $v;
    if(defined $args->{'class'}) {
        $objclass{$tn} = $args->{'class'};
    }
    if(defined $args->{'label'}) {
        $objlabel{$tn} = $args->{'label'};
    }
    if(defined $args->{'invis'}) {
        $objvis{$tn} = -1;
    }
    if(defined $args->{'width'}) {
        $objw{$tn} = $args->{'width'};
    }
    if(defined $args->{'height'}) {
        $objh{$tn} = $args->{'height'};
    }
    if(defined $args->{'offx'}) {
        $objox{$tn} = $args->{'offx'};
    }
    if(defined $args->{'offy'}) {
        $objoy{$tn} = $args->{'offy'};
    }
}

sub printargs {
    my($dst, $pre ) = @_;
    my $n;
    my $v;
    foreach $n (keys %{ $dst }) {
        $v = $dst->{$n};
        print "$pre : :: $n = $v\n";
    }
}

sub storeargs {
    my($dst, $src, $startfield) = @_;
    my $u;
    my $i;
    my @mid;
    my $n;
    my $v;
    print "PARGS : ";
    $i = 0;
    for $u ( @{ $src } ) {
        print "$u ";
        if($i>=$startfield) {
            push(@mid, $u);
        }
        $i++;
    }
    print "\n";

    print "PARGS : mid ".(join("|", @mid))."\n";
    while(@mid) {
        $n = shift(@mid);
        if(defined $optmap{$n}) {
            $v = shift(@mid);
            $dst->{$n} = $v;
        }
        else {
            $dst->{$n} = 'on';
        }
    }
}

# 0   1         2       3
# put <objname> hslolts <params>
sub put_slots {
    my($objid, $ori, @f) = @_;
    my %pargs;
    &storeargs(\%pargs, \@f, 3);
    &printargs(\%pargs, "PARGS");
    my $nh;
    my $lastletter;
    my $last;
    my $i;
    my $n;
    my $rel;

    if($ori eq 'h') {
        $rel = "rightnear";
    }
    elsif($ori eq 'v') {
        $rel = "downnear";
    }

    $nh = 3;
    if(defined $pargs{heads}) {
        $nh = $pargs{heads};
    }
#   $nh = 3;

    $lastletter = 'N';
    if(defined $pargs{lastletter}) {
        $lastletter = $pargs{lastletter};
    }
#   $lastletter = 'N';
    
    $last = '';
    for($i=1;$i<=$nh;$i++) {
            $n = sprintf("%s%d", $f[1], $i);
            $odict{$n}++;
            if($last ne '') {
                $pos{$n} .= "$last\@$rel";
            }
            $last = $n;
    }

            $n = sprintf("_%s_d", $f[1]);
            $odict{$n}++;
            if($ori eq 'h') {
                $objclass{$n} = 'hcdots';
            }
            elsif($ori eq 'v') {
                $objclass{$n} = 'vcdots';
            }

            if($last ne '') {
                $pos{$n} .= "$last\@$rel";
            }
            $last = $n;

            $n = sprintf("%s%s", $f[1], $lastletter);
            $odict{$n}++;
            if($last ne '') {
                $pos{$n} .= "$last\@$rel|";
            }

}

sub put_hslots {
    my($objid, @f) = @_;
    return put_slots($objid,'h',@f);
}

sub put_vslots {
    my($objid, @f) = @_;
    return put_slots($objid,'v',@f);
}

sub put_xhori {
    my($objid, @f) = @_;
    my %pargs;
    my @tail;
    my $first;
    my $last;
    my $i;
    my $valign = "c";
    foreach $i (@f) {
        if($i eq 'top')     { $valign = "t"; }
        if($i eq 'bottom')  { $valign = "b"; }
    }

    @tail = &metaidlistexpand($f[1]);
    &storeargs(\%pargs, \@f, 2);
    &printargs(\%pargs, "PARGS");

    $i = 0;
    while($#tail>=0) {
        $first = shift(@tail);
        print "xhori - $first\n";

        my @stail;
        my $second;
        my $sfirst;
        my $slast;
        my $j;
        $j = 0;
        @stail = split(/\//, $first);

        if($#stail==0) {
            $odict{$first}++;
            if($i==0) {
            }
            else {
                if(defined $pargs{near}) {
                    $pos{$first} .= "$last\@rightnear|";
                }
                else {
                    $pos{$first} .= "$last\@right|";
                }
            }
            $last = $first;
        }
        else {
            my $zero="$objid"."z"."$i";
            my $ogh=$#stail;
            my $oghh=int($ogh/2);
            my $hpad=$ogh%2;
print "ogh $ogh oghh $oghh ; hpad $hpad\n";
            my $snext;
            my $scenter;
            my $dmyhead="$objid"."h";

            if($last eq '') {
                $odict{$dmyhead}++;
                $objclass{$dmyhead} = 'ghost';
                $last = $dmyhead;
            }

            if($valign eq 'c' && $hpad) {
                $odict{$zero}++;
                if(defined $pargs{near}) {
                    $pos{$zero} .= "$last\@rightnear|";
                }
                else {
                    $pos{$zero} .= "$last\@right|";
                }
                $objclass{$zero} = "ghost";
            }

            $sfirst = '';

            $j = 0;
            while($#stail>=0) {
                $second = shift(@stail);
                if($sfirst eq '') {
                    $sfirst = $second;
                }
                $snext = $stail[0];
                $odict{$second}++;
                if($valign eq 'c') {
                    if($j==$oghh) {
                        if($hpad) {
                            if(defined $pargs{near}) {
                                $pos{$second} .= "$last\@righthalfupnear!|";
                            }
                            else {
                                $pos{$second} .= "$last\@righthalfup!|";
                            }
                        }
                        else {
                            if(defined $pargs{near}) {
                                $pos{$second} .= "$last\@rightnear|";
                            }
                            else {
                                $pos{$second} .= "$last\@right|";
                            }
                            $scenter = $second;
                        }
                    }
                    elsif($j<$oghh) {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$snext\@upnear|";
                        }
                        else {
                            $pos{$second} .= "$snext\@up|";
                        }
                    }
                    else {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$slast\@downnear|";
                        }
                        else {
                            $pos{$second} .= "$slast\@down|";
                        }
                    }
                }
                elsif($valign eq 'b') {
                    if($snext eq '') {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$last\@rightnear|";
                        }
                        else {
                            $pos{$second} .= "$last\@right|";
                        }
                    }
                    else {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$snext\@upnear|";
                        }
                        else {
                            $pos{$second} .= "$snext\@up|";
                        }
                    }
                }
                elsif($valign eq 't') {
                    if($slast eq '') {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$last\@rightnear|";
                        }
                        else {
                            $pos{$second} .= "$last\@right|";
                        }
                    }
                    else {
                        if(defined $pargs{near}) {
                            $pos{$second} .= "$slast\@downnear|";
                        }
                        else {
                            $pos{$second} .= "$slast\@down|";
                        }
                    }
                }

                $slast = $second;
                $j++;
            }
            if($valign eq 'c') {
                if($hpad) {
                    $last = $zero;
                }
                else {
                    $last = $scenter;
                }
            }
            elsif($valign eq 'b') {
                $last = $slast;
            }
            elsif($valign eq 't') {
                $last = $sfirst;
            }
        }

        $i++;
    }
}


sub put_xvert {
    my($objid, @f) = @_;
    my %pargs;
    my @tail;
    my $first;
    my $last;
    my $i;
    my $halign = "c";
    foreach $i (@f) {
        if($i eq 'right')   { $halign = "r"; }
        if($i eq 'left')    { $halign = "l"; }
    }

    @tail = &metaidlistexpand($f[1]);
    &storeargs(\%pargs, \@f, 2);
    &printargs(\%pargs, "PARGS");

    $i = 0;
    while($#tail>=0) {
        $first = shift(@tail);
        print "xvert - $first\n";

        my @stail;
        my $second;
        my $sfirst;
        my $slast;
        my $j;
        $j = 0;
        @stail = split(/\//, $first);


        if($#stail==0) {
            $odict{$first}++;
            if($i==0) {
            }
            else {
                $pos{$first} .= "$last\@down|";
            }
            $last = $first;
        }
        else {
            my $zero="$objid"."z"."$i";
            my $ogh=$#stail;
            my $oghh=int($ogh/2);
            my $hpad=$ogh%2;
print "ogh $ogh oghh $oghh ; hpad $hpad\n";
            my $snext;
            my $scenter;
            my $dmyhead="$objid"."h";

            if($last eq '') {
                $odict{$dmyhead}++;
                $objclass{$dmyhead} = 'ghost';
                $last = $dmyhead;
            }

            if($halign eq 'c' && $hpad) {
                $odict{$zero}++;
                $pos{$zero} .= "$last\@down|";
                $objclass{$zero} = "ghost";
            }

            $sfirst = '';

            $j = 0;
            while($#stail>=0) {
                $second = shift(@stail);
                if($sfirst eq '') {
                    $sfirst = $second;
                }
                $snext = $stail[0];
                $odict{$second}++;
                if($halign eq 'c') {
                    if($j==$oghh) {
                        if($hpad) {
                            $pos{$second} .= "$last\@downhalfright!|";
                        }
                        else {
                            $pos{$second} .= "$last\@down|";
                            $scenter = $second;
                        }
                    }
                    elsif($j<$oghh) {
                        $pos{$second} .= "$snext\@right|";
                    }
                    else {
                        $pos{$second} .= "$slast\@left|";
                    }
                }
                elsif($halign eq 'l') {
                    if($snext eq '') {
                        $pos{$second} .= "$last\@down|";
                    }
                    else {
                        $pos{$second} .= "$snext\@right|";
                    }
                }
                elsif($halign eq 'r') {
                    if($slast eq '') {
                        $pos{$second} .= "$last\@down|";
                    }
                    else {
                        $pos{$second} .= "$slast\@left|";
                    }
                }

                $slast = $second;
                $j++;
            }
            if($halign eq 'c') {
                if($hpad) {
                    $last = $zero;
                }
                else {
                    $last = $scenter;
                }
            }
            elsif($halign eq 'l') {
                $last = $slast;
            }
            elsif($halign eq 'r') {
                $last = $sfirst;
            }
        }

        $i++;
    }
}


sub parse {
    my @f;
    my $objname;
    my $v;
    my $objid;

    print STDERR "read===\n";

    while(<>) {
        chomp;
        s/^\s+//g;
        if(/^#/) {
            next;
        }
        elsif(/^$/) {
            next;
        }
        print STDERR "; $_\n";

        if(/^quit/) {
            last;
        }

        $objid = &genobjid();

        @f = split;
        if(/^srand/) {
            if($f[1] eq '') {
                my $seed = time % $$;
                print "SEED $seed\n";
                srand($seed);
            }
            else {
                srand($f[1]);
            }
        }
        elsif(/^hpitch/) {
            $xpitch = $f[1];
        }
        elsif(/^vpitch/) {
            $ypitch = $f[1];
        }
        elsif(/^guideline\b/) {
            $objname = $f[1];
            $odict{$objname}++;
            $odict{$objname}++;
#           $objclass{$objname} = 'guideline';
            $objclass{$objname} = 'hguideline';
            if(/\bvert\b/) {
                $objclass{$objname} = 'vguideline';
            }
#print "G $objname\n";
            if(/\(([\d.]+),([\d.]+)\)-\(([\d.+]),([\d.+])\)/) {
#               print STDERR "$1 x $2 --> $3 x $4\n";
                $g2sx{$objname} = $1;
                $g2sy{$objname} = $2;
                $g2ex{$objname} = $3;
                $g2ey{$objname} = $4;
#                $gdict{$objname}++;
                $objclass{$objname} = 'guideline';
            }
        }
        elsif(/^put\b/) {
            if(/^put\s+\S+\s+on\s+\S+/) {
                my $last='';
                foreach $objname (split(/,/, $f[1])) {
                    $odict{$objname}++;
                    $pos{$objname} .= "$f[3]\@onguideline|";
                    push( @{ $g2o{$f[3]} }, $objname);

                    if(1) {
                        if($last ne '') {
                            $pos{$objname} .= "$last\@right|";
                        }
                        $last = $objname;
                    }
                }
            }
            elsif(/^put\s+\S+\s+(right|left|up|upnear|downnear|leftnear|rightnear|righthalfup!|downhalfright!|halfup|down|samex|samey|samexy)\s+(\S+)/) {
                my %pargs;
                &storeargs(\%pargs, \@f, 3);
                &printargs(\%pargs, "PARGS");
                foreach $objname (split(/,/, $f[1])) {
                    $odict{$objname}++;
                    $odict{$f[3]}++;
                    $pos{$objname} .= "$f[3]\@$f[2]|";
                }
            }
            elsif(/^put\s+\S+\s+rand\s*/) {
                foreach $objname (split(/,/, $f[1])) {
                    $odict{$objname}++;
                    $pos{$objname} .= "\@rand|";
                }
            }

            elsif(/^put\s+\S+\s+hslots\b/) {
                &put_hslots($objid, @f);
            }
            elsif(/^put\s+\S+\s+vslots\b/) {
                &put_vslots($objid, @f);
            }

            elsif(/^put\s+\S+\s+xhori\s*/) {
                &put_xhori($objid, @f);
            }

            elsif(/^put\s+\S+\s+xvert\s*/) {
                &put_xvert($objid, @f);
            }

            elsif(/^put\s+\S+\s+hori\s*/) {
                my @tail;
                my $first;
                my $last;
                my $donear;
#                @tail = split(/,/, $f[1]);
#                @tail = &idlistexpand($f[1], ",");
                @tail = &metaidlistexpand($f[1]);

                $donear = 0;
                if(/\bnear\b/) {
                    $donear = 1;
                }

                $first = shift(@tail);
#               print "hori = $first\n";
                $odict{$first}++;
                $last = $first;

                while($#tail>=0) {
                    $first = shift(@tail);
#                   print "hori - $first\n";
                    $odict{$first}++;
                    if($donear) {
                        $pos{$first} .= "$last\@rightnear|";
                    }
                    else {
                        $pos{$first} .= "$last\@right|";
                    }
                    $last = $first;
                }
            }

            elsif(/^put\s+\S+\s+vert\s*/) {
                my @tail;
                my $first;
                my $last;
                my $donear;
#                @tail = split(/,/, $f[1]);
#                @tail = &idlistexpand($f[1], ",");
                @tail = &metaidlistexpand($f[1]);

                $donear = 0;
                if(/\bnear\b/) {
                    $donear = 1;
                }

                $first = shift(@tail);
#               print "vert = $first\n";
                $odict{$first}++;
                $last = $first;

                while($#tail>=0) {
                    $first = shift(@tail);
#                   print "vert - $first\n";
                    $odict{$first}++;
                    if($donear) {
                        $pos{$first} .= "$last\@downnear|";
                    }
                    else {
                        $pos{$first} .= "$last\@down|";
                    }
                    $last = $first;
                }
            }

        }
        elsif(/^setx\b/) {
            $v = $f[2];
            $objx{$f[1]} = $v;
        }
        elsif(/^sety\b/) {
            $v = $f[2];
            $objy{$f[1]} = $v;
        }
        elsif(/^class\b/) {
            &setclass($f[1],$f[2]);
        }
        elsif(/^label/) {
            $objlabel{$f[1]} = $f[2];
        }
        elsif(/^option/) {
            my %pargs;
            &storeargs(\%pargs, \@f, 1);
            &printargs(\%pargs, "PARGS");
            if($f[1] =~ /^\//) {
                my $frule;
                my @filtered;
                $frule = substr($f[1], 1, length($f[1])-2);
                @filtered = grep /$frule/, keys %odict;
#               print "option target rule $frule\n";
#               print "option target filtered 2 odict ". (join(",", @filtered))."\n";

                foreach my $n (@filtered) {
                    print "option target $n\n";
                }
                foreach my $n (@filtered) {
                    if($n =~ /^_/) {
                        next;
                    }
                    &applyargs($n, \%pargs);
                }
            }
            elsif($f[1] eq 'all') {
                foreach my $n (keys %odict) {
                    if($n =~ /^_/) {
                        next;
                    }
                    &applyargs($n, \%pargs);
                }
            }
            else {
                foreach my $n (split(/,/,$f[1])) {
                    if(defined $odict{$n}) {
                        &applyargs($n, \%pargs);
                    }
                }
            }
        }
        elsif(/^link\b/) {
            &registcomp(@f);
        }
        elsif(/^gather\b/) {
            &registcomp(@f);
        }
        else {
            print " ERROR\n";
        }
    }
}


my %bkoq;
my %bkobjx;
my %bkobjy;

sub xyswap {
    my($p, $q);
    my($x,$y);

    %bkoq   = %oq;
    %bkobjx = %objx;
    %bkobjy = %objy;

    undef %oq;
    undef %objx;
    undef %objy;

    foreach $p (keys %bkoq) {
        ($x,$y) = split(/,/, $p);
        $q = $y.",".$x;
        $oq{$q} = $bkoq{$p};
    }

    foreach $p (keys %bkobjx) {
        $objy{$p} = $bkobjx{$p};
    }
    foreach $p (keys %bkobjx) {
        $objx{$p} = $bkobjy{$p};
    }
    
}


sub setclass {
    my($nlist, $xtype) = @_;
    my $n;
    foreach $n (split(/,/, $nlist)) {
        $objclass{$n} = $xtype;
    }
}


sub countup {
    my $x;
    my $y;

    print STDERR "countup===\n";
#    print STDERR "  object $#olist\n";

    @glist = keys %gdict;

    #
    # mark members of guideline to object-dictionary
    #
    foreach $x (@glist) {
#       print "G $x\n";
        foreach $y ( @{ $g2o{$x} } ) {
#            print "  ".$y;
            $odict{$y}++;
        }
#        print "\n";
    }

    @olist = keys %odict;

    print STDERR "  object ".($#olist+1)."\n";
}

sub bindclass {
    my $o;
    
    print STDERR "bindclass==\n";

    foreach $o (@olist) {
        print "o $o\n";
        if(!defined $objclass{$o}) {
            $objclass{$o} = $defaultobjclass;
        }

        if(defined $classtemplate{$objclass{$o}}) {
            if(!defined $objox{$o}) {
                $objox{$o} = $classtemplate{$objclass{$o}}{ox};
            }
            if(!defined $objoy{$o}) {
                $objoy{$o} = $classtemplate{$objclass{$o}}{oy};
            }
            if(!defined $objw{$o}) {
                $objw{$o} = $classtemplate{$objclass{$o}}{w};
            }
            if(!defined $objh{$o}) {
                $objh{$o} = $classtemplate{$objclass{$o}}{h};
            }
        }
    }
}

sub showrel {
    my $x;
    my $y;

    print STDERR "showrel===\n";

    print "- - -\n";
    foreach $x (keys %pos) {
        print "$x\t-> $pos{$x}   $postype{$x}\n";
    }
    print "- - -\n";
}

sub showloc {
    my $o;
    my $m;

    print STDERR "showloc===\n";

    printf "%8s : %3s %3s %3s %3s: %3s %3s %3s :_ %3s %3s %3s %3s\n",
        "name", "ox","oy", "w", "h", "x", "y", "z", "llx", "lly", "urx", "ury";
    foreach $o (keys %odict) {
        $m = '';
        if(!defined $objllx{$o}) {
            $m = '*';
        }
        printf "%8s : %3d %3d %3d %3d : %3d %3d %3d :%s %3d %3d %3d %3d\n",
            $o, $objox{$o}, $objoy{$o}, $objw{$o}, $objh{$o},
            $objx{$o}, $objy{$o}, $objz{$o}, $m,
            $objllx{$o}, $objlly{$o}, $objurx{$o}, $objury{$o};
    }
}


my $c_single=0;

sub singlesolve {
    my($n) = @_;
    my $gx;
    my $gy;

#   print STDERR "singlesolve n $n\n";
    if($c_single==0) {
        $gx = 0;
        $gy = 0;
    }
    else {
        $gx = int(rand(10));
        $gy = int(rand(10));
    }
    print STDERR "singlesolve n $n <$c_single> rand $gx,$gy\n";
    $c_single++;

 if(0) {

    if($objclass{$n} eq 'hguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $gx + ($#om+1)*$xpitch;
        $g2ey{$n} = $g2sy{$n};
print "GG1 h\n";
    }
    elsif($objclass{$n} eq 'vguideline') {
        my @om;
        @om = @{ $g2o{$n} };
        print "member ".($#om+1)."\n";
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        $g2ex{$n} = $g2sx{$n};
        $g2ey{$n} = $gy + ($#om+1)*$ypitch;
print "GG1 v\n";
    }
    else {
    }

    $objx{$n} = $gx;
    $objy{$n} = $gy;
    $oq{$gx.",".$gy} = $n;
    print "SOLVE $n\t**\t$gx\t$gy\n";

 }

    &tryputpack('**',$gx,$gy,$n,'dmy', 1, $xpitch, 0);

}

sub gl_arrange {
    my($n) = @_;
    my $gx;
    my $gy;
    $gx = $objx{$n};
    $gy = $objy{$n};
    my @om;
    @om = @{ $g2o{$n} };
    if($objclass{$n} =~ /[hv]?guideline/) {
        $g2sx{$n} = $gx;
        $g2sy{$n} = $gy;
        if($objclass{$n} eq 'hguideline') {
            $g2ex{$n} = $gx + ($#om+1)*$xpitch;
            $g2ey{$n} = $g2sy{$n};
        }
        elsif($objclass{$n} eq 'vguideline') {
            $g2ex{$n} = $g2sx{$n};
            $g2ey{$n} = $gy + ($#om+1)*$ypitch;
        }
        else {
        }
        $objx{$n} = $gx;
        $objy{$n} = $gy;
        $oq{$gx.",".$gy} = $n;
        print "SOLVE $n\t@@\t$gx\t$gy\n";
    }
    else {
        print "not support\n";
    }
}


sub solosolve {
    my($n,$dt) = @_;
print "solo: n $n dt $dt\n";
    if($dt eq 'rand') {
        my ($gx, $gy);
        $gx = int(rand(10));
        $gy = int(rand(10));
        &tryputpack('rr', $gx, $gy, $n, 'dmy', 1, $xpitch, $ypitch);
    }
    else {
        print "solo: not implemented type $dt\n";
        next;   
    }
}


sub relsolve {
    my($n,$d,$dt) = @_;
    my($gx,$gy);
    my $ik;
    print "name $n depend $d dependtype $dt\n";
    if($dt eq 'down') {
        if($objy{$n} ne '') {
            $gy = $objy{$n};
        }
        else {
            $gy = int($objy{$d} - $ypitch);
        }
        $gx = $objx{$d};
        $ik = &tryputpack('?d', $gx, $gy, $n, 'dmy', 1, 0, -$ypitch);
    }

    elsif($dt eq 'up') {
 {
    print "d $objllx{$d} $objlly{$d} $objurx{$d} $objury{$d}\n";
 }
        if($objy{$n} ne '') {
            $gy = $objy{$n};
        }
        else {
            $gy = int($objy{$d} + $ypitch);
        }
        $gx = $objx{$d};
        $ik = &tryputpack('?u', $gx, $gy, $n, 'dmy', 1, 0, $ypitch);
    }

    elsif($dt eq 'upnear') {
 {
#   print "n $n | bbox $objllx{$n} $objlly{$n} $objurx{$n} $objury{$n}\n";
    print "d $d | bbox $objllx{$d} $objlly{$d} $objurx{$d} $objury{$d}\n";
    print "n $n | w $objw{$n} h $objh{$n}\n";
 }
        if($objy{$n} ne '') {
            $gy = $objy{$n};
        }
        else {
            $gy = int($objury{$d} + ($objh{$n} - $objoy{$n}) + 0);
        }
        $gx = $objx{$d};
        $ik = &tryputforce('?uN', $gx, $gy, $n, 'dmy', 1, 0, 0);
    }

    elsif($dt eq 'downnear') {
 {
#   print "n $n | bbox $objllx{$n} $objlly{$n} $objurx{$n} $objury{$n}\n";
    print "d $d | bbox $objllx{$d} $objlly{$d} $objurx{$d} $objury{$d}\n";
    print "n $n | w $objw{$n} h $objh{$n}\n";
 }
        if($objy{$n} ne '') {
            $gy = $objy{$n};
        }
        else {
            $gy = int($objlly{$d} - $objoy{$d} - 0);
        }
        $gx = $objx{$d};
        $ik = &tryputforce('?uN', $gx, $gy, $n, 'dmy', 1, 0, 0);
    }

    elsif($dt eq 'righthalfup!') {
        $gy = int($objy{$d} + $ypitch/2);
        $gx = int($objx{$d} + $xpitch);
        $ik = &tryputforce('?rhu!', $gx, $gy, $n, 'dmy', 1, 0, $ypitch/2);
    }


    elsif($dt eq 'righthalfupnear!') {
#       $gx = int($objx{$d} + $xpitch);
        $gx = int($objurx{$d} + $objox{$n} + 0);
#       $gy = int($objy{$d} + $ypitch/2);
#       $gy = int($objury{$d} + ($objh{$n} - $objoy{$n}) + 0);
        $gy = int($objy{$d} + $objh{$n}/2);
        $ik = &tryputforce('?rhu!', $gx, $gy, $n, 'dmy', 1, 0, $ypitch/2);
    }

    elsif($dt eq 'halfup') {
        if($objy{$n} ne '') {
            $gy = $objy{$n};
        }
        else {
            $gy = int($objy{$d} + $ypitch/2);
        }
        $gx = $objx{$d};
        $ik = &tryputpack('?u', $gx, $gy, $n, 'dmy', 1, 0, $ypitch/2);
    }

    elsif($dt eq 'downhalfright!!') {
        $gy = int($objy{$d} + $ypitch);
        $gx = int($objx{$d} + $xpitch/2);
        $ik = &tryputforce('?rhu!', $gx, $gy, $n, 'dmy', 1, $xpitch/2, 0);
    }

    elsif($dt eq 'right') {
        if($objx{$n} ne '') {
            $gx = $objx{$n};
        }
        else {
            $gx = int($objx{$d} + $xpitch);
        }
        $gy = $objy{$d};
        $ik = &tryputpack('?r', $gx, $gy, $n, 'dmy', 1, $xpitch, 0);
    }
    elsif($dt eq 'left') {
        if($objx{$n} ne '') {
            $gx = $objx{$n};
        }
        else {
            $gx = int($objx{$d} - $xpitch);
        }
        $gy = $objy{$d};
        $ik = &tryputpack('?l', $gx, $gy, $n, 'dmy', 1, -$xpitch, 0);
    }

    elsif($dt eq 'leftnear') {
 {
#   print "n $n | bbox $objllx{$n} $objlly{$n} $objurx{$n} $objury{$n}\n";
    print "d $d | bbox $objllx{$d} $objlly{$d} $objurx{$d} $objury{$d}\n";
    print "n $n | w $objw{$n} h $objh{$n}\n";
 }
        if($objx{$n} ne '') {
            $gx = $objx{$n};
        }
        else {
            $gx = int($objllx{$d} - ($objw{$n} - $objox{$n}) - 0);
        }
        $gy = $objy{$d};
        $ik = &tryputforce('?lN', $gx, $gy, $n, 'dmy', 1, -1, 0);
    }

    elsif($dt eq 'rightnear') {
 {
#   print "n $n | bbox $objllx{$n} $objlly{$n} $objurx{$n} $objury{$n}\n";
    print "d $d | bbox $objllx{$d} $objlly{$d} $objurx{$d} $objury{$d}\n";
    print "n $n | w $objw{$n} h $objh{$n}\n";
 }
        if($objx{$n} ne '') {
            $gx = $objx{$n};
        }
        else {
            $gx = int($objurx{$d} + $objox{$n} + 0);
        }
        $gy = $objy{$d};
        $ik = &tryputforce('?rN', $gx, $gy, $n, 'dmy', 1, 0, 0);
    }

    elsif($dt eq 'samexy') {
        $objx{$n} = $objx{$d};
        $objy{$n} = $objy{$d};
    print "SOLVE $n\t:$dt\n";
    }
    elsif($dt eq 'samex') {
        $objx{$n} = $objx{$d};
    print "ARRANGE $n\t:$dt\n";
    }
    elsif($dt eq 'samey') {
        $objy{$n} = $objy{$d};
    print "ARRANGE $n\t:$dt\n";
    }
    elsif($dt eq 'onguideline') {
        &onlinepos($n, $d);
        $gx = $objx{$n};
        $gy = $objy{$n};
    }
    else {
        print "rel: not implemented type $dt\n";
        next;   
    }
  if($gx eq ''||$gy eq'') {
    print "fail resolution |$dt|\n";
    print "     d $d $objx{$d} $objy{$d}\n";
    print "     n $n $objx{$n} $objy{$n}\n";
  }
  else {
    print "SOLVE $n\t->\t$gx\t$gy\t:$dt\n";
  }
}

my %relpri = (
    'guideline'     , 800,
    'onguideline'   , 800,
    'up'            , 100,
    'down'          , 100,
    'right'         , 100,
    'left'          , 100,
    'samex'         ,  50,
    'samey'         ,  50,
    'samexy'        ,  40,
    'rand'          ,  30,
);

sub dumprelpri {
    my $x;
    foreach $x (keys %relpri) {
        print "  x $x val $relpri{$x}\n";
    }
}

#&dumprelpri;
#exit 99;


sub relpricmp {
    my($adst, $atype) = split(/\@/, $a);
    my($bdst, $btype) = split(/\@/, $b);
    my $ap = $relpri{$atype};
    my $bp = $relpri{$btype};
#    print "a '$adst' atype '$atype' -> pre '$ap'\n";
#    print "b '$bdst' btype '$btype' -> pre '$bp'\n";
    if($ap==$bp) {
        return $adst cmp $bdst;
    }
    else {
        return $ap <=> $bp;
    }
}



sub randx {
    my($n, $by) = @_;
    my $gx;
    $gx = int(rand($by));
    &tryputpack('x+', $gx, $by, $n, 'dmy', 1, $xpitch, 0);
}

sub randy {
    my($n, $bx) = @_;
    my $gy;
    $gy = int(rand($bx));
    &tryputpack('x+', $bx, $gy, $n, 'dmy', 1, 0, $ypitch);
}

sub Qrandx {
    my($n, $by) = @_;
    my $gx;
    my $gy;

    $gy = $by;
    $gx = int(rand($by));
    while($oq{$gx.",".$gy} ne '' &&
        !($objclass{$oq{$gx.",".$gy}} =~ /[hv]?guideline/) ) {
        $gx++;
    }
    $objx{$n} = $gx;
    $objy{$n} = $gy;
    $oq{$gx.",".$gy} = $n;
    print "SOLVE $n\tx+\t$gx\t$gy\n";
}

sub Qrandy {
    my($n, $bx) = @_;
    my $gx;
    my $gy;

    $gx = $bx;
    $gy = int(rand($bx));
    while($oq{$gx.",".$gy} ne '' &&
        !($objclass{$oq{$gx.",".$gy}} =~ /[hv]?guideline/) ) {
        $gy++;
    }
    $objx{$n} = $gx;
    $objy{$n} = $gy;
    $oq{$gx.",".$gy} = $n;
    print "SOLVE $n\t+y\t$gx\t$gy\n";
}


sub makepos2 {
    my $n;
    my $a;
    my @as;
    my $cnr;
    my $adst;
    my $atype;

print "makepos2 + + +\n";
print "\t".(join(',',@olist))."\n";

    foreach $n (@olist) {
print "! $n\n";
            if($objx{$n} ne '' && $objy{$n} ne '') {
                next;
            }
        if(!defined $pos{$n}) {
print "NODEP $n\n";
            &singlesolve($n);
        }
        elsif(defined $pos{$n}) {
            if($pos{$n} eq '') {
print "NODEP2 $n\n";
                &singlesolve($n);
            }
            else {
                @as = split(/\|/,$pos{$n});
                $cnr = 0;
print "         ?  $n ($pos{$n})\n";

                foreach $a (@as) {
                    ($adst, $atype) = split(/\@/,$a);
                    if($adst eq '') {
                        next;
                    }
print "             ?? $a |$adst| |$atype|\n";
                    if($objx{$adst} ne '' && $objy{$adst} ne '') {
print "DEPRESOLVED $n ($adst)\n";
print "  type $postype{$n}\n";
                    }
                    else {
                        $cnr++;
                    }
                }

print "  count not resolved $cnr\n";
                if($cnr==0) {
#print "  try solve\n";
#print "  appearance order ".(join(",", @as))."\n";
#                    my @nas;
#                    @nas = sort relpricmp @as;
#print "  priority order ".(join(",", @nas))."\n";
                    foreach $a (sort relpricmp @as) 
#                    foreach $a (@nas) 
                    {
                        if($objx{$n} ne '' && $objy{$n} ne '') {
                            print " already solved\n";
                            last;
                        }
                        ($adst, $atype) = split(/\@/,$a);
                        if($adst eq '') {
                            print "  no reference\n";
                            &solosolve($n,$atype);
                            next;
                        }
#                       &relsolve($n,$a,$postype{$n});
                        &relsolve($n,$adst,$atype);
                    }

                    if($#as==0) {
print " dep $#as\n";
                        if($objx{$n} eq '' && $objy{$n} ne '') {
print " X is not solved\n";
                            &randx($n,$objy{$n});
                        }
                        elsif($objx{$n} ne '' && $objy{$n} eq '') {
print " Y is not solved\n";
                            &randy($n,$objx{$n});
                        }
                    }

                }
            }
        }
    }

}

sub inspection {
    my $x;
    my $c;
    $c = 0;
        foreach $x (@olist) {
            if($objx{$x} eq '' || $objy{$x} eq '') {
                print "unresolv $x\n";
                $c++;
            }
        }

    return $c;
}

sub setdefaulttype {
    my $n;
    my $v;
    foreach $n (sort @olist) {
        if(defined $objclass{$n}) {
        }
        else {
            $objclass{$n} = $defaultobjclass;
        }
        $v = '';
        if(defined $objvis{$n}) {
            if($objvis{$n}<0) {
                $v = 'invis';
            }
        }
        print "  $n\t! $objclass{$n} $v\n";
    }
}

my $rseq=0;

sub registcomp {
    my(@f) = @_;
    my $i;
    my $zname;
    my $zlabel;
    my %m;
    my $dmykey;
    my %pargs;

    $m{fullline} = join('|',@f);

    if($f[1] eq 'from') {
        # 0    1    2     3  4     5      6
        # link from <any> to <any> <type> <params>
        #

        # 0    1    2     3      4
        # link from <any> <type> <params>
        #
        shift(@f);
        shift(@f);
        $m{from} = shift(@f);

        if($f[0] eq 'to' ) {
print "from/to link\n";
            shift(@f);
            $m{to}   = shift(@f);
        }
        else {
print "from link\n";
        }
        $m{type}  = shift(@f);
        &storeargs(\%pargs, \@f, 0);
    }
    else {
print "generic link\n";
        # 0    1      2      3
        # link <objs> <type> <params>
        #
        shift(@f);
        $m{targets} = shift(@f);
        $m{type}    = shift(@f);
        &storeargs(\%pargs, \@f, 0);
    }

    &printargs(\%pargs, "PARGS");

    foreach $i (keys %pargs) {
        $m{$i} = $pargs{$i};
    }

    
    foreach $i (keys %m) {
        print "comp - $i $m{$i}\n";
    }
    
    $dmykey = sprintf(":%s", $rseq);
    $rseq++;

    print "dmykey $dmykey\n";
#    &insert_mtoa(\%complist, $dmykey, \%m);
    &insert_mtoa_withdefaults(\%complist, $dmykey, \%m);
}

# remove specified object
sub removeO {
    my($refoq, $n) = @_;
    my $p;
    foreach $p ( keys %{ $refoq } ) {
        if($refoq->{$p} eq $n) {
            delete $refoq->{$p};
        }
        elsif($refoq->{$p} eq $n."*") {
            delete $refoq->{$p};
        }
    }
}

# remove temporary points
sub removeT {
    my($refoq, $n) = @_;
    my $p;
    foreach $p ( keys %{ $refoq } ) {
        if($refoq->{$p} =~ /^!/) {
            delete $refoq->{$p};
        }
    }
}

sub drawVempty {
    my($refoq, $sx, $sy, $mark, @vs) = @_;
    my($x,$y);
    my($vi);
    my($ov);
    $x = $sx;
    $y = $sy;
    foreach $vi (@vs) {
        $y = $vi;
        $ov = '';
        if(defined $refoq->{$x.",".$y}) {
            $ov = $refoq->{$x.",".$y};
        }
        if($ov eq '') {
            $refoq->{$x.",".$y} = $mark;
        }
        $x++;
    }
}

sub drawV {
    my($refoq, $sx, $sy, $mark, @vs) = @_;
    my($x,$y);
    my($vi);
    $x = $sx;
    $y = $sy;
    foreach $vi (@vs) {
        $y = $vi;
        $refoq->{$x.",".$y} = $mark;
        $x++;
    }
}

sub drawDempty {
    my($refoq, $sx, $sy, $mark, @vs) = @_;
    my($x,$y);
    my($vi);
    my($ov);
    $x = $sx;
    $y = $sy;
    foreach $vi (@vs) {
        $y += $vi;
        $ov = '';
        if(defined $refoq->{$x.",".$y}) {
            $ov = $refoq->{$x.",".$y};
        }
        if($ov eq '') {
            $refoq->{$x.",".$y} = $mark;
        }
        $x++;
    }
}

sub drawD {
    my($refoq, $sx, $sy, $mark, @vs) = @_;
    my($x,$y);
    my($vi);
    $x = $sx;
    $y = $sy;
    foreach $vi (@vs) {
        $y += $vi;
        $refoq->{$x.",".$y} = $mark;
        $x++;
    }
}


sub qexp {
    my(@pp) = @_; 
    my @cr;
    my @carg;
    my @r;
    my $i;
    my $j; 

     if($#pp==0) {
        return 0..($pp[0]-1);
     }   
    else {
        @carg = @pp;
        shift(@carg);
        @cr = &qexp(@carg);
        @r = (); 
        for($i=0;$i<$pp[0];$i++) {
            foreach $j (@cr) {
                push(@r, "$i-$j");
            }   
        }   
    }   
    return @r; 
}

sub vseval {
    my(@vs) = @_;
    my $v;
    my $lv;
    my $minv;
    my $maxv;
    my $c;
    my $r;
    my $d;

    $c = 0;
    $lv = '';
    $maxv = -999999;
    $minv = 999999;
    foreach $v (@vs) {
        if($v!=$lv) {
            $c++;
        }
        if($v<$minv) { $minv = $v; }
        if($v>$maxv) { $maxv = $v; }

        $lv = $v;
    }
    $d = $maxv - $minv;
    $r = $d*$c;
#   print "dyna $d ch $c r $r";
    print sprintf("; %2d %2d -> %3d", $d, $c, $r);

    return $r;
}

sub fillvoq {
    my($refop,$sx,$sy,$mark, @vs) = @_;
    my $x;
    my $y;
    my $lv;
    my $v;
    my $p;
    my $nr;

    $nr = 0;

    if($#vs==0) {
        $x = $sx;
        $v = $vs[0];
        if($v>$sy) {
            for($y=$sy;$y<=$v;$y++) {
                $p = $x.",".$y;
                $refop->{$p} = $mark; 
                $nr++;
            }
        }
        else {
            for($y=$v;$y<=$sy;$y++) {
                $p = $x.",".$y;
                $refop->{$p} = $mark; 
                $nr++;
            }
        }
    }
    else {
        $lv = $sy;
        $x = $sx;
        foreach $v (@vs) {
            $p = $x.",".$v;
            $refop->{$p} = $mark;
            
            if($lv ne '' && $lv != $v) {
                if($lv<$v) {
                    for($y=$lv;$y<=$v;$y++) {
                        $p = ($x-1).",".$y;
                        $refop->{$p} = $mark;
                        $nr++;
                    }
                }
                else {
                    for($y=$v;$y<=$lv;$y++) {
                        $p = ($x-1).",".$y;
                        $refop->{$p} = $mark; 
                        $nr++;
                    }
                }
            }

            $x++;
            $lv = $v;
        }
    }
}

sub countfillvoq {
    my($refop,$igo,$sx,$sy,$mark, @vs) = @_;
    my $x;
    my $y;
    my $lv;
    my $v;
    my $p;
    my $c;


    if($#vs==0) {
        $x = $sx;
        $v = $vs[0];
        if($v>$sy) {
            for($y=$sy;$y<=$v;$y++) {
                $p = $x.",".$y;
#               $refop->{$p} = $mark; 
                if($refop->{$p} eq $igo) {
                    $c++;
                }
            }
        }
        else {
            for($y=$v;$y<=$sy;$y++) {
                $p = $x.",".$y;
#               $refop->{$p} = $mark; 
                if($refop->{$p} eq $igo) {
                    $c++;
                }
            }
        }
    }
    else {

    $c = 0;
    $lv = '';
    $x = $sx;
    foreach $v (@vs) {
        $p = $x.",".$v;
#        $refop->{$p} = $mark;
        if($refop->{$p} eq $igo) {
            $c++;
        }
        
        if($lv ne '' && $lv != $v) {
            if($lv<$v) {
                for($y=$lv;$y<=$v;$y++) {
                    $p = ($x-1).",".$y;
#                   $refop->{$p} = $mark;
                    if($refop->{$p} eq $igo) {
                        $c++;
                    }
                }
            }
            else {
                for($y=$v;$y<=$lv;$y++) {
                    $p = ($x-1).",".$y;
#                   $refop->{$p} = $mark; 
                    if($refop->{$p} eq $igo) {
                        $c++;
                    }
                }
            }
        }

        $x++;
        $lv = $v;
    }

    }
    return $c;
}

sub washsegs {
    my(@src) = @_;
    my @dst;
    my $last;
    my $cur;
    my ($x, $y);
    my $dirty=0;

    $last = undef;
    while(@src) {
        $cur = shift(@src);
        $cur =~ s/[A-Za-z]//g;
        ($x, $y) = split(/,/, $cur);
        $cur = "$x,$y";
        if($last && $last eq $cur) {
            $dirty++;
        }
        else {
            push(@dst, $cur);
        }
        $last = $cur;
    }

    return @dst;
}

#       @vs = &cvseg2vs($sx,$sy,@msegs);
sub cvseg2vs {
    my($sx,$sy,@segs) = @_;
    my @ret;
    my $pair;
    my($x,$y);
    
    foreach $pair (@segs) {
        ($x,$y) = split(/,/, $pair);
        push(@ret, $y);
    }
#   shift(@ret);
    
    return @ret;
}

sub mksegs {
    my($sx,$sy,@vs) = @_;
    my @segs;
    my $lx;
    my $lv;
    my $x;
    my $v;

    push(@segs, "$sx,$sy");

    if($#vs==0) {
        push(@segs, "$sx,$vs[0]z");
    }
    else {

  if(1) {
        if($vs[0]!=$sy) {
            push(@segs, "$sx,$vs[0]u");
            $sy = $vs[0];
        }
  }

        $lx = $sx;
        $lv = $sy;
        $x = $sx;
        foreach $v (@vs) {
            
            if($lv != $v) {
                if($lx-1>=$sx) {
                    push(@segs, sprintf("%d,%da", $lx-1, $lv));
                }
                push(@segs, sprintf("%d,%db", $x-1, $lv));
                $lx = $x;
            }

            $x++;
            $lv = $v;
        }

        if($x != $lx) {
                if($lx!=$sx) {
                    push(@segs, sprintf("%d,%dc", $lx-1, $lv));
                }
                push(@segs, sprintf("%d,%dd", $x-1, $lv));
        }

    }

    print STDERR "segments 1- ".join('/',@segs)."\n";

    @segs = &washsegs(@segs);

    print STDERR "segments 2- ".join('/',@segs)."\n";

    return @segs;
}

sub drawarrow {
    my($m,@segs) = @_;
    my $ret;
    my @fsegs;
    my @bsegs;

    print STDERR "segments ".join('/',@segs)."\n";
    print STDERR "segments count $#segs\n";
    if($#segs<=0) {
        return "";
    }
    elsif($#segs==1) {
        push(@fsegs, $segs[0]); push(@fsegs, $segs[1]);
        push(@bsegs, $segs[1]); push(@bsegs, $segs[0]);
    }
    elsif($#segs==2) {
        push(@fsegs, $segs[1]); push(@fsegs, $segs[2]);
        push(@bsegs, $segs[1]); push(@bsegs, $segs[0]);
    }
    else {
        push(@fsegs, pop(@segs));
        unshift(@fsegs, pop(@segs));
        unshift(@bsegs, shift(@segs));
        unshift(@bsegs, shift(@segs));
    }
    print STDERR "segments fore ".join('/',@fsegs)."\n";
    print STDERR "segments back ".join('/',@bsegs)."\n";

    $ret = '';
    if($m & 1) { $ret .= &genarrowPScmds(@fsegs); }
    if($m & 2) { $ret .= &genarrowPScmds(@bsegs); }

print " arrows ret |$ret|\n";

    return $ret;
}

sub drawarrowVS {
    my($type,$m,$sx,$sy,@vs) = @_;
    my $o;
    print "DRAWARROW $type <$m> $sx,$sy [";
    foreach $o (@vs) {
        print "$o ";
    }
    print "] \n";

    my @segs;
    my $ret;
    @segs = &mksegs($sx,$sy,@vs);
    $ret  = &drawarrow($m,@segs);

 if(0) {


    my $ret;
    my @fsegs;
    my @bsegs;
    print STDERR "segments $type ".join('/',@segs)."\n";
    print STDERR "segments count $#segs\n";
    if($#segs<=0) {
        return "";
    }
    elsif($#segs==1) {
        push(@fsegs, $segs[0]); push(@fsegs, $segs[1]);
        push(@bsegs, $segs[1]); push(@bsegs, $segs[0]);
    }
    elsif($#segs==2) {
        push(@fsegs, $segs[1]); push(@fsegs, $segs[2]);
        push(@bsegs, $segs[1]); push(@bsegs, $segs[0]);
    }
    else {
        push(@fsegs, pop(@segs));
        unshift(@fsegs, pop(@segs));
        unshift(@bsegs, shift(@segs));
        unshift(@bsegs, shift(@segs));
    }
    print STDERR "segments fore ".join('/',@fsegs)."\n";
    print STDERR "segments back ".join('/',@bsegs)."\n";

    $ret = '';
    if($m & 1) { $ret .= &genarrowPScmds(@fsegs); }
    if($m & 2) { $ret .= &genarrowPScmds(@bsegs); }

print " arrows ret |$ret|\n";
 }

    return $ret;
}

sub drawpath {
    my($type,$sx,$sy,@vs) = @_;
    my $x;
    my $y;
    my $lv;
    my $v;
    my $p;
    my $lx;
    my $ret;
    my @segs;

print "drawpath: type |$type|\n";

    @segs = ();

    print "sx $sx sy $sy\n";
    print "vs ".(join(" ",@vs))."\n";

    if($#vs<0) {
        return "";
    }

 if(0) {
    push(@segs, "$sx,$sy");

    if($#vs==0) {
        push(@segs, "$sx,$vs[0]");
    }
    else {

  if(1) {
        if($vs[0]!=$sy) {
            push(@segs, "$sx,$vs[0]");
            $sy = $vs[0];
        }
  }

        $lx = $sx;
        $lv = $sy;
        $x = $sx;
        foreach $v (@vs) {
            
            if($lv != $v) {
                if($lx-1>=$sx) {
                    push(@segs, sprintf("%d,%da", $lx-1, $lv));
                }
                push(@segs, sprintf("%d,%db", $x-1, $lv));
                $lx = $x;
            }

            $x++;
            $lv = $v;
        }

        if($x != $lx) {
                if($lx!=$sx) {
                    push(@segs, sprintf("%d,%dc", $lx-1, $lv));
                }
                push(@segs, sprintf("%d,%dd", $x-1, $lv));
        }

    }
 }
    @segs = &mksegs($sx,$sy,@vs);

    print STDERR "segments $type ".join('/',@segs)."\n";

    if($type eq 'line') {
        $ret = &genlinePScmds(@segs);
    }
    elsif($type eq 'wave') {
        $ret = &genwavePScmds(@segs);
    }
    elsif($type eq 'thunder') {
        $ret = &genthunderPScmds(@segs);
    }
    elsif($type eq 'cline') {
        $ret = &genclinePScmds(@segs);
    }
    elsif($type eq 'xline') {
        $ret = &genxlinePScmds(@segs);
    }
    else {
        $ret = &genlinePScmds(@segs);
    }

    return $ret;
}

sub genarrowPScmds {
    my(@segs) = @_;
    my ($sx,$sy);
    my ($ex,$ey);
    my $p;
    my $r;

    $r = '';
    
    $r .= "%arrow: data ".(join(" ", @segs))."\n";

    $p = shift(@segs);
    ($sx, $sy) = split(/,/, $p);
    $p = shift(@segs);
    ($ex, $ey) = split(/,/, $p);
    
    if($ex==$sx && $ey==$sy) {
        print "SKIP\n";
    }
    else {

#   $r .= "  $sx $sy $ex $ey sparrow\n";
#   $r .= sprintf("  %d %d %d %d sparrow\n", $sx, $sy, $ex,$ey);
    $r .= sprintf("  %.1f %.1f %.1f %.1f sparrow\n", $sx, $sy, $ex,$ey);
    }
    
    return $r;
}

sub genlinePScmds {
    my(@segs) = @_;
    my ($x,$y);
    my ($lx,$ly);
    my $p;
    my $r;

    $r = '';
    
    $r .= "%line: data ".(join(" ", @segs))."\n";

    $p = shift(@segs);
    ($x, $y) = split(/,/, $p);
#   $r .= sprintf("    %d %d moveto\n", $x, $y);
    $r .= sprintf("    %.1f %.1f moveto\n", $x, $y);
    $lx = $x;
    $ly = $y;

    while(@segs) {
        $p = shift(@segs);
        ($x, $y) = split(/,/, $p);
#       $r .= sprintf("    %d %d lineto\n", $x, $y);
        $r .= sprintf("    %.1f %.1f lineto\n", $x, $y);

        $lx = $x;
        $ly = $y;
    }
    $r .= "    stroke\n";
    
    return $r;
}

sub acthunderline {
    my($sx,$sy,$ex,$ey) = @_;
    my $k;
    my $dmy;
    my($x,$y);
    my $r;

    if($sy==$ey) {
        for($x=$sx;$x<$ex;$x++) {
            $r .= sprintf("%f %f lineto\n", $x+0.5,$sy+0.5);
            $r .= sprintf("%d %d lineto\n", $x+1,$sy);
        }
    }
    if($sx==$ex) {
        if($sy<$ey) {
            for($y=$sy;$y<$ey;$y++) {
                $r .= sprintf("%f %f lineto%% N\n", $sx+0.5,$y+0.5);
                $r .= sprintf("%d %d lineto%% N\n", $sx, $y+1);
            }
        }
        else {
#       $r .= "% vertical $sy..$ey\n";
            for($y=$sy;$y>$ey;$y--) {
                $r .= sprintf("%f %f lineto%% R\n", $sx-0.5, $y-0.5);
                $r .= sprintf("%f %f lineto%% R\n", $sx, $y-1);
            }
        }
    }

    return $r;
}

sub acwaveline {
    my($sx,$sy,$ex,$ey) = @_;
    my $k;
    my($x,$y);
    my $r;
    my $dmy;

    if($sy==$ey) {
#       $r .= "% horizontal $sx..$ex\n";
        for($x=$sx;$x<$ex;$x++) {
            $r .= sprintf("%f %f 0.35 135 45 arcn\n", $x+0.25,$sy-0.25);
            $r .= sprintf("%f %f 0.35 225 315 arc\n", $x+0.75,$sy+0.25);
        }
    }
    elsif($sx==$ex) {
#       $r .= "% vertical $sy..$ey\n";
        if($sy>$ey) {
            for($y=$sy;$y>$ey;$y--) {
                $r .= sprintf("%f %f 0.35 135 225 arc\n",  $sx+0.25,$y-0.25);
                $r .= sprintf("%f %f 0.35  45 315 arcn\n", $sx-0.25,$y-0.75);
            }
        }
        else {
            for($y=$sy;$y<$ey;$y++) {
                $r .= sprintf("%f %f 0.35 315  45 arc\n",  $sx-0.25,$y+0.25);
                $r .= sprintf("%f %f 0.35 225 135 arcn\n", $sx+0.25,$y+0.75);
            }
        }

    }

    return $r;
}

sub acxlineline {
    my($sx,$sy,$ex,$ey) = @_;
    my $k;
    my($x,$y);
    my $r;
    my $dmy;

    if($sy==$ey) {
#       $r .= "% horizontal $sx..$ex\n";
        for($x=$sx;$x<$ex;$x++) {
            $r .= "stroke\n";
            $r .= 
                sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                $x+0.0,$sy+0);
            $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                $x+0.0,$sy+0);
            $r .=
                sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                $x+0.5,$sy+0);
            $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                $x+0.5,$sy+0);

            $r .= "stroke\n";
        }
    }
    elsif($sx==$ex) {
#       $r .= "% vertical $sy..$ey\n";
        if($sy>$ey) {
            for($y=$sy;$y>$ey;$y--) {
                $r .= "stroke\n";
                $r .=
                    sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                    $sx+0.0,$y+0);
                $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                    $sx+0.0,$y+0);
                $r .=
                    sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                    $sx+0.0,$y-0.5);
                $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                    $sx+0.0,$y-0.5);

                $r .= "stroke\n";
            }
        }
        else {
            for($y=$sy;$y<$ey;$y++) {
                $r .= "stroke\n";
                $r .=
                    sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                    $sx+0.0,$y+0);
                $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                    $sx+0.0,$y+0);
                $r .=
                    sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                    $x+0.0,$y+0.5);
                $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                    $x+0.0,$y+0.5);
                $r .= "stroke\n";
            }
        }
    }
    else {
            $k = ($ey-$sy)/($ex-$sx);
        $r .= "% xline generic case; k $k\n";
            for($x=$sx;$x<=$ex;$x+=0.5) {
                $y = $k*($x-$sx)+$sy;
                $r .= "stroke\n";
                $r .=
                    sprintf("%f 0.2 sub %f 0.2 sub moveto 0.4 0.4 rlineto\n",
                    $x+0.0,$y+0);
                $r .=
                sprintf("%f 0.2 sub %f 0.2 add moveto 0.4 0.4 neg rlineto\n",
                    $x+0.0,$y+0);

                $r .= "stroke\n";
            }
    }

    return $r;
}

sub acclineline {
    my($sx,$sy,$ex,$ey) = @_;
    my $k;
    my($x,$y);
    my $r;
    my $dmy;

    if($sy==$ey) {
        $r .= "% cline horizontal $sx..$ex\n";
        for($x=$sx;$x<$ex;$x++) {
            $r .= "stroke\n";
            $r .= sprintf("%f %f 0.05 0 360 arc\n", $x+0.0,$sy+0);
            $r .= "stroke\n";
            $r .= sprintf("%f %f 0.05 0 360 arc\n", $x+0.5,$sy+0);
            $r .= "stroke\n";
        }
    }
    elsif($sx==$ex) {
       $r .= "% cline vertical $sy..$ey\n";
        if($sy>$ey) {
            for($y=$sy;$y>$ey;$y--) {
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y-0.0);
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y-0.5);
                $r .= "stroke\n";
            }
        }
        else {
            for($y=$sy;$y<$ey;$y++) {
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y+0.0);
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y+0.5);
                $r .= "stroke\n";
            }
        }
    }
    else {
            $k = ($ey-$sy)/($ex-$sx);
        $r .= "% cline generic case; k $k\n";
            for($x=$sx;$x<=$ex;$x+=0.5) {
                $y = $k*($x-$sx)+$sy;
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $x+0,$y+0.0);
                $r .= "stroke\n";
            }
    }

    return $r;
}

sub acclineline2 {
    my($sx,$sy,$ex,$ey) = @_;
    my $k;
    my($x,$y);
    my $r;
    my $dmy;

    if($sy==$ey) {
#       $r .= "% horizontal $sx..$ex\n";
        for($x=$sx;$x<$ex;$x++) {
            $r .= "stroke\n";
            $r .= sprintf("%f %f 0.05 0 360 arc\n", $x+0.25,$sy+0);
            $r .= "stroke\n";
            $r .= sprintf("%f %f 0.05 0 360 arc\n", $x+0.75,$sy+0);
            $r .= "stroke\n";
        }
    }
    elsif($sx==$ex) {
#       $r .= "% vertical $sy..$ey\n";
        if($sy>$ey) {
            for($y=$sy;$y>$ey;$y--) {
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y-0.25);
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y-0.75);
                $r .= "stroke\n";
            }
        }
        else {
            for($y=$sy;$y<$ey;$y++) {
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y+0.25);
                $r .= "stroke\n";
                $r .= sprintf("%f %f 0.05 0 360 arc\n",  $sx+0,$y+0.75);
                $r .= "stroke\n";
            }
        }

    }

    return $r;
}

sub genthunderPScmds {
    my(@segs) = @_;
    my ($x,$y);
    my ($lx,$ly);
    my $p;
    my $r;

    $r = '';
    
#    $r .= "0 0 1 setrgbcolor\n";
#    $r .= "0.1 setlinewidth\n";

    $p = shift(@segs);
    ($x, $y) = split(/,/, $p);
    $r .= sprintf("    %d %d moveto\n", $x, $y);
    $lx = $x;
    $ly = $y;

    while(@segs) {
        $p = shift(@segs);
        ($x, $y) = split(/,/, $p);
#       $r .= sprintf("    %d %d lineto\n", $x, $y);
        $r .= &acthunderline($lx,$ly,$x,$y);

        $lx = $x;
        $ly = $y;
    }
    $r .= "    stroke\n";
    
    return $r;
}

sub genwavePScmds {
    my(@segs) = @_;
    my ($x,$y);
    my ($lx,$ly);
    my $p;
    my $r;

    $r = '';
    
#    $r .= "0 0 1 setrgbcolor\n";
#    $r .= "0.1 setlinewidth\n";

    $p = shift(@segs);
    ($x, $y) = split(/,/, $p);
    $r .= sprintf("    %d %d moveto\n", $x, $y);
    $lx = $x;
    $ly = $y;

    while(@segs) {
        $p = shift(@segs);
        ($x, $y) = split(/,/, $p);
#       $r .= sprintf("    %d %d lineto\n", $x, $y);
        $r .= &acwaveline($lx,$ly,$x,$y);

        $lx = $x;
        $ly = $y;
    }
    $r .= "    stroke\n";
    
    return $r;
}

sub genxlinePScmds {
    my(@segs) = @_;
    my ($x,$y);
    my ($lx,$ly);
    my $p;
    my $r;

    $r = '';
    
    $p = shift(@segs);
    ($x, $y) = split(/,/, $p);
    $r .= sprintf("    %d %d moveto\n", $x, $y);
    $lx = $x;
    $ly = $y;

    while(@segs) {
        $p = shift(@segs);
        ($x, $y) = split(/,/, $p);
        $r .= &acxlineline($lx,$ly,$x,$y);

        $lx = $x;
        $ly = $y;
    }
    $r .= "    stroke\n";
    
    return $r;
}

sub genclinePScmds {
    my(@segs) = @_;
    my ($x,$y);
    my ($lx,$ly);
    my $p;
    my $r;

    $r = '';
    
    $p = shift(@segs);
    ($x, $y) = split(/,/, $p);
    $r .= sprintf("    %d %d moveto\n", $x, $y);
    $lx = $x;
    $ly = $y;

    while(@segs) {
        $p = shift(@segs);
        ($x, $y) = split(/,/, $p);
        $r .= &acclineline($lx,$ly,$x,$y);

        $lx = $x;
        $ly = $y;
    }
    $r .= "    stroke\n";
    
    return $r;
}


sub brangewidthfilter {
    my($cy, $a, $rs) = @_;
    my($r);
    my ($sy,$ey);
    my @tg = split(/ /,$rs);
    my @og;
    my $u;
    @og = ();
    foreach $u (@tg) {
        ($sy, $ey) = split(/,/, $u);
#        print sprintf(" [%d..%d] cy +0 %d\n", $sy, $ey, $cy);
#        print sprintf(" [%d..%d] cy +a %d\n", $sy, $ey, $cy+$a);
#        print sprintf(" [%d..%d] cy -a %d\n", $sy, $ey, $cy-$a);
        if($ey<=$cy-$a || $sy>$cy+$a) {
#            print "\tunuse\n";
        }
        else {
#            print "\tUSE\n";
            push(@og, $u);
        }
    }
    $r = join(' ', @og);
    return $r;
}

sub brangefilter {
    my($cy, $rs) = @_;
    my($r);
    my ($sy,$ey);
    my @tg = split(/ /,$rs);
    my @og;
    my $u;
    @og = ();
    foreach $u (@tg) {
        ($sy, $ey) = split(/,/, $u);
        if($sy<=$cy && $cy<=$ey) {
            push(@og, $u);
        }
    }
    $r = join(' ', @og);
    return $r;
}

sub udist {
    my($pair,$ex,$ey) = @_;
    my($sx,$sy,$tx,$ty) = split(/,/, $pair);
    return ($ex-$sx)*($ex-$sx)+($ey-$sy)*($ey-$sy);
}

sub trydirectlink {
    my($type, $src, $dst, $mdfy, $pp) = @_;
    my($sx, $sy, $ex, $ey);
    my($x, $y);
    my($k);
    my($ps);        # position state
    my($lps);       # last position state
    my($lx, $ly);   # last position
    my($asx, $asy, $adx, $ady);
    my $dir;
    my @rseq;

    @rseq = ();

    # most left; min of 2 objects
    $sx = $objllx{$src}; if($objllx{$dst}<$sx) { $sx = $objllx{$dst}; }
    $sy = $objlly{$src}; if($objlly{$dst}<$sy) { $sy = $objlly{$dst}; }
    $ex = $objurx{$src}; if($objurx{$dst}>$ex) { $ex = $objurx{$dst}; }
    $ey = $objury{$src}; if($objury{$dst}>$ey) { $ey = $objury{$dst}; }

 if(0) {
    print "direct grid sx $sx sy $sy -> ex $ex ey $ey\n";

    $over_str .= "gsave\n";
    $over_str .= "  0.8 setgray\n";
    $over_str .= "  0.1 setlinewidth\n";
    for($y=$sy;$y<=$ey;$y++) {
        $over_str .= "  $sx $y moveto $ex $y lineto stroke\n";
    }
    for($x=$sx;$x<=$ex;$x++) {
        $over_str .= "  $x $sy moveto $x $ey lineto stroke\n";
    }
    $over_str .= "grestore\n";
 }

 if(0) {
    if($objx{$src}<$objx{$dst}) {
        $dir = 1;
        $sx = $objx{$src}; $sy = $objy{$src};
        $ex = $objx{$dst}; $ey = $objy{$dst};
    }
    else {
        $dir = 2;
        $sx = $objx{$dst}; $sy = $objy{$dst};
        $ex = $objx{$src}; $ey = $objy{$src};
    }
 }
    $dir = 1;
    $sx = $objx{$src}; $sy = $objy{$src};
    $ex = $objx{$dst}; $ey = $objy{$dst};

    print "direct rawline sx $sx sy $sy -> ex $ex ey $ey\n";

 if(0) {
    $over_str .= "gsave\n";
    $over_str .= "  0.8 0.8 1.0 setrgbcolor\n";
    $over_str .= "  0.3 setlinewidth\n";
    $over_str .= "  $sx $sy moveto $ex $ey lineto stroke\n";
    $over_str .= "grestore\n";
 }

    {
        # vertical
        if($ex-$sx==0) {
            $k = undef;
        }
        # horizontal line and others
        else {
            $k = ($ey-$sy)/($ex-$sx);
            print "direct k $k\n";
        }

        {
            my $qd;
            my $qs;

            my @pseq;
            my $pair;

            if(defined $k) {
print "  direct generic line\n";
                for($x=$sx;$x<=$ex;$x++) {
                    $y = $k*($x-$sx)+$sy;
                    push(@pseq, "$x,$y");
                }
            }
            else {
print "  direct vertical line\n";
                $x = $sx;
                if($ey>$sy) {
                    for($y=$sy;$y<=$ey;$y++) {
                        push(@pseq, "$x,$y");
                    }
                }
                else {  
                    $dir = 2;
                    for($y=$ey;$y<=$sy;$y++) {
                        push(@pseq, "$x,$y");
                    }
                }
            }
print "  direct pos-seq ".(join("/",@pseq))."\n";

            $lps = -1;
            $lx = $sx;
            $ly = $sy;
            $asx = undef;

 if(0) {
            $over_str .= "gsave\n";
            $over_str .= "  1.0 0.9 0.7 setrgbcolor\n";
            $over_str .= "  0.1 setlinewidth\n";
 }

            foreach $pair (@pseq) {
                ($x, $y) = split(/,/, $pair);

                $qd = isinbbox($x,$y,$dst);
                $qs = isinbbox($x,$y,$src);

                $ps = 0;
                if($qd && $qs) {
                    last;
                }
                if($qs) { $ps = 1; }
                if($qd) { $ps = 2; }

                print "  direct x,y $x,$y ps $ps ;dir $dir\n";

 if(0) {
                $over_str .= "$x $y moveto $x $y 0.3 0 360 arc ";
                if($ps==1)      { $over_str .= "1 0 0 setrgbcolor fill\n"; }
                elsif($ps==2)   { $over_str .= "0 0 1 setrgbcolor fill\n"; }
                else            { $over_str .= "0 1 0 setrgbcolor fill\n"; }
 }

                if($dir==1) {
                    if($lps==1 && $ps==0) {
                        print "  direct in space\n";
                        $asx = $lx;
                        $asy = $ly;
                    }
                    if($lps==0 && $ps==2) {
                        print "  direct out space\n";
                        $adx = $x;
                        $ady = $y;
                    }
                }
                elsif($dir==2) {
                    if($lps==2 && $ps==0) {
                        print "  direct in space\n";
                        $asx = $lx;
                        $asy = $ly;
                    }
                    if($lps==0 && $ps==1) {
                        print "  direct out space\n";
                        $adx = $x;
                        $ady = $y;
                    }
                }

                $lps = $ps;
                $lx = $x;
                $ly = $y;
            }

 if(0) {
            $over_str .= "grestore\n";
 }

            if(defined $asx) {
                print "  direct asx,asy $asx,$asy adx,ady $adx,$ady\n";
 if(0) {
                $over_str .= "gsave\n";
                $over_str .= "  0.1 setlinewidth\n";
                $over_str .= "  $asx $asy moveto $adx $ady lineto stroke\n";
                $over_str .= "grestore\n";
 }
                push(@rseq, "$asx,$asy");
                push(@rseq, "$adx,$ady");
            }
            else {
                print "  direct no actual src, dst position\n";
            }

        }
    }

    return @rseq;
}


sub markdirectlink {
    my($nar, $mark, @seq) = @_;
    my($sx,$sy) = split(/,/, $seq[0]);
    my($ex,$ey) = split(/,/, $seq[1]);
    my($x,$y);
    my $k;
    my $p;
    my $gx;
    my ($gxl, $gxr);
    my $gy;
    my ($gyd, $gyu);

    if($ex-$sx==0) {
        $k = undef;
    }
    else {
        $k = ($ey-$sy)/($ex-$sx);
    }

    if(defined $k) {
        for($x=$sx;$x<=$ex;$x++) {
            $y = $k*($x-$sx)+$sy;
    
            $gy = int($y);
            if($gy == $y) {
                $p = "$x,$y";
                if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
            }
            else {
                if($y>=0) {
                    $gyu = $gy+1;
                    $gyd = $gy;
                }
                else {
                    $gyu = $gy;
                    $gyd = $gy-1;
                }

                $p = "$x,$gyd";
                if(!defined $nar->{$p}) { $nar->{$p} = $mark; }

                $p = "$x,$gyu";
                if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
            }
        }
    }
    else {
        $x= $sx;
        for($y=$sy;$y<=$ey;$y++) {
            $p = "$x,$y";
            if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
        }
    }
}


sub godirectlink {
    my($type, $src, $dst, $mdfy, $pp) = @_;
    my @qdirectlink;
    my $dir;

    if($objx{$src}<$objx{$dst}) {
        $dir = 1;
    }
    else {
        $dir = 2;
        # swap
        my $dmy;
        $dmy = $dst;
        $dst = $src;
        $src = $dmy;
    }

    @qdirectlink = &trydirectlink($type, $src, $dst, $pp);
    print "direct nqdir $#qdirectlink\n";
    if($#qdirectlink>=1) {
        my $pre  = &mkprop($pp);
#        my $body = &genlinePScmds(@qdirectlink);
        my $body;
        
    if($type eq 'line') {
        $body = &genlinePScmds(@qdirectlink);
    }
    elsif($type eq 'wave') {
        $body = &genwavePScmds(@qdirectlink);
    }
    elsif($type eq 'thunder') {
        $body = &genthunderPScmds(@qdirectlink);
    }
    elsif($type eq 'cline') {
        $body = &genclinePScmds(@qdirectlink);
    }
    elsif($type eq 'xline') {
        $body = &genxlinePScmds(@qdirectlink);
    }
    else {
        $body = &genlinePScmds(@qdirectlink);
    }


        my $ars  = "";
        my $at;
        $at = 0;
        if(defined $pp->{arrow}) {
             if($dir==1) {$at |= 1; } else {$at |= 2; }
        }
        if(defined $pp->{rarrow}) {
             if($dir==1) {$at |= 2; } else {$at |= 1; }
        }
        if($at!=0) {
            $ars  = &drawarrow($at,@qdirectlink);
        }

    print "direct pre $pre\n";
#   print "direct cmd $body\n";
        $over_str .= "% direct link $src->$dst\n";
        $over_str .= "gsave\n";
        $over_str .= $pre;
        $over_str .= $body;
        $over_str .= $ars;
        $over_str .= "grestore\n";
        $over_str .= "% end direct link $src->$dst\n\n";

        if(defined $pp->{exclusive}) {
            &markdirectlink(\%oq, '+', @qdirectlink);
        }
    }

    return 1;
}


sub goindirectlink {
    my($type, $src, $dst, $mdfy, $pp) = @_;
    my($sx,$sy,$ex,$ey);
    my $ik;
    my $dir;

    $sx = $objx{$src};
    $sy = $objy{$src};
    $ex = $objx{$dst};
    $ey = $objy{$dst};

    print "goindirectlink: $src -> $dst\n";
    if(1) {
        if($ex<$sx) {
            my $dmy;
            $dir = 1;
            $dmy = $sx; $sx = $ex; $ex = $dmy;
            $dmy = $sy; $sy = $ey; $ey = $dmy;
            $dmy = $src;
            $src = $dst;
            $dst = $dmy;
            print "SWAP s and e\n";
            print "goindirectlink: $src -> $dst reverse-direction\n";
        }
     }

    my($w, $h);
    my($ox, $oy);

 if(0) {
    $w  = $classtemplate{$objclass{$src}}{w};
    $h  = $classtemplate{$objclass{$src}}{h};
    $ox = $classtemplate{$objclass{$src}}{ox};
    $oy = $classtemplate{$objclass{$src}}{oy};
 }
    $w  = $objw{$src};
    $h  = $objh{$src};
    $ox = $objox{$src};
    $oy = $objoy{$src};

    print "ring1 w $w h $h ox $ox oy $oy\n";

    my $i;
    my($dx,$dy);
    my($tx,$ty);
    my(@ring1);
    my(@ring2);
    push(@ring1, "0,0,1,0");
    for($i=1;$i<$h+$w;$i++) {
        if($i<$oy) {
#print "\tup\n";
            $dx = 0;
            $dy = $i;
            $tx = 1;
            $ty = $i;
        }
        else {
#print "\tleft\n";
            $dx = $oy -$i;
            $dy = $oy;
            $tx = $oy - $i;
            $ty = $oy + 1;
        }
#       print "i $i;+ $dx,$dy\n";
        push(@ring1, "$dx,$dy,$tx,$ty");

        if($i<($h-$oy)) {
#print "\tdown\n";
            $dx = 0;
            $dy = -$i;
            $tx = 1;
            $ty = -$i;
        }
        else {
#print "\tleft\n";
            $dx = ($h-$oy) -$i;
            $dy = -($h-$oy);
            $tx = ($h-$oy) -$i;
            $ty = -($h-$oy) - 1;
        }
#       print "i $i;- $dx,$dy\n";
        push(@ring1, "$dx,$dy,$tx,$ty");
    }

    print "ring1  ".(join('/',@ring1))."\n";

#   @ring1 = sort { &udist($a,$ex,$ey) <=> &udist($b,$ex,$ey) } @ring1;
#   print "ring1' ".(join('/',@ring1))."\n";

    my($x, $y);
    my($bx,$by);

    $i = 0;
    foreach my $p (@ring1) {
        my $rrr;
        my $q;
        ($dx, $dy, $tx, $ty) = split(/,/, $ring1[$i]);
        $x = $sx + ($w-$ox) + $dx;
        $y = $sy + $dy;
        $bx = $sx + ($w-$ox) + $tx;
        $by = $sy + $ty;
        $q = "$x,$y,$bx,$by";
        $rrr = &udist($q, $ex, $ey);
        print "udist $i : ($p) $q : $rrr\n";
push(@ring2, $q);
        $i++;
    }

    @ring2 = sort { &udist($a,$ex,$ey) <=> &udist($b,$ex,$ey) } @ring2;
    print "ring2' ".(join('/',@ring2))."\n";

    my $lk;
    my $sc;
    my $p;
    $sc = 0;
    $i = 0;
    foreach my $p (@ring2) {
#        ($dx, $dy, $tx, $ty) = split(/,/, $p);
#        $x = $sx + ($w-$ox) + $dx;
#        $y = $sy + $dy;
#        $bx = $sx + ($w-$ox) + $tx;
#        $by = $sy + $ty;
        ($x, $y, $bx, $by) = split(/,/, $p);

#        print "goindirectlink: $i from $x, $y <$dx,$dy>\n";
#        print "goindirectlink: $i from $x,$y <$dx,$dy> - $bx,$by <$tx,$ty>\n";
#        $ik = &trylinkh($type,$src,$dst,$x+0,$y+0,$ex+0,$ey+0,$mdfy);
  if(0) {
        $ik = &trylinkh($type,$src,$dst,$bx+0,$by+0,$ex+0,$ey+0,$mdfy);
#        print "goindirectlink: mid $ik\n";
        print "goindirectlink: mid $src->$dst $i : $x,$y <$dx,$dy> -> $ex,$ey ; $ik\n";
        if($ik>0) {
            $sc = 1;
            last;
        }
  }

#        print "goindirectlink: $i from $x,$y <$dx,$dy> - $bx,$by <$tx,$ty>\n";
        print "goindirectlink: $i sx,sy $sx,$sy x,y $x,$y bx,by $bx,$by ex,ey $ex,$ey\n";

        $lk = &trylinkh($type,$src,$dst,$bx+0,$by+0,$ex+0,$ey+0,$mdfy);
print "ret trylinkh $lk\n";
        if($lk) {
            print "goindirectlink: sx.sy $sx,$sy ; x,y $x,$y ; bx,by $bx,$by\n";
            print "goindirectlink: lk |$lk|\n";
            my $nl;
            if($x!=$bx) {
                $nl = "$y,".$lk;
            }
            else {
                $nl = $lk;
            }
            print "goindirectlink: nl |$nl|\n";

            my $pre;

            $pre = '';

            $pre = &mkprop($pp);


 if(0) {
            if(defined $pp->{thick}) {
                $pre .= "  $pp->{thick} 0.1 mul setlinewidth\n";
            }
            if(defined $pp->{framecolor}) {
                my $cn = &colorresolve($pp->{framecolor});
                if($cn eq '') {
                    $pre .= "%ignore color; fail safe\n";
                    $cn = '0 0 0';
                }
                $pre .= "  $cn setrgbcolor\n";
            }

            {
                my $tmp;
                $tmp = &dashresolve($pp);
                if($tmp ne '') {
                    $pre .= $tmp;
                }
            }
  }
            
            my $dstr = &drawpath($type, $x, $y, split(/,/, $nl));

            if($dst ne '' &&
              ( defined $pp->{arrow} || defined $pp->{rarrow})) {
                my $at=0;

                if($dir==0) {
                    if( defined $pp->{arrow}) {
                        $at |= 1;
                    }
                    if( defined $pp->{rarrow}) {
                        $at |= 2;
                    }
                }
                else {
                    if( defined $pp->{arrow}) {
                        $at |= 2;
                    }
                    if( defined $pp->{rarrow}) {
                        $at |= 1;
                    }
                }
                
                my $arstr = &drawarrowVS($type, $at,
                                $x, $y, split(/,/, $nl));
                if($arstr ne '') {
                    $dstr .= $arstr;
                    print "new dstr with arrow |$dstr|\n";
                }
            }
            if($dst ne '') {
                $over_str .= "  gsave\n";
#                $over_str .= "  1 0 0 setrgbcolor\n";
                $over_str .= $pre;
                $over_str .= $dstr;
                $over_str .= "  grestore\n";
            }
            else {
                print "SKIP actual PS cmds\n";
            }

            &fillvoq(\%oq, $x, $y, '/', split(/,/, $nl));

            last;
        }
        else {
        }

        $i++;
    }

    print "goindirectlink: ret $sc\n";
    return $sc;
}




sub trylink {
    my($type, $src, $dst, $mdfy, $pp) = @_;
    my @qdirectlink;

    if(defined $pp->{indirect}) {
        print "trylink: $src -> $dst indirect <$type>\n";
        return &goindirectlink($type, $src, $dst, $mdfy, $pp);
    }
    else {
        print "trylink: $src -> $dst direct   <$type>\n";
        return &godirectlink($type, $src, $dst, $mdfy, $pp);
    }
}    


sub reduceYsteplinksegs {
    my(@src) = @_;
    my @dst;
    print "reduceYsteplinksegs:\n";

    my $pair;
    my($lx,$ly);
    my($cx,$cy);
    my($x,$y);
    my $dir;
    my $ldir;
#   my ($mx,$my);
#   my ($lmx,$lmy);

    $pair = shift(@src);
    ($lx,$ly) = split(/,/, $pair);
    
#   $lmx = $lx; $lmy = $ly;

    $ldir = '';
    while(@src) {
        $pair = shift(@src);
        ($cx,$cy) = split(/,/, $pair);
#        print "\t$lx,$ly -> $cx,$cy\n";

        $dir = '';
        if($cx==$lx) {
            $dir .= 'v';
        }
        if($cy==$ly) {
            $dir .= 'h';
        }
        if($dir eq 'vh') {
            $dir = 's';
            next;
        }
    
#       print "\t\t$dir\n";
        print "\t$lx,$ly -> $cx,$cy\t$dir $ldir\n";

        if($dir ne $ldir) {
            print "\t\tMARK\n";
#           $mx = $cx; $my = $cy;

            if($dir eq 'h' && $ldir ne '') {
                print "\t\t\tPOP\n";
                pop(@dst);
            }
            push(@dst, "$lx,$ly");
#           push(@dst, "$lmx,$lmy");
#           push(@dst, "$mx,$my");

#           $lmx = $mx; $lmy = $my;
        }
        elsif($dir eq 'h') {
            push(@dst, "$lx,$ly");
        }

        $lx = $cx;
        $ly = $cy;
        $ldir = $dir;
    }
    if($ldir eq 'v') {
                print "\t\t\tPOP 2\n";
        pop(@dst);
    }
    push(@dst, "$cx,$cy");

    return @dst;
}


sub reducesteplinksegs {
    my(@src) = @_;
    my @dst;
    print "reducesteplinksegs:\n";

    my $pair;
    my($lx,$ly);
    my($cx,$cy);
    my($x,$y);
    my $dir;
    my $ldir;
#   my ($mx,$my);
#   my ($lmx,$lmy);

    $pair = shift(@src);
    ($lx,$ly) = split(/,/, $pair);
    
#   $lmx = $lx; $lmy = $ly;

    $ldir = '';
    while(@src) {
        $pair = shift(@src);
        ($cx,$cy) = split(/,/, $pair);
#        print "\t$lx,$ly -> $cx,$cy\n";

        $dir = '';
        if($cx==$lx) {
            $dir .= 'v';
        }
        if($cy==$ly) {
            $dir .= 'h';
        }
        if($dir eq 'vh') {
            $dir = 's';
            next;
        }
    
#       print "\t\t$dir\n";
        print "\t$lx,$ly -> $cx,$cy\t$dir $ldir\n";

        if($dir ne $ldir) {
            print "\t\tMARK\n";
#           $mx = $cx; $my = $cy;

            push(@dst, "$lx,$ly");
#           push(@dst, "$lmx,$lmy");
#           push(@dst, "$mx,$my");

#           $lmx = $mx; $lmy = $my;
        }

        $lx = $cx;
        $ly = $cy;
        $ldir = $dir;
    }
    push(@dst, "$cx,$cy");

    return @dst;
}




sub expandsteplinksegs {
    my(@src) = @_;
    my @dst;
    my ($cx,$cy);
    my ($lx,$ly);
    my $pair;
    my ($x,$y);
    
    print "expandsteplinksegs:\n";
    $pair = shift(@src);
    ($lx,$ly) = split(/,/, $pair);
    while(@src) {
        $pair = shift(@src);
        ($cx,$cy) = split(/,/, $pair);
        print "\t$lx,$ly -> $cx,$cy\n";

        # horizontal
        if($cy==$ly) {
            for($x=$lx;$x<=$cx;$x++) {
                push(@dst, "$x,$ly");
            }
        }
        elsif($cx==$lx) {
            for($y=$ly;$y<=$cy;$y++) {
                push(@dst, "$lx,$y");
            }
        }
        else {
        }

        $lx = $cx;
        $ly = $cy;
    }

    return @dst;
}

sub fixsteplinksegs {
    my($src,$dst,@segs) = @_;
    my $pair;
    my ($x,$y);
    my @fsegs;
    my @nsegs;
    my @osegs;
    my $ps;
    my $lps;
    my $os;
    my $od;

    print "fixsteplinksegs: src $src, dst $dst\n";
    foreach $pair (@segs) {
        ($x, $y) = split(/,/, $pair);
        print "  $x, $y\n";
    }
    @fsegs = &expandsteplinksegs(@segs);
    $lps = '';
    foreach $pair (@fsegs) {
        ($x, $y) = split(/,/, $pair);
        $os = &isinbbox($x, $y, $src);
        $od = &isinbbox($x, $y, $dst);

        $ps = $os*1+$od*2;

        print "  $x, $y : $os $od -> $ps $lps\n";

#       if($ps==0 && $lps==1) {
#           push(@nsegs, "$x,$y");
#       }
        if($ps==0) {
            push(@nsegs, "$x,$y");
        }
        if($ps==2 && $lps==0) {
            push(@nsegs, "$x,$y");
            last;
        }

        $lps = $ps;
    }
 if(1) {
#    @osegs = &reducesteplinksegs(@nsegs);
    @osegs = &reduceYsteplinksegs(@nsegs);
    foreach $pair (@osegs) {
        ($x, $y) = split(/,/, $pair);
        print "  $x, $y\n";
    }

    return @osegs;
 }

    return @nsegs;
}

sub trylinkh {
    my($type, $src, $dst, $sx, $sy, $ex, $ey, $mdfy) = @_;
    my %beoq;
    my %afoq;
    my($dx,$dy);
    my($x,$y);
    my($t0);
    my($t1);
    my($t2);
    my $tdiff;
    my $p;
    my $dir=0;

    $t0 = time;

    my ($minx,$miny,$maxx,$maxy)=&scanbb_byoq(\%oq);
    
    print "trylinkh: $src -> $dst\n";
    print "trylinkh: $sx, $sy -> $ex, $ey\n";
    print "trylinkh: mdfy $mdfy\n";
    &mapoq(\%oq);

    $dx = $ex-$sx;
    $dy = $ey-$sy;

    print "dir $dir\n";
    print "sx $sx sy $sy\n";
    print "ex $ex ey $ey\n";
    print "dx $dx dy $dy\n";

    %afoq = %oq;

    &removeO(\%afoq, $src);
    &removeO(\%afoq, $dst);
    &removeT(\%afoq);
    &mapoq(\%afoq);

    
    my %xc;
    my %yc;
    foreach $p (keys %afoq) {
        ($x,$y) = split(/,/, $p);
        $xc{$x} += $y - $miny + 3;
        $yc{$y} += $x + $minx + 3;
    }

    print "X:\t";
    for($x=$minx;$x<=$maxx;$x++) {
        print sprintf("%2d ", $x);
    }
    print "\n";

    print "XC:\t";
    for($x=$minx;$x<=$maxx;$x++) {
        print sprintf("%2x ",$xc{$x});
    }
    print "\n";

    my %bxn;
    my %byn;
    my $xblk;
    my $xblkv;
    my $yblk;
    my $yblkv;
    my $cv;

    $xblk = 0;
    $xblkv = $xc{$sx};

    for($x=$minx;$x<=$maxx;$x++) {
        $bxn{$x} = -1;
    }

    for($x=$sx;$x<=$ex;$x++) {
        $cv = 0;
        if($xc{$x}>0) {
            $cv = $xc{$x};
        }
        if($cv!=$xblkv) {
            $xblk++;
            $xblkv = $cv;
        }
        $bxn{$x} = $xblk;
    }
    
    print "XP:\t";
    for($x=$minx;$x<=$maxx;$x++) {
        print sprintf("%2d ",$bxn{$x});
    }
    print "\n";

    my $i;
    my @pc;
    my $nactive;
    my %xybrange;

    for($i=0;$i<=$xblk;$i++) {

        my($bxs,$bxe);
        $bxs = $maxx+1;
        $bxe = $minx-1;
        for($x=$sx;$x<=$ex;$x++) {
#           print "\t\tx $x $bxn{$x}\n";
            if($bxn{$x} == $i) {
                if($x<$bxs) { $bxs = $x; }
                if($x>$bxe) { $bxe = $x; }
            }
            if($bxn{$x} > $i) {
                last;
            }
        }
#        print "i $i bxs $bxs bxe $bxe\n";

        undef %yc;
        undef %byn;

        my $pos;
        for($y=$maxy+1;$y>=$miny-1;$y--) {
            for($x=$bxs;$x<=$bxe;$x++) {
                $pos = $x.",".$y;
                if(defined $afoq{$pos}) {
                    $yc{$y} += $x + $minx + 3;
                }
            }
        }

        $nactive = 1;   # upper edge is always active
        $yblk = 0;
        $yblkv = 0;
        for($y=$maxy+1;$y>=$miny-1;$y--) {
            $cv = 0;
            if($yc{$y}>0) {
                $cv = $yc{$y};
            }
            if($cv!=$yblkv) {
                $yblk++;
                $yblkv = $cv;
                if($cv==0) {
                    $nactive++;
                }
            }
            $byn{$y} = $yblk;
        }

        my $j;
        my @ybrange;

        for($j=0;$j<=$yblk;$j++) {
            my($bys,$bye);
            $bys = $maxy+999;
            $bye = $miny-999;
            for($y=$maxy+1;$y>=$miny-1;$y--) {
                if($byn{$y} == $j) {
                    if($y<$bys) { $bys = $y; }
                    if($y>$bye) { $bye = $y; }
                }
            }
            if($bye<$bys) {
                print "   IGNORE what happen?\n";
                next;
            }

            if($yc{$bys}==0) {  # save active blk
                push(@ybrange, "$bys,$bye");
            }
        }

        $xybrange{$i} = join(" ", @ybrange);
        push(@pc, $nactive);
    }

    $x = 1;
    print " active stream in blk [ ";
    foreach $i (@pc) {
        print " $i ";
        $x = $x * $i;
    }
    print "] ";
    print "pattern $x\n";
    print " sx,sy $sx,$sy -> ex,ey $ex,$ey\n";

    print " active stream parts in blk [ ";
    for($i=0;$i<=$xblk;$i++) {
        print " <$xybrange{$i}> ";
    }
    print "]\n";

    my $aw=3;

 if(1) {
    print "filter range $aw\n";
#   $xybrange{0} = &brangefilter($sy, $xybrange{0});
#   $xybrange{$xblk} = &brangefilter($ey, $xybrange{$xblk});
   $xybrange{0} = &brangewidthfilter($sy, $aw, $xybrange{0});
   $xybrange{$xblk} = &brangewidthfilter($ey, $aw, $xybrange{$xblk});

    print " active stream parts in blk [ ";
    for($i=0;$i<=$xblk;$i++) {
        print " <$xybrange{$i}> ";
    }
    print "]\n";

    my @qqpc;
    my @qfs;
    for($i=0;$i<=$xblk;$i++) {
        @qfs = split(/ /, $xybrange{$i});
        push(@qqpc, $#qfs+1);
    }
    
    @pc = @qqpc;

    $x = 1;
    print " active stream in blk [ ";
    foreach $i (@pc) {
        print " $i ";
        $x = $x * $i;
    }
    print "] ";
    print "pattern $x\n";

 }

    for($i=0;$i<=$xblk;$i++) {
        my $m;
    
        my($bxs,$bxe);
        $bxs = $maxx+1;
        $bxe = $minx-1;
        for($x=$sx;$x<=$ex;$x++) {
            if($bxn{$x} == $i) {
                if($x<$bxs) { $bxs = $x; }
                if($x>$bxe) { $bxe = $x; }
            }
            if($bxn{$x} > $i) {
                last;
            }
        }

        foreach $m (split(/ /, $xybrange{$i})) {
            my($bys, $bye) = split(/,/, $m);

            ### XXX
            $bxs -= 0.3;
            $bys -= 0.3;
            $bxe += 0.3;
            $bye += 0.3;

            if($debug) {
                $over_str .= "  gsave\n";
                $over_str .= "    0.1 setlinewidth\n";
                $over_str .= "    0 1 0 setrgbcolor\n";
                $over_str .= "    $bxs $bys moveto\n";
                $over_str .= "    $bxe $bys lineto\n";
                $over_str .= "    $bxe $bye lineto\n";
                $over_str .= "    $bxs $bye lineto\n";
                $over_str .= "    closepath stroke\n";
                $over_str .= "  grestore\n";
            }
        }
    }


    my @bplist;
    my $m;
    my $u;
    my @bp;
    my @bpr;

        print sprintf("%3s: %-20s ", "", "");
    for($i=0;$i<=$xblk;$i++) {
        my($bxs,$bxe);
        $bxs = $maxx+1;
        $bxe = $minx-1;
        for($x=$sx;$x<=$ex;$x++) {
#           print "\t\tx $x $bxn{$x}\n";
            if($bxn{$x} == $i) {
                if($x<$bxs) { $bxs = $x; }
                if($x>$bxe) { $bxe = $x; }
            }
            if($bxn{$x} > $i) {
                last;
            }
        }
        print sprintf("%-8s ", "$bxs,$bxe");
    }
        print "\n";

        print sprintf("%3s: %-20s ", "", "");
    for($i=0;$i<=$xblk;$i++) {
        print sprintf("%-8s ", "---");
    }
        print "\n";

    my $rsucc;
    my $rfail;
    my $isfail;

    my @streamlist;
    @streamlist = ();

    my %pathlist;
    my %pathev;


    $rsucc = 0;
    $rfail = 0;

    @bplist = &qexp(@pc);
    $m = 0;
    foreach $u (@bplist) {
        print sprintf("%3d: %-20s ", $m, $u);

        my($bxs,$bxe);

        for($i=0;$i<=$xblk;$i++) {
            $bxs = $maxx+1;
            $bxe = $minx-1;
            for($x=$sx;$x<=$ex;$x++) {
                if($bxn{$x} == $i) {
                    if($x<$bxs) { $bxs = $x; }
                    if($x>$bxe) { $bxe = $x; }
                }
                if($bxn{$x} > $i) {
                    last;
                }
            }
        }

        @bp = split(/-/, $u);
        
        my($lbys, $lbye);

        my(@thestream);

        $isfail = 0;
        
        for($i=0;$i<=$xblk;$i++) {
            @bpr = split(/ /, $xybrange{$i});
            print sprintf(" %-8s", $bpr[$bp[$i]]);

            my($bys, $bye) = split(/,/, $bpr[$bp[$i]]);

            if($i==0) {
                if($sy<$bys || $sy>$bye) {
                    print "/ ";
                    $isfail = 1;
                    last;
                }
            }
            else {
# print "lbys $lbys lbye $lbye; bys $bys bye $bye ";
                if($bys>$lbye || $bye<$lbys) {
                    print "! ";
                    $isfail = 1;
                    last;
                }
                else {
#                   print "* ";
                }
            }
                if($i>=$xblk) {
                    if($ex>=$bys||$ex<=$bye) {
                    }
                    else {
                        print "? ";
                        $isfail = 1;
                        last;
                    }
                }

            push(@thestream, $bpr[$bp[$i]]);

            $lbys = $bys;
            $lbye = $bye;
        }

        if($isfail) {
            $rfail++;
        }
        else {
            $rsucc++;
            print " ok ";
        }
        print "\n";

        if(! $isfail) { 
#        print "\t".(join("|", @thestream))."\n";
            push(@streamlist, join("|", @thestream));
        }

        $m++;
    }

    print " rsucc $rsucc rfail $rfail\n";
    
    if($rsucc<=0) {
        print " no route for the link\n";
        return undef;
    }

  if(0) {
    $m = 0;
    foreach $u (@streamlist) {
        print "$m: $u\n";
        $m++;
    }
  }

    my(@thestream);
    my $ev;
    $m = 0;
    foreach $u (@streamlist) {
        my($bxs,$bxe);
        my($bys,$bye,$lbys,$lbye);
        my($cy,$lcy);
        my(@vs);
        my($v);
        my($moreknee);
        my $ntry;
        my $ig;

#       print "path candidate stream $m\n";
        @thestream = split(/\|/, $u);
 
        $lcy = $sy;
        $lbys = $sy;
        $lbye = $sy;

print "xblk $xblk\n";
BLKLOOP:
        for($i=0;$i<=$xblk;$i++) {
            $bxs = $maxx+1;
            $bxe = $minx-1;
            for($x=$sx;$x<=$ex;$x++) {
                if($bxn{$x} == $i) {
                    if($x<$bxs) { $bxs = $x; }
                    if($x>$bxe) { $bxe = $x; }
                }
                if($bxn{$x} > $i) {
                    last;
                }
            }

            ($bys, $bye) = split(/,/, $thestream[$i]);

            if(0) {
                my $pos;
                printf "=- =- =- =- %d..%d\n", $bxs, $bxe;
                    print sprintf("(%2d) ", $i);
                    for($x=$bxs;$x<=$bxe;$x++) {
                        print sprintf("%2d ",$x+0);
                    }
                print "\n";
                for($y=$bye;$y>=$bys;$y--) {
                    print sprintf("%3d: ",$y);
                    for($x=$bxs;$x<=$bxe;$x++) {
                        $pos = $x . "," . $y;
                        if(defined $afoq{$pos}) {
                            print "*  ";
                        }
                        else {
                            print ".  ";
                        }
                    }
                    print "\n";
                }
            }


print "cy ---\n";
            $ntry=10;
            $ig = 1;

print "cy cur $bys..$bye , last $lbys..$lbye ey $ey, lcy $lcy: x $bxs..$bxe\n";
            if($i>=$xblk) {
print "cy LAST BLK\n";
                $cy = $ey;
print "cy $cy Q\n";
            }
            else {
                if($lcy>$bye) {
                    $cy = $bye;
print "cy $cy E\n";
                }
                elsif($lcy>=$bys && $lcy<=$bye) {
                    $cy = $lcy;
print "cy $cy L\n";
                }
                else {
                    $cy = $bys;
print "cy $cy S\n";
                }
            }

CYSHIFT:    while($ntry>0) {
print "cy ntry $ntry\n";

                if(!defined $afoq{$bxs . "," . $cy}) {
                    $ig = 0;
                    print "cy found unused slot ($cy)\n";
                    last CYSHIFT;
                }
                else {
                    print "cy hit other object/lines\n";
                    if($cy<$bye)        { $cy++;    }
                    elsif($cy==$bye)    { $cy=$bys; }
                    else                { $cy--;    }
                    print "cy $cy X\n";
                }
        
                $ntry--;
            }

            if($ig) { 
                print "cy NOT FOUND\n";
            }

            $moreknee = 0;

            if($i>0) {
                if($cy>$lbye || $cy<$lbys) {
#                       print "OVER last block $lbys..$lbye vs $cy ; cur block $bys..$bye\n";
                    $moreknee = 1;

                    if($cy>$lbye) {
                        print "TURN 1\n";
#                           print " use lbye\n";
                        push(@vs, $lbye);
                        $bxs++;
                    }
                    if($cy<$lbys) {
                        print "TURN 2\n";
#                           print " use lbys\n";
                        push(@vs, $lbys);
                        $bxs++;
                    }
                }
            }

#            if($bxe>$ex) {
#                $bxe = $ex;
#            }

            for($x=$bxs;$x<=$bxe;$x++) {
  if(1) {
                if($x>$ex) {
                    print "ESCAPE 4 i $i/$xblk x $x $bxs..$bxe y $cy\n";
                    #push(@vs, $cy);
                    last BLKLOOP;
                }
  }
  if(0) {
                if(isinbbox($x,$cy,$dst)==1) {
                    print "ESCAPE 3 i $i/$xblk x $x $bxs..$bxe y $cy\n";
                    push(@vs, $cy);
                    last BLKLOOP;
                }
  }
  if(0) {
                if(defined $oq{$x.",".$cy} && $oq{$x.",".$cy} eq $dst) {
                    print "ESCAPE 1 i $i/$xblk x $x $bxs..$bxe y $cy\n";

                    if($x==$bxs) {
                        if($x!=$bxe) {
                            print "   ESCAPE add\n";
                            push(@vs, $cy);
                        }
                    }
                    last BLKLOOP;
                }
  }
                push(@vs, $cy);

            }
  if(0) {
                if(defined $oq{($x+1).",".$cy} && $oq{($x+1).",".$cy} eq $dst) {
                    print "ESCAPE 2 $x+1,$cy\n";
                    last BLKLOOP;
                }
  }

            $lcy = $cy;
            $lbye = $bye;
            $lbys = $bys;

        }

        if($#vs<0) {
            last;
        }
        if($#vs==0) {
# XXX it is vertcial line !!!
        }

        print "cand0 $m:";
        print "vs:\t";
        for($x=$minx;$x<$sx;$x++) {
            print sprintf("%2s ","");
        }
        foreach $v (@vs) {
            print sprintf("%2d ",$v);
        }

        $ev = &vseval(@vs);

        print "\n";


  {
        my @segs;
        @segs = &mksegs($sx,$sy,@vs);
        my @msegs;
        @msegs = &fixsteplinksegs($src,$dst,@segs);
        @vs = &cvseg2vs($sx,$sy,@msegs);
  }

        print "cand1 $m:";
        print "vs:\t";
        for($x=$minx;$x<$sx;$x++) {
            print sprintf("%2s ","");
        }
        foreach $v (@vs) {
            print sprintf("%2d ",$v);
        }

        $ev = &vseval(@vs);

        print "\n";

        my $c;
        $c = &countfillvoq(\%oq, $src, $sx, $sy, '=', @vs);
        print "stand on ignore object $c\n";
        if($c>1) {
            next;
        }

        $pathlist{$m} = join(",", @vs);
        $pathev{$m} = $ev;

        $m++;
    }

    my @betterorder = sort {$pathev{$a} <=> $pathev{$b}} keys %pathlist;

 if(0) {
    foreach my $k (keys %pathlist) {
        printf "%2d:\n", $k;
    }
    foreach my $k (@betterorder) {
        printf "%2d: %3d\n", $k, $pathev{$k};
    }
 }

    my %pathoq;

    print "BEST  ".($betterorder[0])."\n";
    %pathoq = %oq;
    &fillvoq(\%pathoq, $sx, $sy, '=',
        split(/,/, $pathlist{$betterorder[0]}));
    &mapoq(\%pathoq);

    print "WORST ".($betterorder[$#betterorder])."\n";
    %pathoq = %oq;
    &fillvoq(\%pathoq, $sx, $sy, '=',
        split(/,/, $pathlist{$betterorder[$#betterorder]}));
    &mapoq(\%pathoq);

    #
    # use BEST path
    #
    my $choice = $betterorder[0];
 if(0) {
    my $dstr = &drawpath($type, $sx, $sy, split(/,/, $pathlist{$choice}));

    if($dst ne '') {
        $over_str .= "  gsave\n";
        $over_str .= "  1 0 0 setrgbcolor\n";
        $over_str .= $dstr;
        $over_str .= "  grestore\n";
    }

    print "RESULT\n";
    my @vs = split(/,/, $pathlist{$choice});
    &fillvoq(\%oq, $sx, $sy, '/', split(/,/, $pathlist{$choice}));
    &mapoq(\%oq);

#######################
#######################
    $t1 = time;
    $tdiff = $t1 - $t0;
    print "trylinkh: end $rsucc ; diff $tdiff; ret $m\n";

    return $m;
 }
    return $pathlist{$choice};
}

sub trycomp {
    my($type, $src, $dst, $mdfy) = @_;
    my($sx, $sy, $ex, $ey);
    my $k;
    my $dx;
    my($x,$y);

    print "CC type $type $src -> $dst | $mdfy\n";
    $over_str .= "% ED: draw $type $src -> $dst\n";

    $sx = $objx{$src};
    $sy = $objy{$src};
    $ex = $objx{$dst};
    $ey = $objy{$dst};
    
    if($debug) {
        $over_str .= "$sx $sy moveto $ex $ey lineto stroke\n";
        print "CC baseline $sx $sy -> $ex $ey\n";
    }

    my($cc, $ic);
    # correct/ignore counter
    my($o);

    $cc = 0;
    $ic = 0;

    $dx = $ex-$sx;
    if($dx<0) {
        $dx = - $dx;
    }
    if($dx < 1) {
        $k = 999;
        print "CC k vert ; dx $dx vs oqu $oqu\n";
        if($sy<=$ey) {
            $x = $sx;
            for($y=$sy;$y<=$ey;$y++) {
                print "CC pos + $x,$y\n";

                $o = $oq {$x . "," . $y };
                if($o eq '')            { $cc++; }
                elsif($o eq $src)       { $cc++; }
                elsif($o eq $src.'*')   { $cc++; }
                elsif($o eq $dst)       { $cc++; }
                elsif($o eq $dst.'*')   { $cc++; }
                else { print "CC ignore $x,$y |$o|\n"; $ic++; }

                if($mdfy && $o eq '') {
                    $oq {$x . "," . $y } = 3;
                }
            }
        }
        else {
            $x = $sx;
            for($y=$sy;$y>=$ey;$y--) {
                print "CC pos + $x,$y\n";

                $o = $oq {$x . "," . $y };
                if($o eq '')            { $cc++; }
                elsif($o eq $src)       { $cc++; }
                elsif($o eq $src.'*')   { $cc++; }
                elsif($o eq $dst)       { $cc++; }
                elsif($o eq $dst.'*')   { $cc++; }
                else { print "CC ignore $x,$y |$o|\n"; $ic++; }

                if($mdfy && $o eq '') {
                    $oq {$x . "," . $y } = 4;
                }
            }
        }
    }
    else {
        $k = ($ey-$sy)/($ex-$sx);
        print "CC k $k ; dx $dx vs oqu $oqu\n";
        if($sx<=$ex) {
            for($x=$sx;$x<=$ex;$x++) {
                $y = int($sy + ($x - $sx) * $k);
                print "CC pos + $x,$y\n";

                $o = $oq {$x . "," . $y };
                if($o eq '')            { $cc++; }
                elsif($o eq $src)       { $cc++; }
                elsif($o eq $src.'*')   { $cc++; }
                elsif($o eq $dst)       { $cc++; }
                elsif($o eq $dst.'*')   { $cc++; }
                else { print "CC ignore $x,$y |$o|\n"; $ic++; }

                if($mdfy && $o eq '') {
                    $oq {$x . "," . $y } = 1;
                }
            }
        }
        else {
            for($x=$sx;$x>=$ex;$x--) {
                $y = int($sy + ($x - $sx) * $k);
                print "CC pos - $x,$y\n";

                $o = $oq {$x . "," . $y };
                if($o eq '')            { $cc++; }
                elsif($o eq $src)       { $cc++; }
                elsif($o eq $src.'*')   { $cc++; }
                elsif($o eq $dst)       { $cc++; }
                elsif($o eq $dst.'*')   { $cc++; }
                else { print "CC ignore $x,$y |$o|\n"; $ic++; }

                if($mdfy && $o eq '') {
                    $oq {$x . "," . $y } = 2;
                }
            }
        }
    }

    print "CC cc $cc ic $ic\n";

    return $ic;
}

sub expand_cycle {
    my (@ss) = @_;
    my $f;
    my $l;
    my @r;

#print "expand_cycle\n";
    @r = ();
    
    $l = '';
    foreach $f (@ss) {
        if($l ne '') {
            push(@r, "$l,$f");
        }

        $l = $f;
    }
    push(@r, "$l,".$ss[0]);
#print "->".(join("/",@r))."\n";

    return @r;
}

sub expand_list {
    my (@ss) = @_;
    my $f;
    my $l;
    my @r;

#print "expand_cycle\n";
    @r = ();
    
    $l = '';
    foreach $f (@ss) {
        if($l ne '') {
            push(@r, "$l,$f");
        }

        $l = $f;
    }
#print "->".(join("/",@r))."\n";

    return @r;
}


sub nC2 {
    my(@ss) = @_;
    my $f;
    my $g;
    my @src;
    my @dst;
    my @r;

    @src = @ss;
    while(@src) {
        $f = shift(@src);
        @dst = @src;
        foreach $g (@dst) {
#           print "  $f-$g\n";
            push(@r, "$f,$g");
        }
    }
    
    return @r;
}

sub expand_full {
    my (@ss) = @_;
#print "expand_full\n";
    return &nC2(@ss);
}

sub cmpXval {
    my($aX,$aY) = split(/,/, $a);
    my($bX,$bY) = split(/,/, $b);
    return $aX <=> $bX;
}

sub cmpYval {
    my($aX,$aY) = split(/,/, $a);
    my($bX,$bY) = split(/,/, $b);
    return $aY <=> $bY;
}

sub min { my($a,$b) = @_; if($a>$b) {   return $b; } else { return $a; } }
sub max { my($a,$b) = @_; if($a<$b) {   return $b; } else { return $a; } }

sub isoverbbox {
    my($a, $b) = @_;
    my($allx, $ally, $aurx, $aury) = split(/,/, $a);
    my($bllx, $blly, $burx, $bury) = split(/,/, $b);
    my($xin, $yin);
    $xin = $yin = 0;
    if($allx <= $bllx && $bllx <= $aurx) { $xin++; }
    if($allx <= $burx && $burx <= $aurx) { $xin++; }
    if($ally <= $blly && $blly <= $aury) { $yin++; }
    if($ally <= $bury && $bury <= $aury) { $yin++; }

    if($xin >0 && $yin > 0) {
        $allx = &min($allx,$bllx);
        $ally = &min($ally,$blly);
        $aurx = &max($aurx,$burx);
        $aury = &max($aury,$bury);
        return "$allx,$ally,$aurx,$aury";
    }

    return '';
}


#       &genclipmask_ignoreplace(@igps);
sub genclipmask_ignoreplace {
    my(@srcpairlist) = @_;
    my(@pairlist) = @_;
    my(@tmp);
    my $pair;
    my($x,$y);
    my($lx,$ly);
    my($llx,$lly,$urx,$ury);
    my($ic)=0;
    my @rectlist;
    my $rect;
    my $r=0.55;
    my $i;
    my @others;
    my $rect2;
    my $q;
    my $j;
    my $n;

    @rectlist = ();
    foreach $pair (@srcpairlist) {
        ($x, $y) = split(/,/, $pair);
        push(@rectlist, join(",",($x-$r,$y-$r,$x+$r,$y+$r)));
    }

 if(0) {
    $i = 0;
    foreach $rect (@rectlist) {
        ($llx, $lly, $urx, $ury) = split(/,/, $rect);
        print "rect0[$i] $llx $lly $urx $ury\n";
        $i++;
    }
 }


    $n = $#rectlist+1;
    for($j=0;$j<=$n;$j++) {
#print "j $j\n";

        $rect = shift(@rectlist);
        ($llx, $lly, $urx, $ury) = split(/,/, $rect);
        @others = ();

        while(@rectlist) {
            $rect2 = shift(@rectlist);
#print "rect $rect vs rect2 $rect2 --- ";
            $q = &isoverbbox($rect, $rect2);
            if($q ne '') {
#print "over\n";
                $rect = $q;
#print "\tnew rect $rect\n";
            }
            else {
#print "fall\n";
                push(@others, $rect2);
            }
        }

        @rectlist = @others;
        push(@rectlist, $rect);
    

 if(0) {
        $i = 0;
        foreach $rect (@rectlist) {
            ($llx, $lly, $urx, $ury) = split(/,/, $rect);
            print "rect1[$i] $llx $lly $urx $ury\n";
            $i++;
        }

        $i = 0;
        foreach $rect (@others) {
            ($llx, $lly, $urx, $ury) = split(/,/, $rect);
            print "other[$i] $llx $lly $urx $ury\n";
            $i++;
        }
 }

    }

 if(0) {
    $i = 0;
    foreach $rect (@rectlist) {
        ($llx, $lly, $urx, $ury) = split(/,/, $rect);
        print "rect2[$i] $llx $lly $urx $ury\n";
        $i++;
    }
 }

    return @rectlist;
}

sub scanignoreplace_forbox {
    my($nar, $mark, @bbox) = @_;
    my($llx,$lly,$urx,$ury) = @bbox;
    my($x,$y);
    my $p;
    my(@igplaces);
    
    print "scanignoreplace_forbox: $llx $lly $urx $ury\n";

    @igplaces = ();

    $y = $lly;
    for($x=$llx;$x<=$urx;$x++) {
        $p = "$x,$y";
        if(defined $nar->{$p}) { push(@igplaces, $p); }
    }
    $y = $ury;
    for($x=$llx;$x<=$urx;$x++) {
        $p = "$x,$y";
        if(defined $nar->{$p}) { push(@igplaces, $p); }
    }
    $x = $llx;
    for($y=$lly+1;$y<=$ury-1;$y++) {
        $p = "$x,$y";
        if(defined $nar->{$p}) { push(@igplaces, $p); }
    }
    $x = $urx;
    for($y=$lly+1;$y<=$ury-1;$y++) {
        $p = "$x,$y";
        if(defined $nar->{$p}) { push(@igplaces, $p); }
    }

    print "scanignoreplace_forbox: ret ".(join(" ", @igplaces))."\n";

    return @igplaces;
}



sub markbox {
    my($nar, $mark, @bbox) = @_;
    my($llx,$lly,$urx,$ury) = @bbox;
    my($x,$y);
    my $p;
    my(@igplaces);
    
    print "markbox: $llx $lly $urx $ury\n";

    @igplaces = ();

    $y = $lly;
    for($x=$llx;$x<=$urx;$x++) {
        $p = "$x,$y";
        if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
        else { push(@igplaces, $p); }
    }
    $y = $ury;
    for($x=$llx;$x<=$urx;$x++) {
        $p = "$x,$y";
        if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
        else { push(@igplaces, $p); }
    }
    $x = $llx;
    for($y=$lly+1;$y<=$ury-1;$y++) {
        $p = "$x,$y";
        if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
        else { push(@igplaces, $p); }
    }
    $x = $urx;
    for($y=$lly+1;$y<=$ury-1;$y++) {
        $p = "$x,$y";
        if(!defined $nar->{$p}) { $nar->{$p} = $mark; }
        else { push(@igplaces, $p); }
    }

    return @igplaces;
}







sub trybox {
    my($pp) = @_;
    my($llx,$lly,$urx,$ury) = (9999,9999,-9999,-9999);
    my $delta;
    my $rad;
    my $cont="";
    my $path;
    my $pathseed;
    my $fillpath;
    my $framepath;

    print "trybox:\n";
#   print "0 llx $llx lly $lly urx $urx ury $ury\n";
    foreach my $o (split(/,/, $pp->{targets})) {
#print "obj $o: $objllx{$o} $objlly{$o} $objurx{$o} $objury{$o}\n";
        if($llx>$objllx{$o}) {  $llx = $objllx{$o}; }
        if($lly>$objlly{$o}) {  $lly = $objlly{$o}; }
        if($urx<$objurx{$o}) {  $urx = $objurx{$o}; }
        if($ury<$objury{$o}) {  $ury = $objury{$o}; }
    }
    print "1 llx $llx lly $lly urx $urx ury $ury\n";

    $rad = 1;
    if(defined $pp->{rad}) {
        $rad = $pp->{rad};
    }

    $delta = 1;
    if(defined $pp->{delta}) {
        $delta = $pp->{delta};
    }

    $llx-=$delta;
    $lly-=$delta;
    $urx+=$delta;
    $ury+=$delta;

    print "2 llx $llx lly $lly urx $urx ury $ury\n";

    $cont .= "gsave\n";

    if(defined $pp->{thick}) {
        $cont .= "$pp->{thick} 0.1 mul setlinewidth\n";
    }

 if(0) {
    if(defined $pp->{dashed}) {
        $cont .= " [0.5 0.5] 0 setdash\n";
    }
    elsif(defined $pp->{dotted}) {
        $cont .= " [0.25 0.25] 0 setdash\n";
    }
    elsif(defined $pp->{chained}) {
        $cont .= " [0.75 0.25 0.25 0.25] 0 setdash\n";
    }
 }

            {
                my $tmp;
                $tmp = &dashresolve($pp);
                if($tmp ne '') {
                    $cont .= $tmp;
                }
            }
            
    $path = '';

    my $clippath;
    
    $clippath = '';

    if(defined $pp->{exclusive}) {
        my @igps;
        my $p;
        my($x,$y);
        my @rects;
        my $rect;
        my $dmyp;

        my($ollx,$olly,$ourx,$oury) = ($llx,$lly,$urx,$ury);
        $ollx -= 0.5;
        $olly -= 0.5;
        $ourx += 0.5;
        $oury += 0.5;

        @igps = &scanignoreplace_forbox(\%oq, '#', ($llx, $lly, $urx, $ury));

        @rects = &genclipmask_ignoreplace(@igps);

        $clippath .= "% clipping path for box\n";
        $clippath .= "newpath\n";
        $clippath .= "$ollx $olly moveto $ourx $olly lineto $ourx $oury lineto $ollx $oury lineto closepath\n";
        foreach $rect (@rects) {
            my ($cllx,$clly,$curx,$cury) = split(/,/, $rect);
            $clippath .= "$cllx $clly moveto $cllx $cury lineto $curx $cury lineto $curx $clly lineto closepath\n";
        }

        $clippath .= "eoclip\n";

        $clippath .= "newpath\n";
        
    }

    $path .= $clippath;

    $pathseed .= "% seed [$llx $lly $urx $ury]\n";
    if($pp->{type} eq 'box') {
        $pathseed .= "$llx $lly moveto $urx $lly lineto $urx $ury lineto $llx $ury lineto closepath\n";
    }
    elsif($pp->{type} eq 'rbox') {
        $pathseed .= "$llx $rad add $lly moveto $urx $lly $urx $ury $rad sub $rad arcto $urx $ury $llx $rad sub $ury $rad arcto $llx $ury $llx $lly $rad add $rad arcto $llx $lly $llx $rad add $lly $rad arcto closepath\n";
    }

    $fillpath = $pathseed;
    $framepath = $clippath;
    $framepath .= $pathseed;

    if(defined $pp->{fill}) {

    if(defined $pp->{fillcolor}) {
        my $cn = &colorresolve($pp->{fillcolor});
        if($cn eq '') {
            $cont .= "%ignore color; fail safe\n";
            $cn = '0 0 0';
        }
        $cont .= "  $cn setrgbcolor\n";
    }

        $cont .= $fillpath;
        $cont .= "fill\n";
    }

    if(!defined $pp->{noframe}) {
        if(defined $pp->{framecolor}) {
            my $cn = &colorresolve($pp->{framecolor});
            if($cn eq '') {
                $cont .= "%ignore color; fail safe\n";
                $cn = '0 0 0';
            }
            $cont .= "  $cn setrgbcolor\n";
        }
        $cont .= $framepath;
        $cont .= "stroke\n";
    }

    $cont .= "grestore\n";

    if(defined $pp->{exclusive}) {
        my @igps;
        my $p;
        my($x,$y);
        @igps = &markbox(\%oq, '#', ($llx, $lly, $urx, $ury));

 if(0) {
        $cont .= "gsave\n";
        foreach $p (@igps) {
            ($x, $y) = split(/,/, $p);
            $cont .= "$x $y moveto $x $y 0.5 0 360 arc fill\n";
            
        }
        $cont .= "grestore\n";
 }
        
    }

    if(defined $pp->{back}) {
        $back_str .= $cont;
    }
    else {
        $over_str .= $cont;
    }


}


sub playcomp {
    my $i;
    my $pp;
    my @tgs;
    my @comb;
    my $isfull;
    my $iscycle;
    my $pair;
    my ($src, $dst);

    foreach $i (sort keys %complist) {
        print "comp $i\n";
        $pp = $complist{$i};
        print "  targets |$pp->{targets}| type |$pp->{type}|\n";

        if(defined $boxcat{$pp->{type}}) {
print STDERR "boxcategory $pp->{type}\n";
            &trybox($pp);
        }
        else {
print STDERR "linkcategory $pp->{type}\n";

            @comb = ();

            if(defined $pp->{from}) {
                if(defined $pp->{to}) {
                    my $f;
                    my $t;
                    foreach $f (split(/,/, $pp->{from})) {
                        foreach $t (split(/,/, $pp->{to})) {
                            push(@comb, "$f,$t");
                        }
                    }
                }
                else {
                }
            }
            else {

            $isfull = 0;
            $iscycle = 0;

            @tgs = split(/,/, $pp->{targets});
            print 'PC tgs  '.(join("-", @tgs))."\n";

            print 'PC fullline  '.($pp->{fullline})."\n";

            if($pp->{fullline} =~ /cycle/) {
                $iscycle = 1;
            }
            if($pp->{fullline} =~ /\bfull\b/) {
                $isfull = 1;
            }

            if($isfull) {
                @comb = &expand_full(@tgs);
    print "FULL\n";
            }
            elsif($iscycle) {
                @comb = &expand_cycle(@tgs);
    print "CYCLE\n";
            }
            else {
                @comb = &expand_list(@tgs);
            }

            }

            print 'PC comb '.(join(" ", @comb))."\n";

            foreach $pair (@comb) {
                my $ik;
                ($src, $dst) = split(/,/, $pair);
    #            $ik = &trycomp($pp->{type}, $src, $dst, 0);
    #           if($ik==0) {
    #               $ik = &trycomp($pp->{type}, $src, $dst, 1);
    #           }
                $ik = &trylink($pp->{type}, $src, $dst, 0, $pp);
                if($ik==0) {
                    print "ERROR: fail link $src -> $dst\n";
                }
            }
            next;


        }

    }
}


select STDOUT;
$| = 1;

&loadbuiltins;
&parse;
&countup;
&bindclass;
&showrel;

&setdefaulttype;

#####
##### positioning
#####

my $unc;
my $ic;
$ic = 0;
$unc = 999;
while($unc>0 && $ic<10) {
    &makepos2;
    $unc = &inspection;
    $ic++;
}
if($unc>0) {
    print "*** GIVEUP ***\n";
}

&showloc;

#####
##### compsite
#####

#&xyswap;

&printdict(\%complist);
&playcomp;

#&listoq;
&mapoq(\%oq);

&roughdraw($outputfilename, 800, 80);


