#!perl

#NOTE:  this script assumes you are running the Cygwin perl, which uses the
#       Cygwin file paths (i.e. '/' corresponds to 'C:\cygwin')

my $WIN_INSTALLDIR="\\\\dimbo\\panda\\win";

# my $WIN_INSTALLDIR="\\\\cxgeorge-d01\\c\\win";

# my $DEBUG_TREECOPY = 1;

# my $DEBUG_GENERATE_PYTHON_CODE_ONLY = 1;  $ENV{'PANDA_OPTIMIZE'} ='2';

my $DONT_ARCHIVE_OLD_BUILDS = 0;

my $BLD_DTOOL_ONLY=0;
my $DIRPATH_SEPARATOR=':';   # set to ';' for non-cygwin NT perl

my @inst_dirnames=("archive","debug","install","release");
my $ARCHIVENUM=0;
my $DEBUGNUM=1;
my $INSTALLNUM=2;
my $RELEASENUM=3;
my @inst_dirs;
for(my $i=0;$i<=$#inst_dirnames;$i++) {
    $inst_dirs[$i]=$WIN_INSTALLDIR."\\".$inst_dirnames[$i];
}

if(! $DEBUG_GENERATE_PYTHON_CODE_ONLY) {
    $ENV{'PANDA_OPTIMIZE'}='1';  # var has meaning to my special Config.pp
}
$ENV{'PPREMAKE_CONFIG'} = '/usr/local/etc/Config.pp';
$ENV{'TCSH_NO_CSHRC_CHDIR'}='1';
$ENV{'ENABLE_PROFILING'}='1';    # generate .map files

$ENV{'HOME'}="/home/builder";
$ENV{'USER'}="builder";
$ENV{'USERNAME'}=$ENV{'USER'};

my $DONT_ARCHIVE_OLD_BUILDS = (($ENV{'DONT_ARCHIVE_OLD_BUILDS'} ne "") || $DONT_ARCHIVE_OLD_BUILDS);

sub logmsg() {
   my $msg = shift;
   print $msg."\n";
   open(LOGFILE,">>".$fulllogfilename) || die "can't open log file '$fulllogfilename'\n";
   print LOGFILE $msg."\n";
   close(LOGFILE);
}

#could change this to use eval, but it would require doubling the '\''s again in filename
sub mychdir() {
  my $newdir = shift;
  my $retval = chdir($newdir);
  &logmsg("chdir(".$newdir.")");
  if(! $retval) {
      &logmsg("chdir(".$newdir.") failed!!");
      exit(1);
  }
}

sub mymkdir() {
    my $newdir=shift;
    if(!(-e $newdir)) {
      if(!mkdir($newdir,0xfff)) {
       &logmsg("cant create new dir '".$newdir."' !!");
       exit(1);
      }
    }
}

sub myrename() {
  my $n1 = shift;
  my $n2 = shift;
  my $retval;

  &logmsg("rename(".$n1.",".$n2.")");

  if(-e $n2) {

      # find name we can move old target to
      my $newnum=1;
      while(-e $n2.".old.".$newnum) {
          $newnum++;
      }

      $newconflicttargetname=$n2.".old.".$newnum;
      &logmsg("avoiding rename conflict, renaming old ".$n2." to ".$newconflicttargetname);  
      $retval = rename($n2,$newconflicttargetname);
      if(! $retval) {
          &logmsg("rename failing, giving up (check if files or dirs are open in other apps)");
          exit(1);
      }
  }

  $retval = rename($n1,$n2);
  if(! $retval) {
      &logmsg("rename(".$n1.",".$n2.") failed!!!  (check if files or dirs are open in other apps)");
      exit(1);
  }
}

sub myexecstr() {
  my $cmdstr = shift;
  my $errstr = shift;
  my $dologstr = shift;
  my $exec_cshrc_type = shift;

  if($dologstr eq "DO_LOG") {
      if($exec_cshrc_type eq "NT cmd") {
          $cmdstr.=" >> ".$fulllogfilename_win." 2>&1";  # 2>&1 tells nt cmd.exe to redirect stderr to stdout
      } else {
          $cmdstr.=" >>& ".$fulllogfilename;   # for tcsh
      }

      &logmsg($cmdstr);
  }

  my $savedhome = $ENV{'HOME'};

  if( $exec_cshrc_type eq "NO_CSHRC") {
     # change $HOME so .cshrc doesn't get sources by tcsh
     $ENV{'HOME'}="/";  
  } elsif( $exec_cshrc_type eq "NO_PANDA_ATTACH") {
     $ENV{'TCSH_NO_PANDA_ATTACH'}="1";  
  }

  if($exec_cshrc_type eq "NT cmd") {
      my @args = ("cmd.exe", "/c", "$cmdstr");
      $retval = system(@args);
  } else {
      my @args = ("tcsh.exe", "-c", "$cmdstr");
      $retval = system(@args);
  }

  if($retval!=0) {
      $retval= $retval/256;  # actual retval
      if($errstr eq "") {
         &logmsg($cmdstr." failed!!!!!  continuing anyway...\nError return value=".$retval);
      } elsif($errstr ne "nomsg") {
         &logmsg($errstr."\nError return value=".$retval);
         exit($retval);
      }
  }

  if($exec_cshrc_type eq "NO_CSHRC") {
      $ENV{'HOME'}=$savedhome;
  } elsif( $exec_cshrc_type eq "NO_PANDA_ATTACH") {
     delete $ENV{'TCSH_NO_PANDA_ATTACH'};  
  }
}

sub gettimestr() {
    my ($sec,$min,$hour,$mday,$mon,$year,$wday) = localtime(time);
    $mon++;
    return $mon."/".$mday." ".$hour.":".($min<10 ? "0":"").$min;
}

sub appendpath() {
# note cygwin perl.exe requires ':' instead of ';' as path dir separator
  foreach $dir1 (@_) {
      $ENV{'PATH'}.=$DIRPATH_SEPARATOR.$CYGBLDROOT.$dir1;
  }
}

sub make_bsc_file() {
    &logmsg("*** Generating panda.bsc at ".&gettimestr()." ***");
    
#    &mychdir($CYGBLDROOT."/debug");
#    $outputdir=$WINBLDROOT."\\debug";

    &mychdir($CYGBLDROOT);
    $outputdir=$WINBLDROOT;

    $outputname="panda.bsc";
    $outputfilepath=$outputdir."\\".$outputname;
    $cmdfilepath=$outputdir."\\makebsc.txt";
    
    #open(FILES, "where -r . *.sbr |") || die "Couldn't run 'where'!\n";
    #requires unix/cygwin style find.exe
    
    open(FILES, "find ".$dirstodostr." -name \"*.sbr\" -print |") || die "Couldn't run 'find'!\n";
    
    open(OUTFILE, ">".$cmdfilepath) || die "Couldn't open ".$cmdfilepath."!\n";
    
    $filename = <FILES>;  #skip initial line
    $filestr="";
    
    $duline = <FILES>;
    chop $duline;
    $filestr=$duline;
    
    while ($duline = <FILES>)
    {
            chop $duline;  # Remove the newline from the end of the filename
            $filestr=$filestr."\n".$duline;
    }
    
    print OUTFILE "/n /o ".$outputfilepath." \n";
    print OUTFILE "$filestr","\n";
    close(OUTFILE);
    close(FILES);
    
    $bscmake_str="bscmake /o ".$outputfilepath." @".$cmdfilepath."\n";
    
    &myexecstr($bscmake_str,"bscmake failed!!!", "DO_LOG","NT cmd");  

    &myexecstr("copy ".$outputfilepath." ".$inst_dirs[$DEBUGNUM], "copy of ".$outputfilepath." failed!!", "DO_LOG","NT cmd");
    unlink($cmdfilepath);
    &mychdir($CYGBLDROOT);
}

sub addpathsfromfile() {
    my $dirname = shift;
    my $envvarname = shift;

    my $pathfile=$WINBLDROOT."\\".$dirname."\\src\\configfiles\\".$dirname.".pth";
    if(!open(PTHFILE, $pathfile)) {
       &logmsg("Couldn't open ".$pathfile."!");
       exit(1);
    }

    my @filestrs=<PTHFILE>;
    close(PTHFILE);
    foreach my $i (@filestrs) {
      chop($i);
      $i =~ s/\//\\/g;  # switch fwdslash to backslsh
#      print $i,"\n";
      $ENV{$envvarname}.=";".$WINBLDROOT."\\".$dirname."\\".$i;
    }

}

sub gen_python_code() {

    # ETC_PATH required by generatePythonCode
    $ENV{'ETC_PATH'}='/home/builder/player/panda/etc /home/builder/player/direct/etc /home/builder/player/dtool/etc /home/builder/player/toontown/etc';
    my $origpath=$ENV{'PATH'};
    $ENV{'PATH'}="/usr/lib:/c/python16:/bin:/contrib/bin:/mscommon/Tools/WinNT:/mscommon/MSDev98/Bin:/mscommon/Tools:/msvc98/bin:/home/builder/player/dtool/bin:/home/builder/player/dtool/lib:/home/builder/player/direct/bin:/home/builder/player/direct/lib::/home/builder/player/toontown/bin:/home/builder/player/toontown/lib:/home/builder/player/panda/lib:/home/builder/player/panda/bin:/usr/local/bin:.:/c/WINNT/system32:/c/WINNT:/c/WINNT/System32/Wbem:/c/bin:/c/PROGRA~1/TCL/bin:/mspsdk/Bin/:/mspsdk/Bin/WinNT:/mscommon/Tools/WinNT:/mscommon/MSDev98/Bin:/mscommon/Tools:/msvc98/bin::/usr/local/panda/bin:/home/builder/scripts";
    my $directsrcroot=$WINBLDROOT."\\direct\\src";

    $ENV{'PYTHONPATH'}= $WINBLDROOT."\\panda\\lib;".$WINBLDROOT."\\dtool\\lib";
    
    &addpathsfromfile("direct","PYTHONPATH");
    &addpathsfromfile("toontown","PYTHONPATH");

    $ENV{'TCSH_NO_CHANGEPATH'}='1';

    &logmsg($ENV{'PYTHONPATH'}."\n");

    &mychdir($CYGBLDROOT."/direct/bin");

    $outputdir = $WINBLDROOT."\\direct\\lib\\py";
#    &mymkdir($outputdir);
#    $outputdir.= "\\Opt".$ENV{'PANDA_OPTIMIZE'}."-Win32";
#    &mymkdir($outputdir);
# now back to 1 build-type per tree

    my $genpyth_str;
    my $genargstr="-v -d";

    if(($ENV{'PANDA_OPTIMIZE'} eq '1') || ($ENV{'PANDA_OPTIMIZE'} eq '2')) {
       $genpyth_str="python_d ";
    } else {
       $genpyth_str="python -O ";
       $genargstr="-O ".$genargstr;
    }

    $genpyth_str.="generatePythonCode ".$genargstr." '".$outputdir."' -e '".$WINBLDROOT."\\direct\\src\\extensions' -i libdtool libpandaexpress libpanda libdirect libtoontown";
    
    &myexecstr($genpyth_str,"generate python code failed!!!","DO_LOG","NO_PANDA_ATTACH");
    
    $ENV{'PATH'}=$origpath;
    delete $ENV{'TCSH_NO_CHANGEPATH'};
    &mychdir($CYGBLDROOT);
}

my $newdayarchivedirname;

sub archivetree() {
    $treenum=shift;

    if(!(-e $inst_dirs[$treenum])) {
        return;
    }

    if($newdayarchivename eq "") {

        ($devicenum,$inodenum,$mode,$nlink,$uid,$gid,$rdev,$size,$atime,$mtime,$ctime,$blksize,$blocks)
           = stat($inst_dirs[$treenum]."\\dtool\\Sources.pp");
        ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) 
           = localtime($mtime);
        $mon++;

        $newdayarchivedirname=$inst_dirs[$ARCHIVENUM]."\\".($mon<10 ? "0":"").$mon."-".($mday<10 ? "0":"").$mday;
        &mymkdir($newdayarchivedirname);
    }

    &logmsg("*** Archiving ".$inst_dirnames[$treenum]." build on install server at ".&gettimestr()." ***");

    my $archdirname=$newdayarchivedirname."\\".$inst_dirnames[$treenum];

    &myrename($inst_dirs[$treenum],$archdirname);

    foreach my $dir1 (@dirstodolist) {    
        # copy DLL .pdb/.map up to lib dir so we can blow away metalibs subdir
        # could do this is the makefiles instead
        &myexecstr("( for /R ".$archdirname."\\".$dir1."\\metalibs %i in (lib*.pdb lib*.map) do copy %i ".$archdirname."\\".$dir1."\\lib )","nomsg","DO_LOG","NT cmd");

        if($dir1 eq "panda") {
            # dll/exe not under metalibs
            &myexecstr("( for /R ".$archdirname."\\".$dir1."\\src\\audiotraits %i in (lib*.pdb lib*.map) do copy %i ".$archdirname."\\".$dir1."\\lib )","nomsg","DO_LOG","NT cmd");
            &myexecstr("( for /R ".$archdirname."\\".$dir1."\\src\\shader %i in (lib*.pdb lib*.map) do copy %i ".$archdirname."\\".$dir1."\\lib )","nomsg","DO_LOG","NT cmd");
            &myexecstr("( for /R ".$archdirname."\\".$dir1."\\src\\testbed %i in (*.pdb *.map) do copy %i ".$archdirname."\\".$dir1."\\bin )","nomsg","DO_LOG","NT cmd");
        }

        # NT cmd 'for' always returns 144 for some reason, impossible to detect error cond, so just dont check retval
        # delete old objs/pdbs/etc out of archived trees (just blow away the Opt[Win32] dir)
        # &myexecstr("( for /D /R ".$archdirname."\\".$dir1."\\src %i in (Opt*Win32) do rd /s /q %i )","nomsg","DO_LOG","NT cmd");
        
        # instead blow away src,CVS,include,metalibs dirs completely
        # doing every rd twice since windows-samba-RAID link seems to screw up and cause some files to not be deleted the 1st time
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\src","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\CVS","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\include","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\metalibs","nomsg","DO_LOG","NT cmd");

        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\src","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\CVS","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\include","nomsg","DO_LOG","NT cmd");
        &myexecstr("rd /s /q ".$archdirname."\\".$dir1."\\metalibs","nomsg","DO_LOG","NT cmd");

        # del xtra files at root of subdirs
        &myexecstr("del /Q /F ".$archdirname."\\".$dir1."\\*","nomsg","DO_LOG","NT cmd");
    }

    # delete old browse files
    &myexecstr("del /q ".$archdirname."\\*.bsc","nomsg","DO_LOG","NT cmd");

    # could also move .pdb from metalibs to lib, then del metalibs, include, src dirs
}

sub checkoutfiles {
    my $existing_module_str="";
    my $nonexisting_module_str="";

   &mychdir($CYGBLDROOT);

    foreach my $dir1 (@dirstodolist) {
        if(-e $dir1) {
            $existing_module_str.=$dir1." ";
        } else {
            $nonexisting_module_str.=$dir1." ";
        }
    }
    
    if($existing_module_str ne "") {
        # flaw: will bomb is any CVS subdirs are missing

        &myexecstr("( for /D /R . %i in (Opt*Win32) do rd /s /q %i )","nomsg","DO_LOG","NT cmd");
    
        &logmsg("*** REMOVING ALL FILES IN OLD SRC TREES ***");
        # use cvs update to grab new copy
        # note: instead of blowing these away, may want to rename and save them
        # also, I could just blow everything away and check out again
        # cant blow away if people are sitting in those dirs, so we do this instead
        $rmcmd="find ".$existing_module_str." -path '*CVS*' -prune -or -not -name 'bldlog*' -and -not -type d -print | xargs --no-run-if-empty -n 40 rm";
        #&myexecstr($rmcmd,"Removal of old files failed!","DO_LOG","NO_CSHRC");
        &myexecstr($rmcmd,"Removal of old files failed!","DO_LOG","NO_PANDA_ATTACH");
    
        &myexecstr("cvs update -d -R ".$existing_module_str." |& egrep -v 'Updating|^\\?'",
                   "cvs update failed!","DO_LOG","NO_PANDA_ATTACH");
    }
    
    if($nonexisting_module_str ne "") {
        &myexecstr("cvs checkout -R ".$nonexisting_module_str." |& egrep -v 'Updating|^\\?'",
                   "cvs checkout failed!","DO_LOG","NO_PANDA_ATTACH");
    }
}

sub buildall() {
    $treenum=shift;

    # DTOOL ppremake may have already run by DTOOL 'initialize make'
    
    &logmsg("*** Starting ".uc($inst_dirnames[$treenum])." Build (Opt=".$ENV{'PANDA_OPTIMIZE'}.") at ".&gettimestr()." ***");

    &checkoutfiles();

    # cant do attachment, since that often hangs on NT
    # must use non-attachment build system  (BUILD_TYPE 'gmsvc', not 'stopgap', in $PPREMAKE_CONFIG)
    
    # hacks to fix multiproc build issue (cp file to dir occurs before dir creation)
    foreach my $dir1 (@dirstodolist) {    
        &mymkdir($CYGBLDROOT."/".$dir1."/etc"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/bin"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/lib"); 
        &mymkdir($CYGBLDROOT."/".$dir1."/include");
    }
    &mymkdir($CYGBLDROOT."/dtool/include/parser-inc");  # hack to fix makefile multiproc issue

    foreach my $dir1 (@dirstodolist) {    
        my $dir1_upcase = uc($dir1);

        &logmsg("*** PPREMAKE ".$dir1_upcase." ***");
        &mychdir($CYGBLDROOT."/".$dir1);
        &myexecstr("ppremake",$dir1_upcase." ppremake failed!","DO_LOG","NO_PANDA_ATTACH");
    }

    # debug stuff
    # &mychdir($CYGBLDROOT);
    # &myexecstr("dir dtool","dir failed","DO_LOG","NT cmd");

    foreach my $dir1 (@dirstodolist) {    
        my $dir1_upcase = uc($dir1);

        &logmsg("*** BUILDING ".$dir1_upcase." ***");
        &mychdir($CYGBLDROOT."/".$dir1);
        &myexecstr("make install",$dir1_upcase." make install failed!","DO_LOG","NO_PANDA_ATTACH");
    }

    &mychdir($CYGBLDROOT);  # get out of src dirs to allow them to be moved/renamed
    unlink($CYGBLDROOT."/dtool/dtool_config.h");  # fix freakish NTFS bug, this file is regenerated by ppremake anyway

    if($#dirstodolist>1) {    
       &gen_python_code();  # must run AFTER toontown bld
    }

    &mychdir($CYGBLDROOT);  # get out of src dirs to allow them to be moved/renamed

    # archive old current srvr build & copy build to server, if its accessible
    if(!(-e $WIN_INSTALLDIR)) {
        &logmsg("ERROR: Cant access install directory!!  (Is Server down??)  ".$WIN_INSTALLDIR);
        &logmsg("Skipping archive and copy-build-to-server steps for ".$inst_dirnames[$treenum]." build");
        return;
    }

    if($DONT_ARCHIVE_OLD_BUILDS) {
        &myexecstr("rd /s /q ".$inst_dirs[$treenum],"DO_LOG","NT cmd");  # dont bother checking errors here, probably just some shell has the dir cd'd to
    } else {
        &archivetree($treenum);
    }

    &logmsg("*** Copying ".$inst_dirnames[$treenum]." build to install server at ".&gettimestr()." ***");
    &mymkdir($inst_dirs[$treenum]);

    my $xcopy_opt_str="/E /K /C /R /Y /H ";

    if($DEBUG_TREECOPY) {
        $xcopy_opt_str.="/T";  #debug only
    }

    # hopefully there are no extra dirs underneath
    
    foreach my $dir1 (@dirstodolist) {        
        &mymkdir($inst_dirs[$treenum]."\\".$dir1);
        &myexecstr("xcopy ".$xcopy_opt_str." ".$WINBLDROOT."\\".$dir1."\\* ".$inst_dirs[$treenum]."\\".$dir1, 
                   "xcopy of ".$inst_dirnames[$treenum]." tree failed!!", "DO_LOG","NT cmd");
    }
}

# assumes environment already attached to TOOL/PANDA/DIRECT/TOONTOWN
# assumes cygwin env, BLDROOT must use fwd slashes
if($ENV{'BLDROOT'} eq "") {
  $ENV{'BLDROOT'} = "/home/builder/player";
}

if($ENV{'CYGWIN_ROOT'} eq "") {
  $ENV{'CYGWIN_ROOT'} = "C:\\Cygwin";
}

$CYGROOT= $ENV{'CYGWIN_ROOT'};
$CYGROOT =~ s/(.*)\\$/$1/;   # get rid of trailing '\'
#$CYGROOT =~ s/\\/\//g;  # switch backslash to fwdslash  (setting up for cygwin)

$CYGBLDROOT = $ENV{'BLDROOT'};
print "\$CYGBLDROOT='",$CYGBLDROOT,"'\n";

if(($CYGBLDROOT eq "")||(!(-e $CYGBLDROOT))) {
  die "Bad \$CYGBLDROOT !!\n";
}

$WINBLDROOT=$CYGROOT.$CYGBLDROOT;
$WINBLDROOT =~ s/\//\\/g;  # switch fwdslash to backslash
print "\$WINBLDROOT='",$WINBLDROOT,"'\n";

my ($sec,$min,$hour,$mday,$mon,$year,$wday) = localtime(time);
$mon++;
$logfilenamebase="bldlog-".($mon<10 ? "0":"").$mon."-".($mday<10 ? "0":"").$mday.".txt";
$fulllogfilename = $CYGBLDROOT."/".$logfilenamebase;
$fulllogfilename_win = $WINBLDROOT."\\".$logfilenamebase;

# recreate the log to blow away any old one
open(LOGFILE,">".$fulllogfilename) || die "can't open log file '$fulllogfilename'\n";
close(LOGFILE);

&logmsg("*** Panda Build Log Started at ".&gettimestr()." ***");

my @do_install_dir=(1,1,1,1);

if($#ARGV>=0) {
    @do_install_dir=(0,0,0,0);
    if($ARGV[0] eq $inst_dirnames[$INSTALLNUM]) {
        $do_install_dir[$INSTALLNUM]=1;
    } elsif($ARGV[0] eq $inst_dirnames[$DEBUGNUM]) {
        $do_install_dir[$DEBUGNUM]=1;
    } elsif($ARGV[0] eq $inst_dirnames[$RELEASENUM]) {
        $do_install_dir[$RELEASENUM]=1;
    } else {
        &logmsg("invalid argument '".$ARGV[0]."' to builder.pl.  arg must be 'install','debug', or 'release'\n");
        exit(1);
    }
}

if(!(-e $WIN_INSTALLDIR)) {
    &logmsg("ERROR: Cant access install directory!!  ".$WIN_INSTALLDIR);
    exit(1);
}

&mychdir($CYGBLDROOT);
# remove all old files (remove every file except for dirs and CVS files and bldlog*)
# and grab every file clean from CVS

if($BLD_DTOOL_ONLY) {
  @dirstodolist=("dtool");
} else {
  @dirstodolist=("dtool","panda","direct","toontown");
}
$dirstodostr="";
foreach my $dir1 (@dirstodolist) {
  $dirstodostr.=$dir1." ";
}

# makes ppremake build headers, libs in module dirs (panda/lib,dtool/bin,etc), not /usr/local/panda/inc...
foreach my $dir1 (@dirstodolist) {    
  my $dir1_upcase = uc($dir1);
  $ENV{$dir1_upcase}=$CYGBLDROOT."/".$dir1;

  # need this since we are building in src dirs, not install dir
  # so 'interrogate' needs to find its dlls when building panda, etc
  &appendpath("/".$dir1."/bin","/".$dir1."/lib");
}

# pick up cygwin utils
$ENV{'PATH'}="/bin".$DIRPATH_SEPARATOR."/contrib/bin".$DIRPATH_SEPARATOR.$ENV{'PATH'};

# want build to pick up python dll's from /usr/lib before /c/python16
$ENV{'PATH'}="/usr/lib".$DIRPATH_SEPARATOR.$ENV{'PATH'};

if($DEBUG_TREECOPY) {
    goto 'DBGTREECOPY';
}

if($DEBUG_GENERATE_PYTHON_CODE_ONLY) {
    &gen_python_code();
    exit(0);
}

# goto 'SKIP_REMOVE';

SKIP_REMOVE:

# this doesnt work unless you can completely remove the dirs, since cvs checkout
# bombs if dirs exist but CVS dirs do not. 

if($do_install_dir[$INSTALLNUM]) {
  $ENV{'PANDA_OPTIMIZE'}='2';
  &buildall($INSTALLNUM);
}

if($do_install_dir[$RELEASENUM]) {
    $ENV{'PANDA_OPTIMIZE'}='3';
    &buildall($RELEASENUM);
}

BEFORE_DBGBUILD:

if($do_install_dir[$DEBUGNUM]) {
    $ENV{'USE_BROWSEINFO'}='1';   # make .sbr files, this is probably obsolete
    if(! $DEBUG_GENERATE_PYTHON_CODE_ONLY) {
       $ENV{'PANDA_OPTIMIZE'}='1';
    }

    &buildall($DEBUGNUM);
    &make_bsc_file();
    delete $ENV{'USE_BROWSEINFO'};  # this is probably obsolete
}

AFTER_DBGBUILD:

&logmsg("*** Panda Build Log Finished at ".&gettimestr()." ***");

# store log in 'debug' dir
&myexecstr("copy ".$fulllogfilename_win." ".$inst_dirs[$DEBUGNUM], "copy of ".$fulllogfilename_win." failed!!", "","NT cmd");  

exit(0);

# TODO:
# compress old archived blds?
# build DLLs with version stamp set by this script
# implement build-specific opttype mode

