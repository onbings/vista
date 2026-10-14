#/usr/bin/env python
import argparse
import fnmatch
import hashlib
import os
from os.path import normpath, isdir, isfile, dirname, basename, exists as path_exists, join as path_join
import sys

class PathHash(object) :

    def __init__(self, _exclude_pattern = None) :
                
        self.exclude_pattern = _exclude_pattern

    # Description
    #   This function takes a list of paths
    #   and recursively compute a SHA-1 hash
    #   to represent the content of those path
    #
    # Parameters
    #   paths - A list of paths
    #
    # Returns
    #   A SHA-1 hash
    #
    def get_hash_of_paths(self, paths) :
        if not hasattr(paths, '__iter__'):
            raise TypeError('sequence or iterable expected not %r!' % type(paths))

        def is_processing_allowed(path) :
        
            # Do we need to exclude files ?
            if self.exclude_pattern :
                return not fnmatch.fnmatch(path, self.exclude_pattern)                    
            
            return True

        def _update_checksum(checksum, dirname, filenames) :
            for filename in sorted(filenames):           
                if is_processing_allowed(filename) :
                    #print("Processing file {} in dir {}".format(filename, dirname))
                    path = path_join(dirname, filename)
                    if isfile(path):
                        #print("Processing " + path)
                        fh = open(path, 'rb')
                        while 1:
                            buf = fh.read(4096)
                            if not buf : break
                            checksum.update(buf)
                        fh.close()
                else :
                    print("Excluding file {}".format(filename))

        chksum = hashlib.sha1()

        for path in sorted([normpath(os.path.abspath(f)) for f in paths]):
            if path_exists(path):
                if isdir(path):
                    for dirpath, dirnames, files in os.walk(path, topdown=True) :
                       
                        dirnames.sort()
 
                        # Exclude directories if needed
                        for dir in dirnames[:] :
                            if not is_processing_allowed(dir) :
                                print("Excluding directory {}".format(dir))
                                dirnames.remove(dir)
                        
                        _update_checksum(chksum, dirpath, files)
                        
                elif isfile(path):
                    _update_checksum(chksum, dirname(path), [ basename(path) ])
            else :
                print("Path does not exist")

        return chksum.hexdigest()

    def get_hash_of_path(self, path) :
        return self.get_hash_of_paths([path])

#
# Description
#   This is the entry point of the script
#
# Parameters
#   Dir - The base directory to scan
#
# Returns
#    0 - The operation was successful
#   !0 - The operation failed
#
def main(argv):
    parser = argparse.ArgumentParser()

    parser._action_groups.pop()
    required = parser.add_argument_group('Required arguments')
    optional = parser.add_argument_group('Optional arguments')

    # Mandatory arguments
    required.add_argument("-p", "--path",    dest="path",     help = "The path to scan", required=True)

    # Optional arguments
    optional.add_argument("-e", "--exclude", dest="exclude",  help = "The exclusion pattern")
    
    args = parser.parse_args(argv)
    
    hasher = PathHash(args.exclude)    

    print(hasher.get_hash_of_path(args.path))
#
# Description
#   The script main wrapper
#
if __name__ == "__main__":
    try :
        sys.exit(main(sys.argv[1:]))
    except Exception as e :
        print(e)
        sys.exit(1)
        
