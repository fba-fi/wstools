#!/usr/bin/perl
#
#
# searches for a date and serializes data
#
# usage: ser <filename> <date>
#
#
if ($#ARGV != 1) { print "Bad number of arguments: $#ARGV\n"; exit 0; }
$fn=$ARGV[0];
$dat=$ARGV[1];
$ind=-1;
open (INFILE,$fn) or die "Can't open file $fn.";
@lastline=("");
while (<INFILE>)
{
  s/^\s+//;
  @line = split;
  if ($line[0] gt $dat)
  { last; }
  else
  { @lastline=@line; }
}
close INFILE;
if ($lastline[0] eq "")
{ exit 0 };

foreach $parm (@lastline)
{
  if ($ind>=0)
  { print "$ind $parm\n"; }
  $ind+=1;
}
if ($ind>=0)
{ print "0 $lastline[1]\n"; }
