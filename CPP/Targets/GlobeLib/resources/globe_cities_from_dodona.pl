#!/usr/bin/perl -w

use strict;
use DBI;

# use cities_KEYS.txt as input file
#
# set variables for db access
#
my $dbhost = "luke";
my $dbname = "translations";
my $dbuser = "transuser";
my $dbpw = "tranberry";

my $dbh;

my %langs = (
        'EN'            => 'en',
        'SV'            => 'sv',
        'DE'            => 'de',
        'DA'            => 'da',
        'FI'            => 'fi',
        'NO'            => 'no',
        'FR'            => 'fr',
        'ES'            => 'es',
        'IT'            => 'it',
        'NL'            => 'nl',
        'PT'            => 'pt',
        'US'            => 'us',
        'HU'            => 'hu',
        'CS'            => 'cs',
        'EL'            => 'el',
        'PL'            => 'pl',
        'SL'            => 'sl',
        'TR'            => 'tr',
);


my $master_file = "cities_all_langs.txt";

if (!defined $ARGV[0] || $ARGV[0] eq '') {
	die "Script needs input of cities file with coords and name keys\n";
}

open CITY, ">$master_file" || die "Cannot open all langs file\n";

my $select = "SELECT stringKey from strings where stringKey like ?";
my $sql_master = "SELECT translations.stringKey, translations.trstring, " .
      "translations.timest, translations.trcomment FROM " .
      "translations INNER JOIN translations AS " .
      "translation2 ON " .
      "translations.stringKey = translation2.stringKey " .
      "INNER JOIN strings " .
      "ON strings.stringKey=translations.stringKey " .
      "WHERE strings.stringKey = ? AND translation2.lang = ? AND " .
      "(strings.clients LIKE '%G%') " .
      "AND translations.lang = ? " .
      "GROUP BY translations.stringKey, translations.lang, " .
      "translations.timest HAVING " .
      "translations.timest = max(translation2.timest) " .
      "ORDER BY translations.stringKey ASC, " .
      "translations.timest DESC";

&connect;

my $sth = $dbh->prepare($select);
my $sth2 = $dbh->prepare($sql_master);

my $default_en_name;
my $found_name_in_lang;

while (<>) {
	my $stringKey_exists = 0;
	$_ =~ s/\r\n//g;
	chomp;
	my ($lat, $lon, $stringKey) = split(/\t/);
	#print "$stringKey from file\n";
	$sth->execute($stringKey);
	while (my $row = $sth->fetchrow()) {
		print "#### Found stringKey: $row\n";
		$stringKey_exists = 1;
	}
	if ($stringKey_exists) {
		$sth2->execute($stringKey, 'en', 'en');
		$default_en_name = '';
		while (my @en = $sth2->fetchrow_array()) {
			print "    -Name in english: $en[1]\n";
			$default_en_name = $en[1];
		}
		foreach my $lang (keys %langs) {
			print "    -language is $lang\n\n";
			$sth2->execute($stringKey, $langs{$lang}, $langs{$lang});
			$found_name_in_lang = 0;
			while (my @trans = $sth2->fetchrow_array()) {
				$found_name_in_lang = 1;
				#print "looking for $stringKey: $trans[0], $trans[1]\n";
				print CITY "$lang $lat\t$lon\t$trans[1]\n";
			}
			if (!$found_name_in_lang) {
				print "    $stringKey was not translated to $lang\n";
				print CITY "$lang $lat\t$lon\t$default_en_name\n";
			}
		}
	} else {
		print "    $stringKey not found!\n";
	}
}
$sth->finish;
$sth2->finish;

close CITY;

foreach my $lang (keys %langs) {
	`grep "^$lang " $master_file | cut -d" " -f2- > cities_${lang}.txt`;
	`unix2dos cities_${lang}.txt`;
}

# disconnect from db
#
&disconnect;

# subroutine connects to db
#
sub connect {

        $dbh = DBI->connect("DBI:mysql:$dbname:$dbhost", $dbuser, $dbpw)
                || die "Cannot connect to db ($DBI::errstr)\n";

}

# subroutine disconnects from db
#
sub disconnect {

        $dbh->disconnect();

}

