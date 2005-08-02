#!/usr/bin/perl -w
# This scripts checks your system to ensure it has the correct rpm's installed
# to run Ballista. 
# If the correct RPMs are installed ballista will be uncompressed and complied

if (system("rpm -h") == -1)
{
	print("Your system lacks rpm, please double check ballista requirements\n");
	exit 0;
}

@libs = ("'gcc version 3.3.3'","libg++-2.8.1-2","libg++-devel-2.8.1-2","libstdc++5-3.3.1-2mdk","libstdc++5-devel-3.3.1-2mdk","libstdc++5-static-devel-3.3.1-2mdk");
my @notfound;
system("touch temp_cmd_file.txt ");
system("gcc -v 2> temp_cmd_file.txt ");
system("rpm -qa | grep g++ >> temp_cmd_file.txt ");
system("rpm -qa | grep libstdc >> temp_cmd_file.txt");

$missing=0;
$num=0;

      foreach(@libs)
      {
 
	  $grep="grep -q ";
	  $grep.="$_ temp_cmd_file.txt";


	  if ((system("$grep")/256) == 1)
	  {
	      $missing++;
              $nf{$_}=$num;
	    #  push @notfound , $_;
	  }
	$num++;
      }


       system("rm temp_cmd_file.txt");

       if(-e "not_found.txt" )
       {
	       system("rm not_found.txt");
       }

        if($missing > 0)
        {
          $outfile="not_found.txt";
          open OUTFILE,">$outfile" or die "Cannot open $outfile for write :$!";

           $n=0;
	   print "\nThe following dependencie\(s\) are missing: \n\n";
	   print OUTFILE "\nThe following dependencie\(s\) are missing: \n\n";

            foreach(keys %nf)
            {
		$name=$_;

		if($_ eq "'gcc version 3.3.3'")
                {
		    $name="g++-3.3.3-59756cl.i386.rpm for gcc 3.3.3";
                }
		elsif($nf{$_} < 3 && $nf{$_} > 0)
	        {
		   $ext=".i386.rpm";	
		   $name.=$ext;
	        }
	         elsif($nf{$_} > 2)
	        {
		   $ext=".i586.rpm";
		   $name.=$ext;
	        }
		$n++;

		#$name.=$ext;
		print "$name \n";
		print OUTFILE "$name \n";
	    }
	    

	   print "\nThe rpms can be downloaded at www.rpmfind.net\n";
	   print "Install these rpms and re-run this script\n";

	   print OUTFILE "\nThe rpms can be downloaded at www.rpmfind.net\n";
	   print OUTFILE "Install these rpms and re-run this script\n";
	   
	   close OUTFILE;

	   exit 0;
       }
       else
       {
	   if(chdir('ballista') == 1)
            {

              if ((system("./configure")/256) == 0)
              {
		  print "configuration for Ballista done \n";
	      }
	      else
	      {
		  exit 0;
	      }
	    }  
       }
