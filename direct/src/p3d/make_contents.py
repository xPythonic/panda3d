#! /usr/bin/env python

"""
This command will build the contents.xml file at the top of a Panda3D
download hierarchy.  This file lists all of the packages hosted here,
along with their current versions.

This program runs on a local copy of the hosting directory hierarchy;
it must be a complete copy to generate a complete contents.xml file.

make_contents.py [opts]

Options:

  -i install_dir
     The full path to a local directory that contains the
     ready-to-be-published files, as populated by one or more
     iterations of the ppackage script.  It is the user's
     responsibility to copy this directory structure to a server.

  -n "host descriptive name"
     Specifies a descriptive name of the download server that will
     host these contents.  This name may be presented to the user when
     managing installed packages.  If this option is omitted, the name
     is unchanged from the previous pass.

"""

import sys
import getopt
import os
import types

try:
    import hashlib
except ImportError:
    # Legacy Python support
    import md5 as hashlib

class ArgumentError(AttributeError):
    pass

class FileSpec:
    """ Represents a single file in the directory, and its associated
    timestamp, size, and md5 hash. """
    
    def __init__(self, filename, pathname):
        self.filename = filename
        self.pathname = pathname

        s = os.stat(pathname)
        self.size = s.st_size
        self.timestamp = int(s.st_mtime)

        m = hashlib.md5()
        m.update(open(pathname, 'rb').read())
        self.hash = m.hexdigest()

    def getParams(self):
        return 'filename="%s" size="%s" timestamp="%s" hash="%s"' % (
            self.filename, self.size, self.timestamp, self.hash)

class ContentsMaker:
    def __init__(self):
        self.installDir = None
        self.hostDescriptiveName = None

    def build(self):
        if not self.installDir:
            raise ArgumentError, "Stage directory not specified."

        self.packages = []
        self.scanDirectory()

        if not self.packages:
            raise ArgumentError, "No packages found."

        contentsFileBasename = 'contents.xml'
        contentsFilePathname = os.path.join(self.installDir, contentsFileBasename)
        contentsLine = None
        if self.hostDescriptiveName is not None:
            if self.hostDescriptiveName:
                contentsLine = '<contents descriptive_name="%s">' % (
                    self.quoteString(self.hostDescriptiveName))
        else:
            contentsLine = self.readContentsLine(contentsFilePathname)
        if not contentsLine:
            contentsLine = '<contents>'

        # Now write the contents.xml file.
        f = open(contentsFilePathname, 'w')
        print >> f, '<?xml version="1.0" encoding="utf-8" ?>'
        print >> f, ''
        print >> f, contentsLine
        for type, packageName, packagePlatform, packageVersion, file, solo in self.packages:
            extra = ''
            if solo:
                extra += 'solo="1" '
            print >> f, '  <%s name="%s" platform="%s" version="%s" %s%s />' % (
                type, packageName, packagePlatform or '', packageVersion or '', extra, file.getParams())
        print >> f, '</contents>'
        f.close()

    def readContentsLine(self, contentsFilePathname):
        """ Reads the previous iteration of contents.xml to get the
        previous top-level contents line, which contains the
        hostDescriptiveName. """

        try:
            f = open(contentsFilePathname, 'r')
        except IOError:
            return None

        for line in f.readlines():
            if line.startswith('<contents'):
                return line.rstrip()

        return None

    def quoteString(self, str):
        """ Correctly quotes a string for embedding in the xml file. """
        if isinstance(str, types.UnicodeType):
            str = str.encode('utf-8')
        str = str.replace('&', '&amp;')
        str = str.replace('"', '&quot;')
        str = str.replace('\'', '&apos;')
        str = str.replace('<', '&lt;')
        str = str.replace('>', '&gt;')
        return str
    
    def scanDirectory(self):
        """ Walks through all the files in the stage directory and
        looks for the package directory xml files. """

        startDir = self.installDir
        if startDir.endswith(os.sep):
            startDir = startDir[:-1]
        prefix = startDir + os.sep
        for dirpath, dirnames, filenames in os.walk(startDir):
            if dirpath == startDir:
                localpath = ''
                xml = ''
            else:
                assert dirpath.startswith(prefix)
                localpath = dirpath[len(prefix):].replace(os.sep, '/') + '/'
                xml = dirpath[len(prefix):].replace(os.sep, '_') + '.xml'

            solo = False

            # A special case: if a directory contains just one file,
            # it's a "solo", not an xml package.
            if len(filenames) == 1 and not filenames[0].endswith('.xml'):
                xml = filenames[0]
                solo = True

            if xml not in filenames:
                continue

            if localpath.count('/') == 1:
                packageName, junk = localpath.split('/')
                packageVersion = None
                packagePlatform = None
            
            elif localpath.count('/') == 2:
                packageName, packageVersion, junk = localpath.split('/')
                packagePlatform = None

            elif localpath.count('/') == 3:
                packageName, packagePlatform, packageVersion, junk = localpath.split('/')
            else:
                continue

            file = FileSpec(localpath + xml,
                            os.path.join(self.installDir, localpath + xml))
            print file.filename
            self.packages.append(('package', packageName, packagePlatform, packageVersion, file, solo))

            if not solo:
                # Look for an _import.xml file, too.
                xml = xml[:-4] + '_import.xml'
                try:
                    file = FileSpec(localpath + xml,
                                    os.path.join(self.installDir, localpath + xml))
                except OSError:
                    file = None
                if file:
                    print file.filename
                    self.packages.append(('import', packageName, packagePlatform, packageVersion, file, False))
        
                
def makeContents(args):
    opts, args = getopt.getopt(args, 'i:n:h')

    cm = ContentsMaker()
    cm.installDir = '.'
    for option, value in opts:
        if option == '-i':
            cm.installDir = value

        elif option == '-n':
            cm.hostDescriptiveName = value
            
        elif option == '-h':
            print __doc__
            sys.exit(1)

    cm.build()
        

if __name__ == '__main__':
    try:
        makeContents(sys.argv[1:])
    except ArgumentError, e:
        print e.args[0]
        sys.exit(1)
